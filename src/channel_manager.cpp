/**
 * @file channel_manager.cpp
 * @brief Implementation of multi-channel manager for NMDB
 */

#include "nmdb/channel_manager.hpp"
#include <iostream>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#define mkdir(path, mode) _mkdir(path)
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

namespace nmdb {

// ============================================================================
// Channel Implementation
// ============================================================================

Channel::Channel(const std::string& channel_id,
                 const std::string& socket_path,
                 int max_clients)
    : channel_id_(channel_id)
    , socket_path_(socket_path) {
    server_ = std::make_unique<SocketServer>(socket_path, max_clients);
}

Channel::~Channel() {
    stop();
}

bool Channel::start() {
    return server_->start();
}

void Channel::stop() {
    server_->stop();
}

bool Channel::is_running() const {
    return server_->is_running();
}

// ============================================================================
// ChannelManager Implementation
// ============================================================================

ChannelManager::ChannelManager(const std::string& base_socket_dir)
    : base_socket_dir_(base_socket_dir) {
    // Create base directory if it doesn't exist
    mkdir(base_socket_dir_.c_str(), 0755);
}

ChannelManager::~ChannelManager() {
    // Stop all channels
    std::lock_guard<std::mutex> lock(channels_mutex_);
    for (auto& [id, channel] : channels_) {
        channel->stop();
    }
    channels_.clear();
}

bool ChannelManager::create_channel(const std::string& channel_id, int max_clients) {
    std::lock_guard<std::mutex> lock(channels_mutex_);

    // Check if channel already exists
    if (channels_.find(channel_id) != channels_.end()) {
        std::cerr << "ChannelManager: Channel " << channel_id << " already exists" << std::endl;
        return false;
    }

    // Create socket path
    std::string socket_path = base_socket_dir_ + "/" + channel_id + ".sock";

    // Create channel
    auto channel = std::make_unique<Channel>(channel_id, socket_path, max_clients);

    // Setup callbacks
    setup_channel_callbacks(channel.get());

    // Start channel
    if (!channel->start()) {
        std::cerr << "ChannelManager: Failed to start channel " << channel_id << std::endl;
        return false;
    }

    // Store channel
    channels_[channel_id] = std::move(channel);

    std::cout << "ChannelManager: Created channel " << channel_id
              << " at " << socket_path << std::endl;
    return true;
}

bool ChannelManager::destroy_channel(const std::string& channel_id) {
    std::lock_guard<std::mutex> lock(channels_mutex_);

    auto it = channels_.find(channel_id);
    if (it == channels_.end()) {
        std::cerr << "ChannelManager: Channel " << channel_id << " not found" << std::endl;
        return false;
    }

    it->second->stop();
    channels_.erase(it);

    std::cout << "ChannelManager: Destroyed channel " << channel_id << std::endl;
    return true;
}

bool ChannelManager::has_channel(const std::string& channel_id) const {
    std::lock_guard<std::mutex> lock(channels_mutex_);
    return channels_.find(channel_id) != channels_.end();
}

Channel* ChannelManager::get_channel(const std::string& channel_id) {
    std::lock_guard<std::mutex> lock(channels_mutex_);
    auto it = channels_.find(channel_id);
    return (it != channels_.end()) ? it->second.get() : nullptr;
}

std::vector<std::string> ChannelManager::get_channel_ids() const {
    std::lock_guard<std::mutex> lock(channels_mutex_);
    std::vector<std::string> ids;
    ids.reserve(channels_.size());
    for (const auto& [id, channel] : channels_) {
        ids.push_back(id);
    }
    return ids;
}

bool ChannelManager::send_message(const std::string& channel_id,
                                  int client_id,
                                  const uint8_t* data,
                                  size_t size) {
    Channel* channel = get_channel(channel_id);
    if (!channel) {
        std::cerr << "ChannelManager: Channel " << channel_id << " not found" << std::endl;
        return false;
    }

    return channel->get_server()->send_message(client_id, data, size);
}

int ChannelManager::broadcast_to_channel(const std::string& channel_id,
                                         const uint8_t* data,
                                         size_t size) {
    Channel* channel = get_channel(channel_id);
    if (!channel) {
        std::cerr << "ChannelManager: Channel " << channel_id << " not found" << std::endl;
        return 0;
    }

    return channel->get_server()->broadcast_message(data, size);
}

void ChannelManager::setup_channel_callbacks(Channel* channel) {
    auto server = channel->get_server();
    const std::string channel_id = channel->get_id();

    // Setup message callback
    server->set_message_callback([this, channel_id](int client_id, const uint8_t* data, size_t size) {
        if (message_callback_) {
            message_callback_(channel_id, client_id, data, size);
        }
    });
}

} // namespace nmdb
