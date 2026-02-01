#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <mutex>
#include "nmdb_channel_config.pb.h"
#include "cic_data.pb.h"

namespace nmdb {

/**
 * @brief NMDB Channel
 *
 * Represents a single NMDB channel (main or peripheral)
 */
class NMDBChannel {
public:
    explicit NMDBChannel(const std::string& db_path);
    ~NMDBChannel();

    /**
     * @brief Store data in this channel
     */
    bool store(const std::string& key, const sintellix::CICData& data);

    /**
     * @brief Load data from this channel
     */
    bool load(const std::string& key, sintellix::CICData& data);

    /**
     * @brief Delete data from this channel
     */
    bool remove(const std::string& key);

    /**
     * @brief Check if key exists
     */
    bool exists(const std::string& key);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

/**
 * @brief NMDB Channel Manager
 *
 * Manages separate main and peripheral channels as documented in Section 6.3
 * of the technical specification.
 */
class NMDBChannelManager {
public:
    explicit NMDBChannelManager(const NMDBChannelConfig& config);
    ~NMDBChannelManager();

    /**
     * @brief Store data in main channel
     */
    bool store_main(const std::string& key, const sintellix::CICData& data);

    /**
     * @brief Load data from main channel
     */
    bool load_main(const std::string& key, sintellix::CICData& data);

    /**
     * @brief Store data in peripheral channel
     */
    bool store_peripheral(
        const std::string& channel_name,
        const std::string& key,
        const sintellix::CICData& data
    );

    /**
     * @brief Load data from peripheral channel
     */
    bool load_peripheral(
        const std::string& channel_name,
        const std::string& key,
        sintellix::CICData& data
    );

    /**
     * @brief Remove data from main channel
     */
    bool remove_main(const std::string& key);

    /**
     * @brief Remove data from peripheral channel
     */
    bool remove_peripheral(
        const std::string& channel_name,
        const std::string& key
    );

    /**
     * @brief Check if key exists in main channel
     */
    bool exists_main(const std::string& key);

    /**
     * @brief Check if key exists in peripheral channel
     */
    bool exists_peripheral(
        const std::string& channel_name,
        const std::string& key
    );

private:
    std::unique_ptr<NMDBChannel> main_channel_;
    std::unordered_map<std::string, std::unique_ptr<NMDBChannel>> peripheral_channels_;
    std::mutex mutex_;
};

} // namespace nmdb
