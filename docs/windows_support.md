# Windows Support

## Current Status

NMDB has **partial Windows support** with the following limitations:

### ✅ Supported on Windows
- Core `ChannelManager` API
- Protocol Buffers message serialization
- Python client library (nmdb-py)
- Channel lifecycle management
- Message routing and callbacks

### ❌ Not Supported on Windows
- Unix domain socket server (`SocketServer`)
- Direct IPC via Unix sockets
- Socket-based client connections

## Why Unix Sockets Don't Work on Windows

Unix domain sockets are a Linux/Unix-specific IPC mechanism that use filesystem paths for addressing. Windows does not natively support Unix domain sockets (though Windows 10 1803+ has limited support via WSL).

The `SocketServer` component in NMDB is disabled on Windows builds:

```cpp
// In CMakeLists.txt
if(UNIX)
    list(APPEND NMDB_SOURCES src/socket_server.cpp)
endif()
```

## Alternative IPC Mechanisms for Windows

### 1. Named Pipes (Recommended)

Windows named pipes provide similar functionality to Unix sockets.

**Advantages:**
- Native Windows support
- Good performance for local IPC
- Similar API to sockets

**Example Implementation:**

```cpp
// Server side (C++)
#include <windows.h>

HANDLE pipe = CreateNamedPipe(
    "\\\\.\\pipe\\nmdb_channel",
    PIPE_ACCESS_DUPLEX,
    PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
    PIPE_UNLIMITED_INSTANCES,
    4096, 4096, 0, NULL
);

if (ConnectNamedPipe(pipe, NULL)) {
    // Handle client connection
}
```

```python
# Client side (Python)
import win32pipe
import win32file

handle = win32file.CreateFile(
    r'\\.\pipe\nmdb_channel',
    win32file.GENERIC_READ | win32file.GENERIC_WRITE,
    0, None,
    win32file.OPEN_EXISTING,
    0, None
)

# Send/receive data
win32file.WriteFile(handle, b"Hello")
result, data = win32file.ReadFile(handle, 4096)
```

### 2. Memory-Mapped Files

Shared memory for high-performance IPC.

**Advantages:**
- Fastest IPC method
- Zero-copy data sharing
- Cross-platform (with abstraction)

**Example:**

```cpp
// Server creates shared memory
HANDLE hMapFile = CreateFileMapping(
    INVALID_HANDLE_VALUE,
    NULL,
    PAGE_READWRITE,
    0,
    4096,
    "nmdb_shared_mem"
);

LPVOID pBuf = MapViewOfFile(
    hMapFile,
    FILE_MAP_ALL_ACCESS,
    0, 0, 4096
);
```

```python
# Client accesses shared memory
import mmap
import os

# On Windows, use named shared memory
shm = mmap.mmap(-1, 4096, "nmdb_shared_mem")
shm.write(b"Hello")
```

### 3. TCP Sockets (Localhost)

Use TCP sockets bound to localhost for IPC.

**Advantages:**
- Cross-platform
- Easy to implement
- Works with existing socket code

**Disadvantages:**
- Slower than named pipes or shared memory
- Network stack overhead

**Example:**

```cpp
// Server binds to localhost
struct sockaddr_in addr;
addr.sin_family = AF_INET;
addr.sin_addr.s_addr = inet_addr("127.0.0.1");
addr.sin_port = htons(9000);
bind(sock, (struct sockaddr*)&addr, sizeof(addr));
```

## Building on Windows

### Prerequisites

- Visual Studio 2019 or later
- CMake 3.15+
- vcpkg (for dependencies)

### Install Dependencies

```powershell
# Install vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# Install Protobuf
.\vcpkg install protobuf:x64-windows
```

### Build NMDB

```powershell
# Clone repository
git clone https://github.com/rand0mdevel0per/nmdb.git
cd nmdb

# Configure with vcpkg
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake

# Build
cmake --build . --config Release
```

### Python Client on Windows

The Python client works on Windows:

```powershell
pip install nmdb-py
```

However, you'll need a custom server implementation using Windows IPC mechanisms.

## Future Windows Support

Planned enhancements for Windows:

1. **Named Pipe Backend**: Implement `NamedPipeServer` as Windows alternative to `SocketServer`
2. **Shared Memory Backend**: Add memory-mapped file support for high-performance IPC
3. **Unified API**: Abstract IPC mechanism behind common interface
4. **Cross-Platform Client**: Python client that auto-detects platform and uses appropriate IPC

## Workarounds

### Using WSL (Windows Subsystem for Linux)

If you need full Unix socket support, use WSL:

```powershell
# Install WSL
wsl --install

# Run NMDB in WSL
wsl
cd /mnt/c/path/to/nmdb
mkdir build && cd build
cmake ..
cmake --build .
```

### Hybrid Approach

Run server in WSL, connect from Windows via TCP:

```python
# Windows client connecting to WSL server
from nmdb import ChannelClient

# Use TCP instead of Unix socket
client = ChannelClient("tcp://localhost:9000")
client.connect()
```

## Platform Detection

NMDB uses preprocessor macros for platform detection:

```cpp
#ifdef _WIN32
    // Windows-specific code
    #include <windows.h>
#else
    // Unix/Linux code
    #include <sys/socket.h>
    #include <sys/un.h>
#endif
```

## Compilation Flags

Windows-specific CMake configuration:

```cmake
if(MSVC)
    # Disable warnings as errors (Protobuf DLL warnings)
    add_compile_options(/W4)

    # Export all symbols for DLL
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()
```

## Known Issues

1. **Socket Server Disabled**: Unix socket server not available on Windows
2. **Protobuf DLL Warnings**: C4251 warnings about DLL interface (non-critical)
3. **Path Separators**: Use forward slashes or escape backslashes in paths

## Contributing Windows Support

We welcome contributions for Windows IPC support! Priority areas:

1. Implement `NamedPipeServer` class
2. Add memory-mapped file backend
3. Create unified IPC abstraction layer
4. Add Windows-specific tests
5. Update documentation with Windows examples

See [CONTRIBUTING.md](../CONTRIBUTING.md) for guidelines.

## References

- [Windows Named Pipes](https://docs.microsoft.com/en-us/windows/win32/ipc/named-pipes)
- [Windows Memory-Mapped Files](https://docs.microsoft.com/en-us/windows/win32/memory/file-mapping)
- [WSL Documentation](https://docs.microsoft.com/en-us/windows/wsl/)
