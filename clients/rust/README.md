# NMDB Rust Client

Rust client library for the Neural Message Data Bus (NMDB).

## Installation

Add to your `Cargo.toml`:

```toml
[dependencies]
nmdb-rs = "0.1.0"
tokio = { version = "1.35", features = ["full"] }
```

## Quick Start

```rust
use nmdb_rs::NmdbClient;

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    // Create client
    let mut client = NmdbClient::new("/tmp/nmdb/c1.sock");

    // Connect
    client.connect().await?;
    println!("Connected!");

    // Send message
    client.send_text("Hello, NMDB!").await?;

    // Receive message
    let data = client.receive().await?;
    println!("Received: {:?}", data);

    // Disconnect
    client.disconnect();

    Ok(())
}
```

## License

MIT License - see [LICENSE](../../LICENSE) for details.
