"""
NMDB Python Client Library

A high-level Python interface for the Neural Message Data Bus (NMDB).
"""

from .client import NMDBClient
from .channel_client import NMDBChannelClient, NMDBChannelType
from .cic_data import CICData, Channel, ChannelType

__version__ = "0.1.0"
__all__ = [
    "NMDBClient",
    "NMDBChannelClient",
    "NMDBChannelType",
    "CICData",
    "Channel",
    "ChannelType",
]
