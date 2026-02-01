/**
 * @file nmdb_client.h
 * @brief NMDB C Client Library
 *
 * Provides a C API for connecting to NMDB channels.
 */

#ifndef NMDB_CLIENT_H
#define NMDB_CLIENT_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque client handle
 */
typedef struct nmdb_client nmdb_client_t;

/**
 * @brief Message callback function type
 * @param user_data User-provided data
 * @param data Message data
 * @param size Data size in bytes
 */
typedef void (*nmdb_message_callback_t)(void* user_data, const uint8_t* data, size_t size);

/**
 * @brief Create NMDB client
 * @param socket_path Path to Unix socket (e.g., "/tmp/nmdb/c1.sock")
 * @return Client handle, or NULL on failure
 */
nmdb_client_t* nmdb_client_create(const char* socket_path);

/**
 * @brief Destroy NMDB client
 * @param client Client handle
 */
void nmdb_client_destroy(nmdb_client_t* client);

/**
 * @brief Connect to NMDB channel
 * @param client Client handle
 * @return true if connected successfully, false otherwise
 */
bool nmdb_client_connect(nmdb_client_t* client);

/**
 * @brief Disconnect from NMDB channel
 * @param client Client handle
 */
void nmdb_client_disconnect(nmdb_client_t* client);

/**
 * @brief Check if client is connected
 * @param client Client handle
 * @return true if connected, false otherwise
 */
bool nmdb_client_is_connected(const nmdb_client_t* client);

/**
 * @brief Send raw bytes to channel
 * @param client Client handle
 * @param data Data to send
 * @param size Data size in bytes
 * @return true if sent successfully, false otherwise
 */
bool nmdb_client_send_raw(nmdb_client_t* client, const uint8_t* data, size_t size);

/**
 * @brief Send text message to channel
 * @param client Client handle
 * @param text Text content (null-terminated)
 * @return true if sent successfully, false otherwise
 */
bool nmdb_client_send_text(nmdb_client_t* client, const char* text);

/**
 * @brief Set message callback
 * @param client Client handle
 * @param callback Callback function
 * @param user_data User data to pass to callback
 */
void nmdb_client_set_callback(nmdb_client_t* client, nmdb_message_callback_t callback, void* user_data);

#ifdef __cplusplus
}
#endif

#endif // NMDB_CLIENT_H
