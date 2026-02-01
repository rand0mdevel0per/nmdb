"""
NMDB CIC Data Support

Provides Python classes for working with CIC (Common Interchange Container) data.
These classes match the protobuf definitions in proto/cic_data.proto.
"""

from typing import List, Dict, Optional
from enum import IntEnum
import struct


class ChannelType(IntEnum):
    """Channel type enumeration matching CIC proto definition."""
    CHANNEL_TYPE_UNSPECIFIED = 0
    CHANNEL_TYPE_TEXT = 1
    CHANNEL_TYPE_IMAGE = 2
    CHANNEL_TYPE_AUDIO = 3
    CHANNEL_TYPE_COMPOSITE = 4
    CHANNEL_TYPE_RAW = 5


class Channel:
    """
    Represents a single channel within CIC data.

    Attributes:
        name: Channel identifier
        type: Channel type (ChannelType enum)
        data: Channel data as bytes
        dimension: Data dimension
        metadata: Additional metadata as key-value pairs
    """

    def __init__(
        self,
        name: str = "",
        channel_type: ChannelType = ChannelType.CHANNEL_TYPE_UNSPECIFIED,
        data: bytes = b"",
        dimension: int = 0,
        metadata: Optional[Dict[str, str]] = None
    ):
        self.name = name
        self.type = channel_type
        self.data = data
        self.dimension = dimension
        self.metadata = metadata or {}

    def serialize(self) -> bytes:
        """Serialize channel to bytes."""
        parts = []

        # Name
        name_bytes = self.name.encode('utf-8')
        parts.append(struct.pack('I', len(name_bytes)))
        parts.append(name_bytes)

        # Type
        parts.append(struct.pack('I', self.type))

        # Data
        parts.append(struct.pack('I', len(self.data)))
        parts.append(self.data)

        # Dimension
        parts.append(struct.pack('I', self.dimension))

        # Metadata
        parts.append(struct.pack('I', len(self.metadata)))
        for key, value in self.metadata.items():
            key_bytes = key.encode('utf-8')
            value_bytes = value.encode('utf-8')
            parts.append(struct.pack('I', len(key_bytes)))
            parts.append(key_bytes)
            parts.append(struct.pack('I', len(value_bytes)))
            parts.append(value_bytes)

        return b''.join(parts)

    @staticmethod
    def deserialize(data: bytes, offset: int = 0) -> tuple['Channel', int]:
        """
        Deserialize channel from bytes.

        Returns:
            Tuple of (Channel object, new offset)
        """
        channel = Channel()

        # Name
        name_len = struct.unpack('I', data[offset:offset+4])[0]
        offset += 4
        channel.name = data[offset:offset+name_len].decode('utf-8')
        offset += name_len

        # Type
        channel.type = ChannelType(struct.unpack('I', data[offset:offset+4])[0])
        offset += 4

        # Data
        data_len = struct.unpack('I', data[offset:offset+4])[0]
        offset += 4
        channel.data = data[offset:offset+data_len]
        offset += data_len

        # Dimension
        channel.dimension = struct.unpack('I', data[offset:offset+4])[0]
        offset += 4

        # Metadata
        metadata_count = struct.unpack('I', data[offset:offset+4])[0]
        offset += 4
        for _ in range(metadata_count):
            key_len = struct.unpack('I', data[offset:offset+4])[0]
            offset += 4
            key = data[offset:offset+key_len].decode('utf-8')
            offset += key_len

            value_len = struct.unpack('I', data[offset:offset+4])[0]
            offset += 4
            value = data[offset:offset+value_len].decode('utf-8')
            offset += value_len

            channel.metadata[key] = value

        return channel, offset


class CICData:
    """
    CIC (Common Interchange Container) data structure.

    Attributes:
        channels: List of Channel objects
        cic_id: Unique CIC identifier
        timestamp: Creation timestamp (milliseconds)
        metadata: Additional metadata as key-value pairs
    """

    def __init__(
        self,
        channels: Optional[List[Channel]] = None,
        cic_id: str = "",
        timestamp: int = 0,
        metadata: Optional[Dict[str, str]] = None
    ):
        self.channels = channels or []
        self.cic_id = cic_id
        self.timestamp = timestamp
        self.metadata = metadata or {}

    def add_channel(self, channel: Channel) -> 'CICData':
        """Add a channel to this CIC data."""
        self.channels.append(channel)
        return self

    def get_channel(self, name: str) -> Optional[Channel]:
        """Get channel by name."""
        for channel in self.channels:
            if channel.name == name:
                return channel
        return None

    def get_channels_by_type(self, channel_type: ChannelType) -> List[Channel]:
        """Get all channels of a specific type."""
        return [ch for ch in self.channels if ch.type == channel_type]

    def serialize(self) -> bytes:
        """Serialize CIC data to bytes."""
        parts = []

        # Channels
        parts.append(struct.pack('I', len(self.channels)))
        for channel in self.channels:
            channel_data = channel.serialize()
            parts.append(struct.pack('I', len(channel_data)))
            parts.append(channel_data)

        # CIC ID
        cic_id_bytes = self.cic_id.encode('utf-8')
        parts.append(struct.pack('I', len(cic_id_bytes)))
        parts.append(cic_id_bytes)

        # Timestamp
        parts.append(struct.pack('Q', self.timestamp))

        # Metadata
        parts.append(struct.pack('I', len(self.metadata)))
        for key, value in self.metadata.items():
            key_bytes = key.encode('utf-8')
            value_bytes = value.encode('utf-8')
            parts.append(struct.pack('I', len(key_bytes)))
            parts.append(key_bytes)
            parts.append(struct.pack('I', len(value_bytes)))
            parts.append(value_bytes)

        return b''.join(parts)

    @staticmethod
    def deserialize(data: bytes) -> 'CICData':
        """Deserialize CIC data from bytes."""
        cic = CICData()
        offset = 0

        # Channels
        channel_count = struct.unpack('I', data[offset:offset+4])[0]
        offset += 4
        for _ in range(channel_count):
            channel_data_len = struct.unpack('I', data[offset:offset+4])[0]
            offset += 4
            channel, new_offset = Channel.deserialize(data, offset)
            cic.channels.append(channel)
            offset = new_offset

        # CIC ID
        cic_id_len = struct.unpack('I', data[offset:offset+4])[0]
        offset += 4
        cic.cic_id = data[offset:offset+cic_id_len].decode('utf-8')
        offset += cic_id_len

        # Timestamp
        cic.timestamp = struct.unpack('Q', data[offset:offset+8])[0]
        offset += 8

        # Metadata
        metadata_count = struct.unpack('I', data[offset:offset+4])[0]
        offset += 4
        for _ in range(metadata_count):
            key_len = struct.unpack('I', data[offset:offset+4])[0]
            offset += 4
            key = data[offset:offset+key_len].decode('utf-8')
            offset += key_len

            value_len = struct.unpack('I', data[offset:offset+4])[0]
            offset += 4
            value = data[offset:offset+value_len].decode('utf-8')
            offset += value_len

            cic.metadata[key] = value

        return cic
