"""
NMDB Message Builder and Parser

Provides high-level API for constructing and parsing NMDB messages.
"""

from typing import Optional, List, Dict, Any
import struct


class MessageBuilder:
    """
    Builder for constructing NMDB messages.

    Example:
        >>> builder = MessageBuilder()
        >>> builder.set_source("c1").set_target("c2")
        >>> builder.set_text("Hello, world!")
        >>> data = builder.build()
    """

    def __init__(self):
        """Initialize message builder."""
        self.source_channel: str = ""
        self.target_channel: str = ""
        self.message_type: int = 1  # REQUEST
        self.text_content: str = ""
        self.text_encoding: str = "utf-8"
        self.text_language: str = "en"
        self.temperature: float = 1.0
        self.top_p: float = 1.0
        self.max_tokens: int = 100
        self.nested_messages: List[bytes] = []

    def set_source(self, channel_id: str) -> "MessageBuilder":
        """Set source channel ID."""
        self.source_channel = channel_id
        return self

    def set_target(self, channel_id: str) -> "MessageBuilder":
        """Set target channel ID."""
        self.target_channel = channel_id
        return self

    def set_type(self, message_type: int) -> "MessageBuilder":
        """Set message type."""
        self.message_type = message_type
        return self

    def set_text(
        self,
        content: str,
        encoding: str = "utf-8",
        language: str = "en"
    ) -> "MessageBuilder":
        """Set text payload."""
        self.text_content = content
        self.text_encoding = encoding
        self.text_language = language
        return self

    def set_generation_params(
        self,
        temperature: float = 1.0,
        top_p: float = 1.0,
        max_tokens: int = 100
    ) -> "MessageBuilder":
        """Set generation parameters."""
        self.temperature = temperature
        self.top_p = top_p
        self.max_tokens = max_tokens
        return self

    def add_nested_message(self, nested_data: bytes) -> "MessageBuilder":
        """Add nested message (CIC support)."""
        self.nested_messages.append(nested_data)
        return self

    def build(self) -> bytes:
        """
        Build the message.

        Returns:
            Serialized message data
        """
        # Simple serialization format (will be replaced with Protobuf)
        # Format: [source_len][source][target_len][target][type][text_len][text]
        parts = []

        # Source channel
        source_bytes = self.source_channel.encode('utf-8')
        parts.append(struct.pack('I', len(source_bytes)))
        parts.append(source_bytes)

        # Target channel
        target_bytes = self.target_channel.encode('utf-8')
        parts.append(struct.pack('I', len(target_bytes)))
        parts.append(target_bytes)

        # Message type
        parts.append(struct.pack('I', self.message_type))

        # Text content
        text_bytes = self.text_content.encode(self.text_encoding)
        parts.append(struct.pack('I', len(text_bytes)))
        parts.append(text_bytes)

        return b''.join(parts)


class MessageParser:
    """
    Parser for extracting data from NMDB messages.

    Example:
        >>> parser = MessageParser(message_data)
        >>> if parser.parse():
        >>>     text = parser.get_text()
        >>>     print(text)
    """

    def __init__(self, data: bytes):
        """
        Initialize message parser.

        Args:
            data: Message data to parse
        """
        self.data = data
        self.source_channel: str = ""
        self.target_channel: str = ""
        self.message_type: int = 0
        self.text_content: str = ""
        self.nested_messages: List[bytes] = []

    def parse(self) -> bool:
        """
        Parse the message.

        Returns:
            True if parsed successfully, False otherwise
        """
        try:
            offset = 0

            # Parse source channel
            source_len = struct.unpack('I', self.data[offset:offset+4])[0]
            offset += 4
            self.source_channel = self.data[offset:offset+source_len].decode('utf-8')
            offset += source_len

            # Parse target channel
            target_len = struct.unpack('I', self.data[offset:offset+4])[0]
            offset += 4
            self.target_channel = self.data[offset:offset+target_len].decode('utf-8')
            offset += target_len

            # Parse message type
            self.message_type = struct.unpack('I', self.data[offset:offset+4])[0]
            offset += 4

            # Parse text content
            text_len = struct.unpack('I', self.data[offset:offset+4])[0]
            offset += 4
            self.text_content = self.data[offset:offset+text_len].decode('utf-8')

            return True
        except Exception as e:
            print(f"Parse error: {e}")
            return False

    def get_source(self) -> str:
        """Get source channel ID."""
        return self.source_channel

    def get_target(self) -> str:
        """Get target channel ID."""
        return self.target_channel

    def get_type(self) -> int:
        """Get message type."""
        return self.message_type

    def get_text(self) -> str:
        """Get text content."""
        return self.text_content

    def get_nested_messages(self) -> List[bytes]:
        """Get nested messages."""
        return self.nested_messages

    def has_nested_messages(self) -> bool:
        """Check if message has nested messages."""
        return len(self.nested_messages) > 0
