"""
NMDB CIC Data Example

Demonstrates how to use CIC (Common Interchange Container) data
with NMDB Python client.
"""

import time
from nmdb import CICData, Channel, ChannelType, NMDBChannelClient, NMDBChannelType


def example_basic_cic():
    """Example: Create and send basic CIC data."""
    print("=== Basic CIC Data Example ===\n")

    # Create CIC data with text channel
    cic = CICData()
    cic.cic_id = "example_001"
    cic.timestamp = int(time.time() * 1000)

    # Add text channel
    text_channel = Channel(
        name="user_input",
        channel_type=ChannelType.CHANNEL_TYPE_TEXT,
        data=b"Hello, NMDB!",
        dimension=12,
        metadata={"encoding": "utf-8", "language": "en"}
    )
    cic.add_channel(text_channel)

    # Serialize and deserialize
    serialized = cic.serialize()
    print(f"Serialized CIC size: {len(serialized)} bytes")

    deserialized = CICData.deserialize(serialized)
    print(f"Deserialized CIC ID: {deserialized.cic_id}")
    print(f"Number of channels: {len(deserialized.channels)}")
    print(f"First channel name: {deserialized.channels[0].name}")
    print(f"First channel data: {deserialized.channels[0].data.decode('utf-8')}\n")


def example_multimodal_cic():
    """Example: Create CIC data with multiple modalities."""
    print("=== Multimodal CIC Data Example ===\n")

    cic = CICData()
    cic.cic_id = "multimodal_001"
    cic.timestamp = int(time.time() * 1000)

    # Add text channel
    text_channel = Channel(
        name="text_input",
        channel_type=ChannelType.CHANNEL_TYPE_TEXT,
        data=b"Describe this image",
        dimension=18
    )
    cic.add_channel(text_channel)

    # Add image channel (placeholder data)
    image_data = b"\x00" * 1024  # Placeholder for actual image embedding
    image_channel = Channel(
        name="image_embedding",
        channel_type=ChannelType.CHANNEL_TYPE_IMAGE,
        data=image_data,
        dimension=1024,
        metadata={"format": "embedding", "model": "CLIP"}
    )
    cic.add_channel(image_channel)

    # Add audio channel (placeholder data)
    audio_data = b"\x00" * 512  # Placeholder for actual audio embedding
    audio_channel = Channel(
        name="audio_embedding",
        channel_type=ChannelType.CHANNEL_TYPE_AUDIO,
        data=audio_data,
        dimension=512,
        metadata={"format": "embedding", "model": "Wav2Vec2"}
    )
    cic.add_channel(audio_channel)

    print(f"Created multimodal CIC with {len(cic.channels)} channels:")
    for ch in cic.channels:
        print(f"  - {ch.name} ({ch.type.name}): {ch.dimension} dimensions")
    print()


def example_auxiliary_channels():
    """Example: Send data to auxiliary channels."""
    print("=== Auxiliary Channels Example ===\n")

    # Note: This example assumes NMDB server is running
    # with auxiliary channels configured

    client = NMDBChannelClient("/tmp/nmdb")

    # Connect to text auxiliary channel
    print("Connecting to text auxiliary channel...")
    if client.connect_auxiliary("text_aux", NMDBChannelType.CHANNEL_TYPE_TEXT_AUX):
        print("Connected to text_aux")

        # Send text to auxiliary channel
        success = client.send_text_auxiliary(
            "text_aux",
            "This is auxiliary text data",
            encoding="utf-8",
            language="en"
        )
        print(f"Sent text to auxiliary channel: {success}")
    else:
        print("Failed to connect to text_aux (server may not be running)")

    # Connect to audio auxiliary channel
    print("\nConnecting to audio auxiliary channel...")
    if client.connect_auxiliary("audio_aux", NMDBChannelType.CHANNEL_TYPE_AUDIO_AUX):
        print("Connected to audio_aux")

        # Send audio to auxiliary channel
        audio_data = b"\x00" * 1024  # Placeholder audio data
        success = client.send_audio_auxiliary(
            "audio_aux",
            audio_data,
            sample_rate=16000,
            channels=1,
            format="raw"
        )
        print(f"Sent audio to auxiliary channel: {success}")
    else:
        print("Failed to connect to audio_aux (server may not be running)")

    client.disconnect_all()
    print()


def example_channel_extraction():
    """Example: Extract specific channels from CIC data."""
    print("=== Channel Extraction Example ===\n")

    # Create CIC with multiple channels
    cic = CICData()
    cic.add_channel(Channel("text", ChannelType.CHANNEL_TYPE_TEXT, b"Hello"))
    cic.add_channel(Channel("image", ChannelType.CHANNEL_TYPE_IMAGE, b"\x00" * 512))
    cic.add_channel(Channel("audio", ChannelType.CHANNEL_TYPE_AUDIO, b"\x00" * 256))

    # Extract specific channel by name
    text_channel = cic.get_channel("text")
    if text_channel:
        print(f"Extracted text channel: {text_channel.data.decode('utf-8')}")

    # Extract all channels of a specific type
    image_channels = cic.get_channels_by_type(ChannelType.CHANNEL_TYPE_IMAGE)
    print(f"Found {len(image_channels)} image channel(s)")

    audio_channels = cic.get_channels_by_type(ChannelType.CHANNEL_TYPE_AUDIO)
    print(f"Found {len(audio_channels)} audio channel(s)")
    print()


def example_composite_channel():
    """Example: Create composite channel from multiple channels."""
    print("=== Composite Channel Example ===\n")

    # Create individual channels
    text_ch = Channel("text", ChannelType.CHANNEL_TYPE_TEXT, b"Hello")
    image_ch = Channel("image", ChannelType.CHANNEL_TYPE_IMAGE, b"\x00" * 512)
    audio_ch = Channel("audio", ChannelType.CHANNEL_TYPE_AUDIO, b"\x00" * 256)

    # Create composite channel
    composite_data = text_ch.serialize() + image_ch.serialize() + audio_ch.serialize()
    composite_channel = Channel(
        name="multimodal_composite",
        channel_type=ChannelType.CHANNEL_TYPE_COMPOSITE,
        data=composite_data,
        dimension=len(composite_data),
        metadata={"component_count": "3", "types": "text,image,audio"}
    )

    print(f"Created composite channel: {composite_channel.name}")
    print(f"Composite size: {composite_channel.dimension} bytes")
    print(f"Metadata: {composite_channel.metadata}")
    print()


if __name__ == "__main__":
    print("NMDB CIC Data Examples\n")
    print("=" * 50 + "\n")

    example_basic_cic()
    example_multimodal_cic()
    example_channel_extraction()
    example_composite_channel()
    example_auxiliary_channels()

    print("=" * 50)
    print("\nAll examples completed!")
