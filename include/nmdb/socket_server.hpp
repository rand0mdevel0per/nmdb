/**
 * @file socket_server.hpp
 * @brief Unix socket server for NMDB multi-channel communication
 *
 * Provides async I/O server supporting multiple clients and channels.
 */

#ifndef NMDB_SOCKET_SERVER_HPP
#define NMDB_SOCKET_SERVER_HPP

#include <string>
#include <memory>
#include <functional>
#include <vector>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <unordered_map>

namespace nmdb {

// Forward declarations
class ChannelManager;
struct NMDBMessage;

/**
 * @class SocketServer
 * @brief Unix socket server with multi-client support
 *
 * Features:
 * - Async I/O using epoll (Linux) or select (cross-platform)
 * - Multi-client connection management
 * - Thread-safe message queue
 * - Automatic reconnection handling
 */
class SocketServer {
public:
    /**
     * @brief Connection callback type
     * @param client_id Unique client identifier
     * @param connected True if connected, false if disconnected
     */
    using ConnectionCallback = std::function<void(int client_id, bool connected)>;

    /**
     * @brief Message callback type
     * @param client_id Client identifier
     * @param data Message data
     * @param size Data size in bytes
     */
    using MessageCallback = std::function<void(int client_id, const uint8_t* data, size_t size)>;

    /**
     * @brief Constructor
     * @param socket_path Unix socket path (e.g., "/tmp/nmdb_c1.sock")
     * @param max_clients Maximum number of concurrent clients
     */
    explicit SocketServer(const std::string& socket_path, int max_clients = 32);

    /**
     * @brief Destructor
     */
    ~SocketServer();

    // Disable copy and move
    SocketServer(const SocketServer&) = delete;
    SocketServer& operator=(const SocketServer&) = delete;
    SocketServer(SocketServer&&) = delete;
    SocketServer& operator=(SocketServer&&) = delete;

    /**
     * @brief Start the server
     * @return True if started successfully
     */
    bool start();

    /**
     * @brief Stop the server
     */
    void stop();

    /**
     * @brief Check if server is running
     * @return True if running
     */
    bool is_running() const { return running_.load(); }

    /**
     * @brief Send message to a specific client
     * @param client_id Client identifier
     * @param data Message data
     * @param size Data size in bytes
     * @return True if sent successfully
     */
    bool send_message(int client_id, const uint8_t* data, size_t size);

    /**
     * @brief Broadcast message to all connected clients
     * @param data Message data
     * @param size Data size in bytes
     * @return Number of clients that received the message
     */
    int broadcast_message(const uint8_t* data, size_t size);

    /**
     * @brief Set connection callback
     * @param callback Connection callback function
     */
    void set_connection_callback(ConnectionCallback callback) {
        connection_callback_ = std::move(callback);
    }

    /**
     * @brief Set message callback
     * @param callback Message callback function
     */
    void set_message_callback(MessageCallback callback) {
        message_callback_ = std::move(callback);
    }

    /**
     * @brief Get number of connected clients
     * @return Number of connected clients
     */
    int get_client_count() const;

private:
    /**
     * @brief Accept loop for incoming connections
     */
    void accept_loop();

    /**
     * @brief I/O loop for handling client messages
     */
    void io_loop();

    /**
     * @brief Handle client disconnection
     * @param client_id Client identifier
     */
    void handle_disconnect(int client_id);

    /**
     * @brief Cleanup resources
     */
    void cleanup();

    // Server configuration
    std::string socket_path_;
    int max_clients_;

    // Socket file descriptors
    int server_fd_;
    std::unordered_map<int, int> client_fds_;  // client_id -> fd

    // Threading
    std::atomic<bool> running_;
    std::thread accept_thread_;
    std::thread io_thread_;
    mutable std::mutex clients_mutex_;

    // Callbacks
    ConnectionCallback connection_callback_;
    MessageCallback message_callback_;

    // Client ID counter
    std::atomic<int> next_client_id_;
};

} // namespace nmdb

#endif // NMDB_SOCKET_SERVER_HPP
