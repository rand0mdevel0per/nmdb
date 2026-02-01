"""
NMDB Channel Client

High-level client for interacting with NMDB channels including
main channels and auxiliary channels (TEXT_AUX, IMAGE_AUX, AUDIO_AUX).
"""

from typing import Optional, Dict, List
from enum import IntEnum
from .client import NMDBClient
from .cic_data import CICData, Channel, ChannelType


class NMDBChannelType(IntEnum):
    """NMDB channel types matching channel.proto."""
    CHANNEL_TYPE_UNSPECIFIED = 0
    CHANNEL_TYPE_TEXT = 1
    CHANNEL_TYPE_TTS = 2
    CHANNEL_TYPE_IMAGE = 3
    CHANNEL_TYPE_AUDIO = 4
    CHANNEL_TYPE_CUSTOM = 5
    CHANNEL_TYPE_CONTROL = 6
    CHANNEL_TYPE_TEXT_AUX = 7
    CHANNEL_TYPE_IMAGE_AUX = 8
    CHANNEL_TYPE_AUDIO_AUX = 9


class NMDBChannelClient:
    """
    High-level client for NMDB channel operations.

    Supports both main channels and auxiliary channels with CIC data.

    Example:
        >>> client = NMDBChannelClient("/tmp/nmdb")
        >>> client.connect_main("main_channel")
        >>>
        >>> # Send CIC data to main channel
        >>> cic = CICData()
        >>> cic.add_channel(Channel("text", ChannelType.CHANNEL_TYPE_TEXT, b"Hello"))
        >>> client.send_cic_main(cic)
        >>>
        >>> # Connect to auxiliary channel
        >>> client.connect_auxiliary("audio_aux", NMDBChannelType.CHANNEL_TYPE_AUDIO_AUX)
        >>> client.send_cic_auxiliary("audio_aux", audio_cic)
    """

    def __init__(self, base_path: str = "/tmp/nmdb"):
        """
        Initialize NMDB channel client.

        Args:
            base_path: Base directory for NMDB sockets
        """
        self.base_path = base_path
        self.main_client: Optional[NMDBClient] = None
        self.auxiliary_clients: Dict[str, NMDBClient] = {}

    def connect_main(self, channel_name: str = "main") -> bool:
        """
        Connect to main channel.

        Args:
            channel_name: Main channel name (default: "main")

        Returns:
            True if connected successfully
        """
        socket_path = f"{self.base_path}/{channel_name}.sock"
        self.main_client = NMDBClient(socket_path)
        return self.main_client.connect()

    def connect_auxiliary(
        self,
        channel_name: str,
        channel_type: NMDBChannelType
    ) -> bool:
        """
        Connect to auxiliary channel.

        Args:
            channel_name: Auxiliary channel name
            channel_type: Channel type (TEXT_AUX, IMAGE_AUX, or AUDIO_AUX)

        Returns:
            True if connected successfully
        """
        socket_path = f"{self.base_path}/{channel_name}.sock"
        client = NMDBClient(socket_path)

        if client.connect():
            self.auxiliary_clients[channel_name] = client
            return True
        return False

    def disconnect_all(self):
        """Disconnect from all channels."""
        if self.main_client:
            self.main_client.disconnect()
            self.main_client = None

        for client in self.auxiliary_clients.values():
            client.disconnect()
        self.auxiliary_clients.clear()

    def send_cic_main(self, cic_data: CICData) -> bool:
        """
        Send CIC data to main channel.

        Args:
            cic_data: CIC data to send

        Returns:
            True if sent successfully
        """
        if not self.main_client or not self.main_client.connected:
            return False

        serialized = cic_data.serialize()
        return self.main_client.send_raw(serialized)

    def send_cic_auxiliary(self, channel_name: str, cic_data: CICData) -> bool:
        """
        Send CIC data to auxiliary channel.

        Args:
            channel_name: Auxiliary channel name
            cic_data: CIC data to send

        Returns:
            True if sent successfully
        """
        if channel_name not in self.auxiliary_clients:
            return False

        client = self.auxiliary_clients[channel_name]
        if not client.connected:
            return False

        serialized = cic_data.serialize()
        return client.send_raw(serialized)

    def send_text_auxiliary(
        self,
        channel_name: str,
        text: str,
        encoding: str = "utf-8",
        language: str = "en"
    ) -> bool:
        """
        Send text to auxiliary channel.

        Args:
            channel_name: Auxiliary channel name
            text: Text content
            encoding: Text encoding
            language: Language code

        Returns:
            True if sent successfully
        """
        # Create CIC data with text channel
        cic = CICData()
        channel = Channel(
            name="text",
            channel_type=ChannelType.CHANNEL_TYPE_TEXT,
            data=text.encode(encoding),
            dimension=len(text),
            metadata={"encoding": encoding, "language": language}
        )
        cic.add_channel(channel)

        return self.send_cic_auxiliary(channel_name, cic)

    def send_audio_auxiliary(
        self,
        channel_name: str,
        audio_data: bytes,
        sample_rate: int = 16000,
        channels: int = 1,
        format: str = "raw"
    ) -> bool:
        """
        Send audio to auxiliary channel.

        Args:
            channel_name: Auxiliary channel name
            audio_data: Audio data bytes
            sample_rate: Sample rate in Hz
            channels: Number of audio channels
            format: Audio format

        Returns:
            True if sent successfully
        """
        # Create CIC data with audio channel
        cic = CICData()
        channel = Channel(
            name="audio",
            channel_type=ChannelType.CHANNEL_TYPE_AUDIO,
            data=audio_data,
            dimension=len(audio_data),
            metadata={
                "sample_rate": str(sample_rate),
                "channels": str(channels),
                "format": format
            }
        )
        cic.add_channel(channel)

        return self.send_cic_auxiliary(channel_name, cic)

    def __enter__(self):
        """Context manager entry."""
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        """Context manager exit."""
        self.disconnect_all()
