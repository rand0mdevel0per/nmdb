/**
 * @file message_utils.hpp
 * @brief Utility functions for NMDB message handling
 *
 * Provides helpers for creating, parsing, and manipulating NMDB messages,
 * including channel-in-channel (CIC) support.
 */

#ifndef NMDB_MESSAGE_UTILS_HPP
#define NMDB_MESSAGE_UTILS_HPP

#include <string>
#include <vector>
#include <cstdint>
#include <memory>

namespace nmdb {

// Forward declarations
struct NMDBMessage;
struct MessageHeader;
struct MessagePayload;
struct GenerationParams;

/**
 * @class MessageBuilder
 * @brief Builder for constructing NMDB messages with CIC support
 *
 * Example usage:
 * @code
 * MessageBuilder builder;
 * builder.set_source("c1")
 *        .set_target("c2")
 *        .set_text("Hello, world!")
 *        .add_nested_message(nested_msg)
 *        .build();
 * @endcode
 */
class MessageBuilder {
public:
    /**
     * @brief Constructor
     */
    MessageBuilder();

    /**
     * @brief Set source channel
     * @param channel_id Source channel ID
     * @return Reference to this builder
     */
    MessageBuilder& set_source(const std::string& channel_id);

    /**
     * @brief Set target channel
     * @param channel_id Target channel ID
     * @return Reference to this builder
     */
    MessageBuilder& set_target(const std::string& channel_id);

    /**
     * @brief Set message type
     * @param type Message type
     * @return Reference to this builder
     */
    MessageBuilder& set_type(int type);

    /**
     * @brief Set text payload
     * @param content Text content
     * @param encoding Encoding (default: "utf-8")
     * @param language Language code (default: "en")
     * @return Reference to this builder
     */
    MessageBuilder& set_text(const std::string& content,
                            const std::string& encoding = "utf-8",
                            const std::string& language = "en");

    /**
     * @brief Set generation parameters
     * @param temperature Sampling temperature
     * @param top_p Nucleus sampling threshold
     * @param max_tokens Maximum tokens to generate
     * @return Reference to this builder
     */
    MessageBuilder& set_generation_params(float temperature = 1.0f,
                                         float top_p = 1.0f,
                                         int max_tokens = 100);

    /**
     * @brief Add nested message (CIC support)
     * @param nested_data Serialized nested message data
     * @param size Data size
     * @return Reference to this builder
     */
    MessageBuilder& add_nested_message(const uint8_t* nested_data, size_t size);

    /**
     * @brief Build the message
     * @param out_data Output buffer for serialized message
     * @param out_size Output size
     * @return True if built successfully
     */
    bool build(std::vector<uint8_t>& out_data);

private:
    std::string source_channel_;
    std::string target_channel_;
    int message_type_;
    std::string text_content_;
    std::string text_encoding_;
    std::string text_language_;
    float temperature_;
    float top_p_;
    int max_tokens_;
    std::vector<std::vector<uint8_t>> nested_messages_;
};

/**
 * @class MessageParser
 * @brief Parser for extracting data from NMDB messages
 *
 * Example usage:
 * @code
 * MessageParser parser(message_data, message_size);
 * if (parser.parse()) {
 *     std::string text = parser.get_text();
 *     auto nested = parser.get_nested_messages();
 * }
 * @endcode
 */
class MessageParser {
public:
    /**
     * @brief Constructor
     * @param data Message data
     * @param size Data size
     */
    MessageParser(const uint8_t* data, size_t size);

    /**
     * @brief Parse the message
     * @return True if parsed successfully
     */
    bool parse();

    /**
     * @brief Get source channel
     * @return Source channel ID
     */
    std::string get_source() const { return source_channel_; }

    /**
     * @brief Get target channel
     * @return Target channel ID
     */
    std::string get_target() const { return target_channel_; }

    /**
     * @brief Get message type
     * @return Message type
     */
    int get_type() const { return message_type_; }

    /**
     * @brief Get text content
     * @return Text content (empty if not text message)
     */
    std::string get_text() const { return text_content_; }

    /**
     * @brief Get nested messages (CIC support)
     * @return Vector of nested message data
     */
    const std::vector<std::vector<uint8_t>>& get_nested_messages() const {
        return nested_messages_;
    }

    /**
     * @brief Check if message has nested messages
     * @return True if has nested messages
     */
    bool has_nested_messages() const { return !nested_messages_.empty(); }

private:
    const uint8_t* data_;
    size_t size_;
    std::string source_channel_;
    std::string target_channel_;
    int message_type_;
    std::string text_content_;
    std::vector<std::vector<uint8_t>> nested_messages_;
};

} // namespace nmdb

#endif // NMDB_MESSAGE_UTILS_HPP
