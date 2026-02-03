# Getting Started with NMDB

## Installation

### Python Client

```bash
pip install nmdb-py
```

### C++ Library

```bash
git clone https://github.com/rand0mdevel0per/nmdb.git
cd nmdb
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

## Quick Example

### Python Client

```python
from nmdb import ChannelClient

# Connect to channel
client = ChannelClient("/tmp/nmdb/c1.sock")
client.connect()

# Send message
client.send_message(b"Hello NMDB")

# Receive response
response = client.receive_message(timeout=5.0)
print(response)

# Disconnect
client.disconnect()
```

### C++ Server

```cpp
#include <nmdb/channel_manager.hpp>

int main() {
    // Create manager
    nmdb::ChannelManager manager("/tmp/nmdb");

    // Create channel
    manager.create_channel("c1", 32);

    // Set callback
    manager.set_message_callback([](const std::string& channel_id,
                                    int client_id,
                                    const uint8_t* data,
                                    size_t size) {
        std::cout << "Received: " << std::string((char*)data, size) << std::endl;
    });

    // Keep running
    std::this_thread::sleep_for(std::chrono::hours(1));
    return 0;
}
```

## Next Steps

- Read [Architecture](architecture.md) for design details
- Check [API Reference](api_reference.md) for complete API
- See [Python Client Guide](python_client.md) for Python usage
- Review [Windows Support](windows_support.md) for Windows-specific info
