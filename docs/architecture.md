# NMDB Architecture

## Overview

NMDB (Neural Message Data Bus) is a high-performance, multi-modal communication bus designed for AI systems. It provides a flexible, protocol-based architecture for connecting neural network models with peripheral modules.

## Core Components

### 1. Channel Manager

The `ChannelManager` is the central component that manages multiple communication channels.

**Responsibilities:**
- Create and destroy channels dynamically
- Route messages between clients and channels
- Manage channel lifecycle
- Handle client connections

**Key Features:**
- Thread-safe operations with mutex protection
- Dynamic channel creation/deletion
- Client connection management
- Message routing and callbacks

### 2. Channel

Each `Channel` represents an independent communication endpoint.

**Properties:**
- Unique channel ID
- Maximum client capacity
- Socket server for client connections
- Message queue for async processing

**Lifecycle:**
1. Creation: Allocated by ChannelManager
2. Active: Accepting client connections and processing messages
3. Destruction: Cleanup and resource release

### 3. Socket Server (Unix/Linux only)

The `SocketServer` handles low-level socket communication.

**Features:**
- Unix domain sockets for IPC
- Non-blocking I/O with epoll (Linux)
- Dedicated accept and I/O threads
- Automatic client cleanup

**Note:** On Windows, socket server is disabled. Use alternative IPC mechanisms (named pipes, TCP sockets, or memory-mapped files).

### 4. Protocol Buffers

NMDB uses Protocol Buffers for message serialization.

**Message Types:**
- `Message`: Main message container
- `CICData`: Channel-in-Channel nested data
- `NMDBChannelConfig`: Channel configuration

**Benefits:**
- Language-agnostic
- Efficient binary serialization
- Schema evolution support
- Type safety

## Architecture Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                     Channel Manager                          │
│  ┌────────────┐  ┌────────────┐  ┌────────────┐            │
│  │ Channel c1 │  │ Channel c2 │  │ Channel c3 │            │
│  │            │  │            │  │            │            │
│  │ Socket     │  │ Socket     │  │ Socket     │            │
│  │ Server     │  │ Server     │  │ Server     │            │
│  └─────┬──────┘  └─────┬──────┘  └─────┬──────┘            │
└────────┼───────────────┼───────────────┼───────────────────┘
         │               │               │
    Unix Socket     Unix Socket     Unix Socket
         │               │               │
    ┌────▼────┐     ┌────▼────┐     ┌────▼────┐
    │ Client  │     │ Client  │     │ Client  │
    │ (TTS)   │     │ (Text)  │     │ (Image) │
    └─────────┘     └─────────┘     └─────────┘
```

## Data Flow

### Message Sending

1. Client connects to channel socket
2. Client serializes message using Protobuf
3. Client sends message over socket
4. SocketServer receives message
5. Channel processes message
6. ChannelManager invokes callback
7. Application handles message

### Message Receiving

1. Application creates response message
2. Message serialized to Protobuf
3. ChannelManager routes to target channel
4. Channel sends to target client
5. Client receives and deserializes message

## Threading Model

### Main Thread
- Channel management
- Configuration updates
- Callback invocation

### Accept Thread (per channel)
- Accept new client connections
- Register clients with channel
- Handle connection errors

### I/O Thread (per channel)
- Read messages from clients
- Write messages to clients
- Handle client disconnections
- Non-blocking I/O operations

## Memory Management

### Channel Storage
- Channels stored in `std::unordered_map`
- Automatic cleanup on destruction
- Thread-safe access with mutex

### Client Management
- Client list per channel
- Automatic cleanup on disconnect
- Connection state tracking

### Message Buffers
- Dynamic allocation for variable-size messages
- Protobuf handles serialization buffers
- Automatic cleanup after processing

## Platform Support

### Linux/Unix
- Full support with Unix domain sockets
- epoll for efficient I/O multiplexing
- pthread for threading

### Windows
- Socket server disabled (Unix sockets not available)
- Alternative IPC mechanisms recommended:
  - Named pipes
  - TCP sockets (localhost)
  - Memory-mapped files
- Core functionality (ChannelManager, Protocol) fully supported

## Performance Characteristics

### Latency
- Unix socket: < 1ms for local IPC
- Protobuf serialization: ~10-100μs for typical messages
- Thread synchronization: < 10μs with mutex

### Throughput
- Single channel: ~100K messages/sec
- Multiple channels: Linear scaling with CPU cores
- Limited by socket buffer size and CPU

### Scalability
- Channels: Limited by file descriptors (~1000s)
- Clients per channel: Configurable (default 32)
- Message size: Limited by available memory

## Security Considerations

### Unix Socket Permissions
- Socket files created with 0755 permissions
- Accessible to same user by default
- Can be restricted with filesystem permissions

### Message Validation
- Protobuf provides schema validation
- Application should validate message content
- No built-in authentication/encryption

### Resource Limits
- Max clients per channel prevents DoS
- Message size limits prevent memory exhaustion
- Connection timeouts prevent resource leaks

## Future Enhancements

1. **Windows IPC Support**: Implement named pipes or memory-mapped files
2. **Authentication**: Add client authentication mechanism
3. **Encryption**: Support TLS for secure communication
4. **Message Compression**: Reduce bandwidth for large messages
5. **Distributed Mode**: Support remote channels over TCP
6. **Monitoring**: Add metrics and health checks
7. **Load Balancing**: Distribute clients across multiple channels

## References

- Protocol Buffers: https://developers.google.com/protocol-buffers
- Unix Domain Sockets: https://man7.org/linux/man-pages/man7/unix.7.html
- epoll: https://man7.org/linux/man-pages/man7/epoll.7.html
