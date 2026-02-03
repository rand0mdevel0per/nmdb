# Python Client Guide

## Installation

Install the nmdb-py package from PyPI:

```bash
pip install nmdb-py
```

## Basic Usage

### Connecting to a Channel

```python
from nmdb import ChannelClient

# Create client with socket path
client = ChannelClient("/tmp/nmdb/my_channel.sock")

# Connect to the channel
if client.connect():
    print("Connected successfully")
else:
    print("Connection failed")
```

### Sending Messages

```python
# Send text message
message = b"Hello NMDB"
if client.send_message(message):
    print("Message sent")

# Send binary data
import struct
data = struct.pack('i', 42)
client.send_message(data)
```

### Receiving Messages

```python
# Blocking receive (wait forever)
response = client.receive_message()
print(f"Received: {response}")

# Receive with timeout (5 seconds)
try:
    response = client.receive_message(timeout=5.0)
    print(f"Received: {response}")
except TimeoutError:
    print("No message received within timeout")
```

### Disconnecting

```python
# Always disconnect when done
client.disconnect()
```

## Working with Protocol Buffers

### Using CICData

```python
from nmdb import CICData

# Create CIC data with embedding
cic = CICData(emb=[0.1, 0.2, 0.3, 0.4])

# Create nested CIC data
nested1 = CICData(emb=[1.0, 2.0])
nested2 = CICData(emb=[3.0, 4.0])
parent = CICData(nested=[nested1, nested2])

# Serialize to bytes
data = cic.to_bytes()

# Send over channel
client.send_message(data)

# Receive and deserialize
response = client.receive_message(timeout=5.0)
received_cic = CICData.from_bytes(response)
print(f"Embedding: {received_cic.emb}")
```

### Using Message Protocol

```python
from nmdb.proto import message_pb2

# Create message
msg = message_pb2.Message()
msg.text = "Hello from Python"

# Add generation parameters
msg.params.temperature = 0.7
msg.params.top_p = 0.9
msg.params.max_tokens = 100

# Serialize and send
client.send_message(msg.SerializeToString())

# Receive and parse
response = client.receive_message(timeout=5.0)
response_msg = message_pb2.Message()
response_msg.ParseFromString(response)
print(f"Response: {response_msg.text}")
```

## Advanced Usage

### Context Manager

```python
from nmdb import ChannelClient

# Use with context manager for automatic cleanup
with ChannelClient("/tmp/nmdb/channel.sock") as client:
    client.send_message(b"Hello")
    response = client.receive_message(timeout=5.0)
    print(response)
# Automatically disconnected
```

### Error Handling

```python
from nmdb import ChannelClient

try:
    client = ChannelClient("/tmp/nmdb/channel.sock")
    client.connect()

    client.send_message(b"Test message")
    response = client.receive_message(timeout=5.0)

except ConnectionError as e:
    print(f"Connection error: {e}")
except TimeoutError as e:
    print(f"Timeout: {e}")
except Exception as e:
    print(f"Unexpected error: {e}")
finally:
    if client:
        client.disconnect()
```

### Multi-threaded Usage

```python
import threading
from nmdb import ChannelClient

def worker(channel_path, message):
    # Each thread needs its own client instance
    client = ChannelClient(channel_path)
    client.connect()

    client.send_message(message)
    response = client.receive_message(timeout=5.0)
    print(f"Thread {threading.current_thread().name}: {response}")

    client.disconnect()

# Create multiple threads
threads = []
for i in range(5):
    t = threading.Thread(
        target=worker,
        args=("/tmp/nmdb/channel.sock", f"Message {i}".encode())
    )
    threads.append(t)
    t.start()

# Wait for all threads
for t in threads:
    t.join()
```

## Examples

### Text Generation Client

```python
from nmdb import ChannelClient
from nmdb.proto import message_pb2

def generate_text(prompt, temperature=0.7, max_tokens=100):
    client = ChannelClient("/tmp/nmdb/text_gen.sock")
    client.connect()

    # Create request
    request = message_pb2.Message()
    request.text = prompt
    request.params.temperature = temperature
    request.params.max_tokens = max_tokens

    # Send and receive
    client.send_message(request.SerializeToString())
    response_data = client.receive_message(timeout=30.0)

    # Parse response
    response = message_pb2.Message()
    response.ParseFromString(response_data)

    client.disconnect()
    return response.text

# Usage
result = generate_text("Once upon a time", temperature=0.8, max_tokens=200)
print(result)
```

### Image Processing Client

