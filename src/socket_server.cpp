/**
 * @file socket_server.cpp
 * @brief Implementation of Unix socket server for NMDB
 */

#include "nmdb/socket_server.hpp"
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <cerrno>
#include <iostream>

#ifdef __linux__
#include <sys/epoll.h>
#endif

namespace nmdb {

SocketServer::SocketServer(const std::string& socket_path, int max_clients)
    : socket_path_(socket_path)
    , max_clients_(max_clients)
    , server_fd_(-1)
    , running_(false)
    , next_client_id_(1) {
}

SocketServer::~SocketServer() {
    stop();
}

bool SocketServer::start() {
    if (running_.load()) {
        std::cerr << "SocketServer: Already running" << std::endl;
        return false;
    }

    // Create Unix domain socket
    server_fd_ = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd_ < 0) {
        std::cerr << "SocketServer: Failed to create socket: " << strerror(errno) << std::endl;
        return false;
    }

    // Set socket to non-blocking mode
    int flags = fcntl(server_fd_, F_GETFL, 0);
    fcntl(server_fd_, F_SETFL, flags | O_NONBLOCK);

    // Remove existing socket file if it exists
    unlink(socket_path_.c_str());

    // Bind socket
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path_.c_str(), sizeof(addr.sun_path) - 1);

    if (bind(server_fd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "SocketServer: Failed to bind socket: " << strerror(errno) << std::endl;
        close(server_fd_);
        server_fd_ = -1;
        return false;
    }

    // Listen for connections
    if (listen(server_fd_, max_clients_) < 0) {
        std::cerr << "SocketServer: Failed to listen: " << strerror(errno) << std::endl;
        close(server_fd_);
        server_fd_ = -1;
        return false;
    }

    // Start worker threads
    running_.store(true);
    accept_thread_ = std::thread(&SocketServer::accept_loop, this);
    io_thread_ = std::thread(&SocketServer::io_loop, this);

    std::cout << "SocketServer: Started on " << socket_path_ << std::endl;
    return true;
}

void SocketServer::stop() {
    if (!running_.load()) {
        return;
    }

    running_.store(false);

    // Wait for threads to finish
    if (accept_thread_.joinable()) {
        accept_thread_.join();
    }
    if (io_thread_.joinable()) {
        io_thread_.join();
    }

    cleanup();
    std::cout << "SocketServer: Stopped" << std::endl;
}

void SocketServer::cleanup() {
    // Close all client connections
    {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        for (auto& [client_id, fd] : client_fds_) {
            close(fd);
        }
        client_fds_.clear();
    }

    // Close server socket
    if (server_fd_ >= 0) {
        close(server_fd_);
        server_fd_ = -1;
    }

    // Remove socket file
    unlink(socket_path_.c_str());
}

int SocketServer::get_client_count() const {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    return static_cast<int>(client_fds_.size());
}

void SocketServer::accept_loop() {
    while (running_.load()) {
        struct sockaddr_un client_addr;
        socklen_t client_len = sizeof(client_addr);

        int client_fd = accept(server_fd_, (struct sockaddr*)&client_addr, &client_len);

        if (client_fd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // No pending connections, sleep briefly
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }
            std::cerr << "SocketServer: Accept failed: " << strerror(errno) << std::endl;
            continue;
        }

        // Set client socket to non-blocking
        int flags = fcntl(client_fd, F_GETFL, 0);
        fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

        // Add client to map
        int client_id = next_client_id_.fetch_add(1);
        {
            std::lock_guard<std::mutex> lock(clients_mutex_);
            client_fds_[client_id] = client_fd;
        }

        // Notify connection callback
        if (connection_callback_) {
            connection_callback_(client_id, true);
        }

        std::cout << "SocketServer: Client " << client_id << " connected" << std::endl;
    }
}

void SocketServer::io_loop() {
    const size_t BUFFER_SIZE = 8192;
    std::vector<uint8_t> buffer(BUFFER_SIZE);

    while (running_.load()) {
        std::vector<int> clients_to_check;
        {
            std::lock_guard<std::mutex> lock(clients_mutex_);
            for (const auto& [client_id, fd] : client_fds_) {
                clients_to_check.push_back(client_id);
            }
        }

        for (int client_id : clients_to_check) {
            int fd;
            {
                std::lock_guard<std::mutex> lock(clients_mutex_);
                auto it = client_fds_.find(client_id);
                if (it == client_fds_.end()) {
                    continue;
                }
                fd = it->second;
            }

            ssize_t bytes_read = recv(fd, buffer.data(), BUFFER_SIZE, MSG_DONTWAIT);

            if (bytes_read > 0) {
                // Notify message callback
                if (message_callback_) {
                    message_callback_(client_id, buffer.data(), static_cast<size_t>(bytes_read));
                }
            } else if (bytes_read == 0) {
                // Client disconnected
                handle_disconnect(client_id);
            } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
                // Error occurred
                std::cerr << "SocketServer: Read error for client " << client_id
                          << ": " << strerror(errno) << std::endl;
                handle_disconnect(client_id);
            }
        }

        // Sleep briefly to avoid busy-waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

bool SocketServer::send_message(int client_id, const uint8_t* data, size_t size) {
    int fd;
    {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        auto it = client_fds_.find(client_id);
        if (it == client_fds_.end()) {
            return false;
        }
        fd = it->second;
    }

    ssize_t bytes_sent = send(fd, data, size, MSG_NOSIGNAL);
    if (bytes_sent < 0) {
        std::cerr << "SocketServer: Send failed for client " << client_id
                  << ": " << strerror(errno) << std::endl;
        handle_disconnect(client_id);
        return false;
    }

    return bytes_sent == static_cast<ssize_t>(size);
}

int SocketServer::broadcast_message(const uint8_t* data, size_t size) {
    std::vector<int> client_ids;
    {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        for (const auto& [client_id, fd] : client_fds_) {
            client_ids.push_back(client_id);
        }
    }

    int success_count = 0;
    for (int client_id : client_ids) {
        if (send_message(client_id, data, size)) {
            success_count++;
        }
    }

    return success_count;
}

void SocketServer::handle_disconnect(int client_id) {
    int fd;
    {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        auto it = client_fds_.find(client_id);
        if (it == client_fds_.end()) {
            return;
        }
        fd = it->second;
        client_fds_.erase(it);
    }

    close(fd);

    // Notify connection callback
    if (connection_callback_) {
        connection_callback_(client_id, false);
    }

    std::cout << "SocketServer: Client " << client_id << " disconnected" << std::endl;
}

} // namespace nmdb
