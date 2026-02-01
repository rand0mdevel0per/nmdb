"""
Sintellix + NMDB Integration Example

Demonstrates the complete workflow:
1. Extract subchannel from sintellix model output
2. Wrap with text/image/audio shell
3. Insert into NMDB auxiliary channel

This example shows how the saltts project can use both libraries together.
"""

import numpy as np
from nmdb import NMDBChannelClient, NMDBChannelType, CICData, Channel, ChannelType

# Note: This assumes sintellix Python package is installed
# If not available, the example will show the conceptual workflow
try:
    from sintellix.channel_ops import (
        SubchannelExtractor,
        ChannelWrapper,
        extract_wrap_and_insert
    )
    SINTELLIX_AVAILABLE = True
except ImportError:
    SINTELLIX_AVAILABLE = False
    print("Warning: sintellix package not available, showing conceptual workflow only\n")


def example_complete_workflow():
    """
    Complete workflow: Model output -> Subchannel extraction -> Auxiliary channel.

    This is the primary use case for the saltts project.
    """
    print("=== Complete Workflow Example ===\n")

    # Step 1: Simulate sintellix model output
    print("Step 1: Generate model output (simulated)")
    model_output = np.random.randn(2048).astype(np.float64)
    print(f"Model output shape: {model_output.shape}")
    print(f"Model output dtype: {model_output.dtype}\n")

    if SINTELLIX_AVAILABLE:
        # Step 2: Extract subchannel from model output
        print("Step 2: Extract subchannel for audio")
        extractor = SubchannelExtractor()
        audio_subchannel = extractor.extract_subchannel(
            model_output,
            start_idx=1024,
            length=512,
            channel_name="audio_output"
        )
        print(f"Extracted subchannel: {audio_subchannel['name']}")
        print(f"Subchannel dimension: {audio_subchannel['dimension']}\n")

        # Step 3: Wrap with audio shell
        print("Step 3: Wrap subchannel with audio shell")
        wrapper = ChannelWrapper()
        wrapped_audio = wrapper.wrap_as_audio(
            audio_subchannel,
            format="raw",
            sample_rate=22050,
            channels=1,
            bit_depth=16
        )
        print(f"Wrapped channel: {wrapped_audio['name']}")
        print(f"Wrapped type: {wrapped_audio['type']}")
        print(f"Metadata: {wrapped_audio['metadata']}\n")
    else:
        # Conceptual workflow without sintellix
        print("Step 2-3: Extract and wrap (conceptual)")
        audio_data = model_output[1024:1536].tobytes()
        wrapped_audio = {
            'name': 'audio_output_audio',
            'type': 7,  # AUDIO_AUX
            'data': audio_data,
            'dimension': 512,
            'metadata': {
                'sample_rate': '22050',
                'channels': '1',
                'format': 'raw'
            }
        }
        print(f"Wrapped channel: {wrapped_audio['name']}\n")

    # Step 4: Insert into NMDB auxiliary channel
    print("Step 4: Insert into NMDB auxiliary channel")
    client = NMDBChannelClient("/tmp/nmdb")

    if client.connect_auxiliary("audio_aux", NMDBChannelType.CHANNEL_TYPE_AUDIO_AUX):
        print("Connected to audio_aux")

        # Create CIC data from wrapped channel
        cic = CICData()
        cic.cic_id = "saltts_audio_001"

        audio_channel = Channel(
            name=wrapped_audio['name'],
            channel_type=ChannelType.CHANNEL_TYPE_AUDIO,
            data=wrapped_audio['data'],
            dimension=wrapped_audio['dimension'],
            metadata=wrapped_audio['metadata']
        )
        cic.add_channel(audio_channel)

        success = client.send_cic_auxiliary("audio_aux", cic)
        print(f"Sent to auxiliary channel: {success}")

        client.disconnect_all()
    else:
        print("Failed to connect (server may not be running)")

    print()


