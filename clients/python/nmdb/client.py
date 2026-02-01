"""
NMDB Python Client

Provides a high-level Python interface for connecting to NMDB channels.
"""

import socket
import struct
import threading
from typing import Optional, Callable, Dict, Any
from pathlib import Path


class NMDBClient:
    """
    NMDB client for connecting to Unix socket channels.

    Example:
        >>> client = NMDBClient("/tmp/nmdb/c1.sock")
        >>> client.connect()
        >>> client.send_text("Hello, world!")
        >>> client.disconnect()
    """

    def __init__(self, socket_path: str):
        """
        Initialize NMDB client.

        Args:
            socket_path: Path to Unix socket (e.g., "/tmp/nmdb/c1.sock")
        """
        self.socket_path = socket_path
        self.sock: Optional[socket.socket] = None
        self.connected = False
        self._receive_thread: Optional[threading.Thread] = None
        self._running = False
        self._message_callback: Optional[Callable[[bytes], None]] = None

    def connect(self) -> bool:
        """
        Connect to NMDB channel.

        Returns:
            True if connected successfully, False otherwise
        """
        try:
            self.sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
            self.sock.connect(self.socket_path)
            self.connected = True

            # Start receive thread
            self._running = True
            self._receive_thread = threading.Thread(target=self._receive_loop, daemon=True)
            self._receive_thread.start()

            return True
        except Exception as e:
            print(f"Failed to connect: {e}")
            return False

    def disconnect(self):
        """Disconnect from NMDB channel."""
        self._running = False
        if self._receive_thread:
            self._receive_thread.join(timeout=1.0)

        if self.sock:
            self.sock.close()
            self.sock = None

        self.connected = False

    def send_raw(self, data: bytes) -> bool:
        """
        Send raw bytes to channel.

        Args:
            data: Raw bytes to send

        Returns:
            True if sent successfully, False otherwise
        """
        if not self.connected or not self.sock:
            return False

        try:
            self.sock.sendall(data)
            return True
        except Exception as e:
            print(f"Failed to send: {e}")
            return False

    def send_text(self, text: str, encoding: str = "utf-8") -> bool:
        """
        Send text message to channel.

        Args:
            text: Text content
            encoding: Text encoding (default: "utf-8")

        Returns:
            True if sent successfully, False otherwise
        """
        data = text.encode(encoding)
        return self.send_raw(data)

    def set_message_callback(self, callback: Callable[[bytes], None]):
        """
        Set callback for received messages.

        Args:
            callback: Function to call when message is received
        """
        self._message_callback = callback

    def _receive_loop(self):
        """Internal receive loop running in separate thread."""
        buffer_size = 8192

        while self._running and self.sock:
            try:
                data = self.sock.recv(buffer_size)
                if not data:
                    # Connection closed
                    break

                if self._message_callback:
                    self._message_callback(data)
            except Exception as e:
                if self._running:
                    print(f"Receive error: {e}")
                break

        self.connected = False

    def __enter__(self):
        """Context manager entry."""
        self.connect()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        """Context manager exit."""
        self.disconnect()
