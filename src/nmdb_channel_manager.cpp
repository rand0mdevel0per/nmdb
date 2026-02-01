#include "nmdb_channel_manager.h"
#include <fstream>
#include <filesystem>
#include <stdexcept>

namespace nmdb {

// ============================================================================
// NMDBChannel::Impl - Internal implementation
// ============================================================================

class NMDBChannel::Impl {
public:
    explicit Impl(const std::string& db_path) : db_path_(db_path) {
        // Create directory if it doesn't exist
        std::filesystem::path path(db_path_);
        std::filesystem::create_directories(path.parent_path());
    }

    bool store(const std::string& key, const sintellix::CICData& data) {
        std::string file_path = get_file_path(key);

        std::ofstream ofs(file_path, std::ios::binary);
        if (!ofs) {
            return false;
        }

        std::string serialized;
        if (!data.SerializeToString(&serialized)) {
            return false;
        }

        ofs.write(serialized.data(), serialized.size());
        return ofs.good();
    }

    bool load(const std::string& key, sintellix::CICData& data) {
        std::string file_path = get_file_path(key);

        std::ifstream ifs(file_path, std::ios::binary);
        if (!ifs) {
            return false;
        }

        std::string serialized(
            (std::istreambuf_iterator<char>(ifs)),
            std::istreambuf_iterator<char>()
        );

        return data.ParseFromString(serialized);
    }

    bool remove(const std::string& key) {
        std::string file_path = get_file_path(key);
        return std::filesystem::remove(file_path);
    }

    bool exists(const std::string& key) {
        std::string file_path = get_file_path(key);
        return std::filesystem::exists(file_path);
    }

private:
    std::string get_file_path(const std::string& key) {
        return db_path_ + "/" + key + ".cic";
    }

    std::string db_path_;
};

// ============================================================================
// NMDBChannel Implementation
// ============================================================================

NMDBChannel::NMDBChannel(const std::string& db_path)
    : impl_(std::make_unique<Impl>(db_path)) {
}

NMDBChannel::~NMDBChannel() = default;

bool NMDBChannel::store(const std::string& key, const sintellix::CICData& data) {
    return impl_->store(key, data);
}

bool NMDBChannel::load(const std::string& key, sintellix::CICData& data) {
    return impl_->load(key, data);
}

bool NMDBChannel::remove(const std::string& key) {
    return impl_->remove(key);
}

bool NMDBChannel::exists(const std::string& key) {
    return impl_->exists(key);
}

// ============================================================================
// NMDBChannelManager Implementation
// ============================================================================

NMDBChannelManager::NMDBChannelManager(const NMDBChannelConfig& config) {
    // Initialize main channel
    main_channel_ = std::make_unique<NMDBChannel>(
        config.main_channel().database_path()
    );

    // Initialize peripheral channels
    for (const auto& peripheral_config : config.peripheral_channels()) {
        peripheral_channels_[peripheral_config.channel_name()] =
            std::make_unique<NMDBChannel>(peripheral_config.database_path());
    }
}

NMDBChannelManager::~NMDBChannelManager() = default;

bool NMDBChannelManager::store_main(
    const std::string& key,
    const sintellix::CICData& data
) {
    std::lock_guard<std::mutex> lock(mutex_);
    return main_channel_->store(key, data);
}

bool NMDBChannelManager::load_main(
    const std::string& key,
    sintellix::CICData& data
) {
    std::lock_guard<std::mutex> lock(mutex_);
    return main_channel_->load(key, data);
}

bool NMDBChannelManager::store_peripheral(
    const std::string& channel_name,
    const std::string& key,
    const sintellix::CICData& data
) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = peripheral_channels_.find(channel_name);
    if (it == peripheral_channels_.end()) {
        return false;
    }

    return it->second->store(key, data);
}

bool NMDBChannelManager::load_peripheral(
    const std::string& channel_name,
    const std::string& key,
    sintellix::CICData& data
) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = peripheral_channels_.find(channel_name);
    if (it == peripheral_channels_.end()) {
        return false;
    }

    return it->second->load(key, data);
}

bool NMDBChannelManager::remove_main(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    return main_channel_->remove(key);
}

bool NMDBChannelManager::remove_peripheral(
    const std::string& channel_name,
    const std::string& key
) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = peripheral_channels_.find(channel_name);
    if (it == peripheral_channels_.end()) {
        return false;
    }

    return it->second->remove(key);
}

bool NMDBChannelManager::exists_main(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    return main_channel_->exists(key);
}

bool NMDBChannelManager::exists_peripheral(
    const std::string& channel_name,
    const std::string& key
) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = peripheral_channels_.find(channel_name);
    if (it == peripheral_channels_.end()) {
        return false;
    }

    return it->second->exists(key);
}

} // namespace nmdb
