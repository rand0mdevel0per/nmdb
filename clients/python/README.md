# NMDB Python Client

Python client library for the Neural Message Data Bus (NMDB).

## Installation

```bash
pip install nmdb-py
```

Or install from source:

```bash
cd clients/python
pip install -e .
```

## Quick Start

```python
from nmdb import NMDBClient

# Connect to channel
client = NMDBClient("/tmp/nmdb/c1.sock")
client.connect()

# Send text message
client.send_text("Hello, NMDB!")

# Set message callback
def on_message(data: bytes):
    print(f"Received: {data.decode('utf-8')}")

client.set_message_callback(on_message)

# Keep running
import time
time.sleep(10)

# Disconnect
client.disconnect()
```

## Context Manager

```python
from nmdb import NMDBClient

with NMDBClient("/tmp/nmdb/c1.sock") as client:
    client.send_text("Hello, NMDB!")
    # Auto-disconnect on exit
```

## License

MIT License - see [LICENSE](../../LICENSE) for details.
