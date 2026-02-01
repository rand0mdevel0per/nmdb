# NMDB (Neural Message Data Bus)

**NMDB** is a high-performance, multi-modal communication bus designed for AI systems. It provides a flexible, protocol-based architecture for connecting neural network models with peripheral modules like TTS, text generators, and image generators.

## Features

- **Multi-Channel Architecture**: Dynamic creation and management of multiple communication channels
- **Unix Socket Communication**: High-performance, low-latency inter-process communication
- **Multi-Modal Support**: Native support for text, image, audio, and custom binary data
- **Channel-in-Channel (CIC)**: Nested message support for complex data packaging
- **Thread-Safe**: Concurrent operations with mutex-protected data structures
- **Protocol-First Design**: Language-agnostic Protobuf-based protocol
- **Multi-Language Support**: Client libraries for Python, C, and Rust
- **OpenAI-Compatible Parameters**: Support for temperature, top_p, and other generation parameters
- **Async I/O**: Non-blocking socket operations with dedicated I/O threads

## Architecture

```
┌─────────────┐     Unix Socket      ┌──────────────┐
│  Sintellix  │ ←──→ c1.sock ←──→   │     TTS      │
│   (Core)    │                      │   Module     │
│             │ ←──→ c2.sock ←──→   │    Text      │
│   Model     │                      │  Generator   │
│             │ ←──→ c3.sock ←──→   │    Image     │
└─────────────┘                      │  Generator   │
                                     └──────────────┘
```

NMDB acts as a communication bus between the core neural network model and peripheral modules, enabling real-time multi-modal interactions for AI-VTuber and other applications.

## Installation

### Prerequisites

- CMake 3.15 or higher
- C++20 compatible compiler (GCC 10+, Clang 11+, MSVC 2019+)
- Protocol Buffers 3.0 or higher
- pthread (Linux/Unix)

### Building from Source

```bash
# Clone the repository
git clone https://github.com/yourusername/nmdb.git
cd nmdb

# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
cmake --build . --config Release

# Install (optional)
sudo cmake --install .
```

### Building with Examples

```bash
cmake -DBUILD_EXAMPLES=ON ..
cmake --build . --config Release
```

## Quick Start

### C++ Server Example

```cpp
#include <nmdb/channel_manager.hpp>
#include <iostream>

int main() {
    // Create channel manager
    nmdb::ChannelManager manager("/tmp/nmdb");

    // Create channels
    manager.create_channel("c1", 32);  // TTS channel
    manager.create_channel("c2", 32);  // Text channel

    // Set message callback
    manager.set_message_callback([](const std::string& channel_id,
                                    int client_id,
                                    const uint8_t* data,
                                    size_t size) {
        std::cout << "Received message on " << channel_id
                  << " from client " << client_id << std::endl;
    });

    // Keep running
    std::this_thread::sleep_for(std::chrono::hours(1));

    return 0;
}
```

## Multi-Language Support

NMDB provides client libraries for multiple programming languages:

- **Python**: `nmdb-py` - High-level Pythonic API
- **C**: `libnmdb-c` - Low-level C API for embedded systems
- **Rust**: `nmdb-rs` - Safe Rust bindings with zero-cost abstractions

See the `clients/` directory for language-specific documentation and examples.

## Protocol

NMDB uses Protocol Buffers for message serialization. The protocol supports:

- Multi-modal data (text, image, audio, custom binary)
- OpenAI-compatible generation parameters
- Channel-in-channel (CIC) nested messages
- Extensible metadata fields

See `proto/message.proto` for the complete protocol definition.

## License

MIT License - see [LICENSE](LICENSE) for details.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## Acknowledgments

Part of the Sintellix project - a next-generation neural network architecture for AI systems.
```
