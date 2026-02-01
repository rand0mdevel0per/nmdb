//! Basic NMDB Rust client example

use nmdb_rs::NmdbClient;
use std::error::Error;

#[tokio::main]
async fn main() -> Result<(), Box<dyn Error>> {
    let socket_path = std::env::args()
        .nth(1)
        .unwrap_or_else(|| "/tmp/nmdb/c1.sock".to_string());

    println!("Connecting to {}...", socket_path);

    // Create and connect client
    let mut client = NmdbClient::new(socket_path);
    client.connect().await?;

    println!("Connected successfully!");

    // Send some messages
    println!("\nSending messages...");
    client.send_text("Hello, NMDB!").await?;
    client.send_text("This is a test message.").await?;
    client.send_text("Rust client works!").await?;

    println!("\nListening for messages (press Ctrl+C to exit)...");

    // Receive loop
    loop {
        match client.receive().await {
            Ok(data) => {
                if let Ok(text) = String::from_utf8(data.to_vec()) {
                    println!("Received: {}", text);
                } else {
                    println!("Received binary data: {} bytes", data.len());
                }
            }
            Err(e) => {
                eprintln!("Error: {}", e);
                break;
            }
        }
    }

    // Disconnect
    println!("\nShutting down...");
    client.disconnect();
    println!("Disconnected.");

    Ok(())
}
