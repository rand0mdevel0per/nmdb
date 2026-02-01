//! NMDB Rust Client Library
//!
//! Provides a safe Rust interface for the Neural Message Data Bus (NMDB).
//!
//! # Example
//!
//! ```no_run
//! use nmdb_rs::NmdbClient;
//!
//! #[tokio::main]
//! async fn main() -> Result<(), Box<dyn std::error::Error>> {
//!     let mut client = NmdbClient::new("/tmp/nmdb/c1.sock");
//!     client.connect().await?;
//!     client.send_text("Hello, NMDB!").await?;
//!     Ok(())
//! }
//! ```

mod client;

pub use client::NmdbClient;
pub use client::NmdbError;
