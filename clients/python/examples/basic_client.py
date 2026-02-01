#!/usr/bin/env python3
"""
Basic NMDB Python client example.

This example demonstrates how to connect to an NMDB channel and send/receive messages.
"""

import sys
import time
from nmdb import NMDBClient


def main():
    # Connect to NMDB channel
    socket_path = "/tmp/nmdb/c1.sock"
    print(f"Connecting to {socket_path}...")

    client = NMDBClient(socket_path)
    if not client.connect():
        print("Failed to connect!")
        return 1

    print("Connected successfully!")

    # Set message callback
    def on_message(data: bytes):
        try:
            text = data.decode('utf-8')
            print(f"Received: {text}")
        except Exception as e:
            print(f"Received binary data: {len(data)} bytes")

    client.set_message_callback(on_message)

    # Send some messages
    print("\nSending messages...")
    client.send_text("Hello, NMDB!")
    client.send_text("This is a test message.")
    client.send_text("Python client works!")

    # Keep running for a while
    print("\nListening for messages (press Ctrl+C to exit)...")
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("\nShutting down...")

    # Disconnect
    client.disconnect()
    print("Disconnected.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
