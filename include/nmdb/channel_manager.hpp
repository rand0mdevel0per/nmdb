/**
 * @file channel_manager.hpp
 * @brief Multi-channel manager for NMDB
 *
 * Manages multiple communication channels with dynamic creation/destruction.
 */

#ifndef NMDB_CHANNEL_MANAGER_HPP
#define NMDB_CHANNEL_MANAGER_HPP

#include "socket_server.hpp"
#include <string>
#include <memory>
#include <unordered_map>
#include <mutex>
#include <functional>

namespace nmdb {

// Forward declarations
struct ChannelMetadata;
struct NMDBMessage;

/**
 * @class Channel
 * @brief Represents a single communication channel
 */
class Channel {
public:
    /**
     * @brief Constructor
     * @param channel_id Channel identifier (e.g., "c1", "c2")
     * @param socket_path Unix socket path
     * @param max_clients Maximum number of clients
     */
    Channel(const std::string& channel_id,
            const std::string& socket_path,
            int max_clients = 32);

    /**
     * @brief Destructor
     */
    ~Channel();

    /**
     * @brief Start the channel
     * @return True if started successfully
     */
    bool start();

    /**
     * @brief Stop the channel
     */
    void stop();

    /**
     * @brief Check if channel is running
     * @return True if running
     */
    bool is_running() const;

    /**
     * @brief Get channel ID
     * @return Channel identifier
     */
    const std::string& get_id() const { return channel_id_; }

    /**
     * @brief Get socket path
     * @return Socket path
     */
    const std::string& get_socket_path() const { return socket_path_; }

    /**
     * @brief Get underlying socket server
     * @return Pointer to socket server
     */
    SocketServer* get_server() { return server_.get(); }

private:
    std::string channel_id_;
    std::string socket_path_;
    std::unique_ptr<SocketServer> server_;
};

/**
 * @class ChannelManager
 * @brief Manages multiple communication channels
 *
 * Features:
 * - Dynamic channel creation/destruction
 * - Multi-channel message routing
 * - Channel-in-channel (CIC) support
 * - Thread-safe operations
 */
class ChannelManager {
public:
    /**
     * @brief Message routing callback
     * @param channel_id Source channel ID
     * @param client_id Client identifier
     * @param data Message data
     * @param size Data size
     */
    using MessageCallback = std::function<void(
        const std::string& channel_id,
        int client_id,
        const uint8_t* data,
        size_t size)>;

    /**
     * @brief Constructor
     * @param base_socket_dir Base directory for socket files (e.g., "/tmp/nmdb")
     */
    explicit ChannelManager(const std::string& base_socket_dir = "/tmp/nmdb");

    /**
     * @brief Destructor
     */
    ~ChannelManager();

    // Disable copy and move
    ChannelManager(const ChannelManager&) = delete;
    ChannelManager& operator=(const ChannelManager&) = delete;
    ChannelManager(ChannelManager&&) = delete;
    ChannelManager& operator=(ChannelManager&&) = delete;

    /**
     * @brief Create a new channel
     * @param channel_id Channel identifier (e.g., "c1", "c2")
     * @param max_clients Maximum number of clients
     * @return True if created successfully
     */
    bool create_channel(const std::string& channel_id, int max_clients = 32);

    /**
     * @brief Destroy a channel
     * @param channel_id Channel identifier
     * @return True if destroyed successfully
     */
    bool destroy_channel(const std::string& channel_id);

    /**
     * @brief Check if channel exists
     * @param channel_id Channel identifier
     * @return True if exists
     */
    bool has_channel(const std::string& channel_id) const;

    /**
     * @brief Get channel by ID
     * @param channel_id Channel identifier
     * @return Pointer to channel, or nullptr if not found
     */
    Channel* get_channel(const std::string& channel_id);

    /**
     * @brief Get all channel IDs
     * @return Vector of channel IDs
     */
    std::vector<std::string> get_channel_ids() const;

    /**
     * @brief Send message to a specific channel and client
     * @param channel_id Channel identifier
     * @param client_id Client identifier
     * @param data Message data
     * @param size Data size
     * @return True if sent successfully
     */
    bool send_message(const std::string& channel_id,
                     int client_id,
                     const uint8_t* data,
                     size_t size);

    /**
     * @brief Broadcast message to all clients on a channel
     * @param channel_id Channel identifier
     * @param data Message data
     * @param size Data size
     * @return Number of clients that received the message
     */
    int broadcast_to_channel(const std::string& channel_id,
                            const uint8_t* data,
                            size_t size);

    /**
     * @brief Set message callback
     * @param callback Message callback function
     */
    void set_message_callback(MessageCallback callback) {
        message_callback_ = std::move(callback);
    }

private:
    /**
     * @brief Setup message callback for a channel
     * @param channel Channel to setup
     */
    void setup_channel_callbacks(Channel* channel);

    // Configuration
    std::string base_socket_dir_;

    // Channel storage
    std::unordered_map<std::string, std::unique_ptr<Channel>> channels_;
    mutable std::mutex channels_mutex_;

    // Callbacks
    MessageCallback message_callback_;
};

} // namespace nmdb

#endif // NMDB_CHANNEL_MANAGER_HPP
