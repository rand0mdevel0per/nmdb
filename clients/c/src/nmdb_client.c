/**
 * @file nmdb_client.c
 * @brief NMDB C Client Implementation
 */

#include "nmdb_client.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include <errno.h>
#include <stdio.h>

#define BUFFER_SIZE 8192

/**
 * @brief Client structure
 */
struct nmdb_client {
    char* socket_path;
    int sock_fd;
    bool connected;
    pthread_t receive_thread;
    bool running;
    nmdb_message_callback_t callback;
    void* user_data;
};

/**
 * @brief Receive loop thread function
 */
static void* receive_loop(void* arg) {
    nmdb_client_t* client = (nmdb_client_t*)arg;
    uint8_t buffer[BUFFER_SIZE];

    while (client->running) {
        ssize_t bytes_read = recv(client->sock_fd, buffer, BUFFER_SIZE, 0);

        if (bytes_read > 0) {
            if (client->callback) {
                client->callback(client->user_data, buffer, (size_t)bytes_read);
            }
        } else if (bytes_read == 0) {
            // Connection closed
            break;
        } else {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                fprintf(stderr, "Receive error: %s\n", strerror(errno));
                break;
            }
        }
    }

    client->connected = false;
    return NULL;
}

nmdb_client_t* nmdb_client_create(const char* socket_path) {
    if (!socket_path) {
        return NULL;
    }

    nmdb_client_t* client = (nmdb_client_t*)malloc(sizeof(nmdb_client_t));
    if (!client) {
        return NULL;
    }

    client->socket_path = strdup(socket_path);
    if (!client->socket_path) {
        free(client);
        return NULL;
    }

    client->sock_fd = -1;
    client->connected = false;
    client->running = false;
    client->callback = NULL;
    client->user_data = NULL;

    return client;
}

void nmdb_client_destroy(nmdb_client_t* client) {
    if (!client) {
        return;
    }

    nmdb_client_disconnect(client);

    if (client->socket_path) {
        free(client->socket_path);
    }

    free(client);
}

bool nmdb_client_connect(nmdb_client_t* client) {
    if (!client || client->connected) {
        return false;
    }

    // Create Unix domain socket
    client->sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client->sock_fd < 0) {
        fprintf(stderr, "Failed to create socket: %s\n", strerror(errno));
        return false;
    }

    // Connect to server
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, client->socket_path, sizeof(addr.sun_path) - 1);

    if (connect(client->sock_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        fprintf(stderr, "Failed to connect: %s\n", strerror(errno));
        close(client->sock_fd);
        client->sock_fd = -1;
        return false;
    }

    client->connected = true;

    // Start receive thread
    client->running = true;
    if (pthread_create(&client->receive_thread, NULL, receive_loop, client) != 0) {
        fprintf(stderr, "Failed to create receive thread\n");
        close(client->sock_fd);
        client->sock_fd = -1;
        client->connected = false;
        return false;
    }

    return true;
}

void nmdb_client_disconnect(nmdb_client_t* client) {
    if (!client || !client->connected) {
        return;
    }

    client->running = false;

    // Wait for receive thread to finish
    pthread_join(client->receive_thread, NULL);

    // Close socket
    if (client->sock_fd >= 0) {
        close(client->sock_fd);
        client->sock_fd = -1;
    }

    client->connected = false;
}

bool nmdb_client_is_connected(const nmdb_client_t* client) {
    return client && client->connected;
}

bool nmdb_client_send_raw(nmdb_client_t* client, const uint8_t* data, size_t size) {
    if (!client || !client->connected || !data || size == 0) {
        return false;
    }

    ssize_t bytes_sent = send(client->sock_fd, data, size, 0);
    if (bytes_sent < 0) {
        fprintf(stderr, "Failed to send: %s\n", strerror(errno));
        return false;
    }

    return (size_t)bytes_sent == size;
}

bool nmdb_client_send_text(nmdb_client_t* client, const char* text) {
    if (!text) {
        return false;
    }

    return nmdb_client_send_raw(client, (const uint8_t*)text, strlen(text));
}

void nmdb_client_set_callback(nmdb_client_t* client, nmdb_message_callback_t callback, void* user_data) {
    if (!client) {
        return;
    }

    client->callback = callback;
    client->user_data = user_data;
}
