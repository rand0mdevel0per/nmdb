# NMDB API Reference

## C++ API

### ChannelManager

Main class for managing communication channels.

#### Constructor

```cpp
ChannelManager(const std::string& base_socket_dir)
```

**Parameters:**
- `base_socket_dir`: Base directory for socket files (e.g., "/tmp/nmdb")

**Example:**
```cpp
nmdb::ChannelManager manager("/tmp/nmdb");
```

#### Methods

##### create_channel

```cpp
bool create_channel(const std::string& channel_id, int max_clients = 32)
```

Create a new communication channel.

**Parameters:**
- `channel_id`: Unique identifier for the channel
- `max_clients`: Maximum number of concurrent clients (default: 32)

**Returns:** `true` if successful, `false` if channel already exists

**Example:**
```cpp
manager.create_channel("tts", 16);
manager.create_channel("text_gen", 32);
```

##### destroy_channel

```cpp
bool destroy_channel(const std::string& channel_id)
```

Destroy an existing channel and disconnect all clients.

**Parameters:**
- `channel_id`: ID of the channel to destroy

**Returns:** `true` if successful, `false` if channel doesn't exist

##### set_message_callback

```cpp
void set_message_callback(MessageCallback callback)
```

Set callback function for incoming messages.

**Callback Signature:**
```cpp
void callback(const std::string& channel_id,
              int client_id,
              const uint8_t* data,
              size_t size)
```

**Example:**
```cpp
manager.set_message_callback([](const std::string& channel_id,
                                int client_id,
                                const uint8_t* data,
                                size_t size) {
    std::cout << "Message from " << channel_id << std::endl;
});
```

##### send_message

```cpp
bool send_message(const std::string& channel_id,
                  int client_id,
                  const uint8_t* data,
                  size_t size)
```

Send message to specific client.

**Parameters:**
- `channel_id`: Target channel ID
- `client_id`: Target client ID
- `data`: Message data buffer
- `size`: Size of message data

**Returns:** `true` if successful

##### broadcast_message

```cpp
bool broadcast_message(const std::string& channel_id,
                       const uint8_t* data,
                       size_t size)
```

Broadcast message to all clients on a channel.

**Parameters:**
- `channel_id`: Target channel ID
- `data`: Message data buffer
- `size`: Size of message data

**Returns:** `true` if successful

### Channel

Represents a single communication channel.

#### Methods

##### start

```cpp
bool start()
```

Start the channel and begin accepting connections.

##### stop

```cpp
void stop()
```

Stop the channel and disconnect all clients.

##### get_client_count

```cpp
int get_client_count() const
```

Get number of connected clients.

## Python API

### Installation

```bash
pip install nmdb-py
```

### ChannelClient

Client for connecting to NMDB channels.

#### Constructor

```python
ChannelClient(socket_path: str)
```

**Parameters:**
- `socket_path`: Path to channel socket file

**Example:**
```python
from nmdb import ChannelClient

client = ChannelClient("/tmp/nmdb/c1.sock")
```

#### Methods

##### connect

```python
def connect() -> bool
```

Connect to the channel.

**Returns:** `True` if successful

##### disconnect

```python
def disconnect()
```

Disconnect from the channel.

##### send_message

```python
def send_message(data: bytes) -> bool
```

Send message to channel.

**Parameters:**
- `data`: Message data as bytes

**Returns:** `True` if successful

##### receive_message

```python
def receive_message(timeout: float = None) -> bytes
```

Receive message from channel.

**Parameters:**
- `timeout`: Timeout in seconds (None = blocking)

**Returns:** Message data as bytes

**Example:**
```python
client.connect()
client.send_message(b"Hello NMDB")
response = client.receive_message(timeout=5.0)
client.disconnect()
```

### CICData

Container for Channel-in-Channel data.

#### Constructor

```python
CICData(emb: list = None, nested: list = None)
```

**Parameters:**
- `emb`: Embedding vector (list of floats)
- `nested`: List of nested CICData objects

#### Methods

##### to_bytes

```python
def to_bytes() -> bytes
```

Serialize to bytes using Protobuf.

##### from_bytes

```python
@staticmethod
def from_bytes(data: bytes) -> CICData
```

Deserialize from bytes.

**Example:**
```python
from nmdb import CICData

# Create CIC data
cic = CICData(emb=[0.1, 0.2, 0.3])

# Serialize
data = cic.to_bytes()

# Deserialize
cic2 = CICData.from_bytes(data)
```

## Protocol Buffers

### Message

Main message container.

```protobuf
message Message {
    string text = 1;
    bytes image = 2;
    bytes audio = 3;
    CICData cic = 4;
    GenerationParams params = 5;
}
```

### CICData

Channel-in-Channel nested data.

```protobuf
message CICData {
    repeated double emb = 1;
    repeated CICData nested = 2;
}
```

### GenerationParams

OpenAI-compatible generation parameters.

```protobuf
message GenerationParams {
    float temperature = 1;
    float top_p = 2;
    int32 max_tokens = 3;
}
```

## Error Handling

### C++ Exceptions

NMDB uses return values for error handling. Check return values:

```cpp
if (!manager.create_channel("test", 32)) {
    std::cerr << "Failed to create channel" << std::endl;
}
```

### Python Exceptions

Python client raises exceptions on errors:

```python
try:
    client.connect()
except ConnectionError as e:
    print(f"Connection failed: {e}")
```

## Thread Safety

- `ChannelManager`: Thread-safe (mutex-protected)
- `Channel`: Thread-safe (mutex-protected)
- `SocketServer`: Thread-safe (dedicated threads)
- Python `ChannelClient`: Not thread-safe (use one per thread)

## Performance Tips

1. **Reuse connections**: Keep client connections open
2. **Batch messages**: Send multiple messages together
3. **Use callbacks**: Avoid polling for messages
4. **Limit clients**: Don't exceed max_clients per channel
5. **Message size**: Keep messages < 1MB for best performance
