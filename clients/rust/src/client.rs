//! NMDB client implementation

use std::path::PathBuf;
use tokio::io::{AsyncReadExt, AsyncWriteExt};
use tokio::net::UnixStream;
use bytes::Bytes;

/// NMDB client error type
#[derive(Debug)]
pub enum NmdbError {
    IoError(std::io::Error),
    NotConnected,
    InvalidData,
}

impl std::fmt::Display for NmdbError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            NmdbError::IoError(e) => write!(f, "IO error: {}", e),
            NmdbError::NotConnected => write!(f, "Not connected"),
            NmdbError::InvalidData => write!(f, "Invalid data"),
        }
    }
}

impl std::error::Error for NmdbError {}

impl From<std::io::Error> for NmdbError {
    fn from(err: std::io::Error) -> Self {
        NmdbError::IoError(err)
    }
}

/// NMDB client
pub struct NmdbClient {
    socket_path: PathBuf,
    stream: Option<UnixStream>,
}

impl NmdbClient {
    /// Create a new NMDB client
    pub fn new<P: Into<PathBuf>>(socket_path: P) -> Self {
        Self {
            socket_path: socket_path.into(),
            stream: None,
        }
    }

    /// Connect to NMDB channel
    pub async fn connect(&mut self) -> Result<(), NmdbError> {
        let stream = UnixStream::connect(&self.socket_path).await?;
        self.stream = Some(stream);
        Ok(())
    }

    /// Disconnect from NMDB channel
    pub fn disconnect(&mut self) {
        self.stream = None;
    }

    /// Check if connected
    pub fn is_connected(&self) -> bool {
        self.stream.is_some()
    }

    /// Send raw bytes
    pub async fn send_raw(&mut self, data: &[u8]) -> Result<(), NmdbError> {
        let stream = self.stream.as_mut().ok_or(NmdbError::NotConnected)?;
        stream.write_all(data).await?;
        Ok(())
    }

    /// Send text message
    pub async fn send_text(&mut self, text: &str) -> Result<(), NmdbError> {
        self.send_raw(text.as_bytes()).await
    }

    /// Receive data
    pub async fn receive(&mut self) -> Result<Bytes, NmdbError> {
        let stream = self.stream.as_mut().ok_or(NmdbError::NotConnected)?;
        let mut buffer = vec![0u8; 8192];
        let n = stream.read(&mut buffer).await?;

        if n == 0 {
            return Err(NmdbError::NotConnected);
        }

        buffer.truncate(n);
        Ok(Bytes::from(buffer))
    }
}