```python
from nmdb import ChannelClient
from nmdb.proto import message_pb2
import cv2

def process_image(image_path):
    # Read image
    img = cv2.imread(image_path)
    _, img_encoded = cv2.imencode('.jpg', img)
    img_bytes = img_encoded.tobytes()

    # Create message
    msg = message_pb2.Message()
    msg.image = img_bytes

    # Send to processing channel
    client = ChannelClient("/tmp/nmdb/image_proc.sock")
    client.connect()
    client.send_message(msg.SerializeToString())

    # Receive processed result
    response_data = client.receive_message(timeout=10.0)
    response = message_pb2.Message()
    response.ParseFromString(response_data)

    client.disconnect()
    return response

# Usage
result = process_image("input.jpg")
print(f"Processing result: {result.text}")
```

### Streaming Client

```python
from nmdb import ChannelClient
from nmdb.proto import message_pb2
import time

def streaming_generate(prompt):
    client = ChannelClient("/tmp/nmdb/stream.sock")
    client.connect()

    # Send initial prompt
    request = message_pb2.Message()
    request.text = prompt
    client.send_message(request.SerializeToString())

    # Receive streaming responses
    full_text = ""
    while True:
        try:
            response_data = client.receive_message(timeout=1.0)
            response = message_pb2.Message()
            response.ParseFromString(response_data)

            if response.text == "<END>":
                break

            full_text += response.text
            print(response.text, end='', flush=True)

        except TimeoutError:
            break

    client.disconnect()
    return full_text

# Usage
result = streaming_generate("Write a story about")
```

## Best Practices

### 1. Connection Reuse

Reuse connections instead of creating new ones for each message:

```python
# Good: Reuse connection
client = ChannelClient("/tmp/nmdb/channel.sock")
client.connect()

for i in range(100):
    client.send_message(f"Message {i}".encode())
    response = client.receive_message(timeout=5.0)

client.disconnect()

# Bad: Create new connection each time
for i in range(100):
    client = ChannelClient("/tmp/nmdb/channel.sock")
    client.connect()
    client.send_message(f"Message {i}".encode())
    response = client.receive_message(timeout=5.0)
    client.disconnect()
```

### 2. Timeout Configuration

Always set appropriate timeouts:

```python
# Short timeout for quick operations
response = client.receive_message(timeout=1.0)

# Longer timeout for heavy processing
response = client.receive_message(timeout=30.0)

# No timeout (blocking) - use with caution
response = client.receive_message()
```

### 3. Error Recovery

Implement retry logic for transient failures:

```python
import time

def send_with_retry(client, message, max_retries=3):
    for attempt in range(max_retries):
        try:
            client.send_message(message)
            return client.receive_message(timeout=5.0)
        except (ConnectionError, TimeoutError) as e:
            if attempt == max_retries - 1:
                raise
            time.sleep(1.0 * (attempt + 1))  # Exponential backoff
```

### 4. Resource Cleanup

Always clean up resources:

```python
# Use try-finally
client = ChannelClient("/tmp/nmdb/channel.sock")
try:
    client.connect()
    # ... operations ...
finally:
    client.disconnect()

# Or use context manager
with ChannelClient("/tmp/nmdb/channel.sock") as client:
    # ... operations ...
```

## Troubleshooting

### Connection Refused

```python
# Error: Connection refused
# Solution: Ensure server is running and socket path is correct
import os
socket_path = "/tmp/nmdb/channel.sock"
if not os.path.exists(socket_path):
    print(f"Socket file not found: {socket_path}")
```

### Timeout Errors

```python
# Error: TimeoutError
# Solution: Increase timeout or check server responsiveness
try:
    response = client.receive_message(timeout=5.0)
except TimeoutError:
    print("Server not responding, check server logs")
```

### Serialization Errors

```python
# Error: ParseError
# Solution: Ensure both sides use same protobuf schema
from google.protobuf.message import DecodeError

try:
    msg = message_pb2.Message()
    msg.ParseFromString(data)
except DecodeError as e:
    print(f"Invalid protobuf data: {e}")
```

## Performance Tips

1. **Batch Messages**: Send multiple items in one message when possible
2. **Reuse Connections**: Keep connections open for multiple operations
3. **Use Binary Format**: Protobuf is more efficient than JSON
4. **Set Appropriate Timeouts**: Avoid unnecessary waiting
5. **Thread Safety**: Use one client per thread, don't share clients

## API Reference

See [API Reference](api_reference.md) for complete API documentation.
