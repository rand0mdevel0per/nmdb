/**
 * @file basic_client.c
 * @brief Basic NMDB C client example
 */

#include <nmdb_client.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

static volatile int running = 1;

void signal_handler(int sig) {
    (void)sig;
    running = 0;
}

void on_message(void* user_data, const uint8_t* data, size_t size) {
    (void)user_data;
    printf("Received: %.*s\n", (int)size, data);
}

int main(int argc, char* argv[]) {
    const char* socket_path = "/tmp/nmdb/c1.sock";

    if (argc > 1) {
        socket_path = argv[1];
    }

    printf("Connecting to %s...\n", socket_path);

    // Create client
    nmdb_client_t* client = nmdb_client_create(socket_path);
    if (!client) {
        fprintf(stderr, "Failed to create client\n");
        return 1;
    }

    // Connect
    if (!nmdb_client_connect(client)) {
        fprintf(stderr, "Failed to connect\n");
        nmdb_client_destroy(client);
        return 1;
    }

    printf("Connected successfully!\n");

    // Set message callback
    nmdb_client_set_callback(client, on_message, NULL);

    // Send some messages
    printf("\nSending messages...\n");
    nmdb_client_send_text(client, "Hello, NMDB!");
    nmdb_client_send_text(client, "This is a test message.");
    nmdb_client_send_text(client, "C client works!");

    // Setup signal handler
    signal(SIGINT, signal_handler);

    // Keep running
    printf("\nListening for messages (press Ctrl+C to exit)...\n");
    while (running) {
        sleep(1);
    }

    // Cleanup
    printf("\nShutting down...\n");
    nmdb_client_destroy(client);
    printf("Disconnected.\n");

    return 0;
}
