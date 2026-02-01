# NMDB C Client

C client library for the Neural Message Data Bus (NMDB).

## Building

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

## Installation

```bash
sudo cmake --install .
```

## Quick Start

```c
#include <nmdb_client.h>
#include <stdio.h>

void on_message(void* user_data, const uint8_t* data, size_t size) {
    printf("Received: %.*s\n", (int)size, data);
}

int main() {
    // Create client
    nmdb_client_t* client = nmdb_client_create("/tmp/nmdb/c1.sock");

    // Connect
    if (!nmdb_client_connect(client)) {
        fprintf(stderr, "Failed to connect\n");
        return 1;
    }

    // Set callback
    nmdb_client_set_callback(client, on_message, NULL);

    // Send message
    nmdb_client_send_text(client, "Hello, NMDB!");

    // Keep running...
    sleep(10);

    // Cleanup
    nmdb_client_destroy(client);
    return 0;
}
```

## License

MIT License - see [LICENSE](../../LICENSE) for details.