def example_multimodal_extraction():
    """
    Extract multiple subchannels for different modalities.
    """
    print("=== Multimodal Extraction Example ===\n")

    # Simulate large model output with multiple modalities
    model_output = np.random.randn(4096).astype(np.float64)
    print(f"Model output shape: {model_output.shape}\n")

    if SINTELLIX_AVAILABLE:
        extractor = SubchannelExtractor()

        # Extract multiple subchannels
        subchannels = extractor.extract_multiple_subchannels(
            model_output,
            ranges=[
                (0, 1024, "text_embedding"),
                (1024, 1024, "image_embedding"),
                (2048, 1024, "audio_embedding"),
                (3072, 1024, "control_signals")
            ]
        )

        print(f"Extracted {len(subchannels)} subchannels:")
        for sc in subchannels:
            print(f"  - {sc['name']}: {sc['dimension']} dimensions")
        print()
    else:
        print("Conceptual: Extract 4 subchannels from model output")
        print("  - text_embedding: 1024 dimensions")
        print("  - image_embedding: 1024 dimensions")
        print("  - audio_embedding: 1024 dimensions")
        print("  - control_signals: 1024 dimensions\n")


def example_convenience_function():
    """
    Use the convenience function for complete pipeline.
    """
    print("=== Convenience Function Example ===\n")

    if not SINTELLIX_AVAILABLE:
        print("Sintellix not available, skipping this example\n")
        return

    # Simulate model output
    model_output = np.random.randn(2048).astype(np.float64)

    # Complete pipeline in one function call
    print("Executing complete pipeline with extract_wrap_and_insert()...")

    # Note: This will fail if NMDB server is not running
    # but demonstrates the API usage
    try:
        success = extract_wrap_and_insert(
            model_output=model_output,
            start_idx=0,
            length=1024,
            wrapper_type='audio',
            aux_channel_name='audio_aux',
            nmdb_connection=None,  # Uses placeholder
            sample_rate=22050,
            channels=1,
            name='tts_output'
        )
        print(f"Pipeline execution: {success}")
    except Exception as e:
        print(f"Pipeline execution failed (expected if server not running): {e}")

    print()


def example_cic_to_nmdb_workflow():
    """
    Demonstrate CIC data flow from creation to NMDB transmission.
    """
    print("=== CIC to NMDB Workflow Example ===\n")

    # Create CIC data with multiple channels
    cic = CICData()
    cic.cic_id = "workflow_example_001"

    # Add text channel
    text_data = "Generated text from model".encode('utf-8')
    cic.add_channel(Channel(
        "text_output",
        ChannelType.CHANNEL_TYPE_TEXT,
        text_data,
        len(text_data),
        {"encoding": "utf-8", "source": "sintellix_model"}
    ))

    # Add audio channel (simulated)
    audio_data = np.random.randn(512).astype(np.float64).tobytes()
    cic.add_channel(Channel(
        "audio_output",
        ChannelType.CHANNEL_TYPE_AUDIO,
        audio_data,
        512,
        {"sample_rate": "22050", "format": "raw"}
    ))

    print(f"Created CIC with {len(cic.channels)} channels")

    # Serialize CIC
    serialized = cic.serialize()
    print(f"Serialized size: {len(serialized)} bytes")

    # Send to NMDB (if server is running)
    client = NMDBChannelClient("/tmp/nmdb")
    if client.connect_main("main"):
        print("Connected to main channel")
        success = client.send_cic_main(cic)
        print(f"Sent CIC to main channel: {success}")
        client.disconnect_all()
    else:
        print("Could not connect to main channel (server may not be running)")

    print()


if __name__ == "__main__":
    print("Sintellix + NMDB Integration Examples")
    print("=" * 60 + "\n")

    if SINTELLIX_AVAILABLE:
        print("✓ Sintellix package available\n")
    else:
        print("✗ Sintellix package not available (showing conceptual examples)\n")

    example_complete_workflow()
    example_multimodal_extraction()
    example_convenience_function()
    example_cic_to_nmdb_workflow()

    print("=" * 60)
    print("\nIntegration examples completed!")
    print("\nNote: Some examples require NMDB server to be running.")
    print("Start server with: nmdb_server --config config.json")
