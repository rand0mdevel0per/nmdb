cmake_minimum_required(VERSION 3.15)
project(nmdb VERSION 0.1.0 LANGUAGES CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
if(MSVC)
    add_compile_options(/W4)
    # Disable /WX (warnings as errors) due to Protobuf DLL interface warnings
    # add_compile_options(/WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Find Protobuf (compatible with vcpkg)
find_package(protobuf CONFIG QUIET)
if(protobuf_FOUND)
    # vcpkg CONFIG mode - use target-based linking
    set(PROTOBUF_LIBRARIES protobuf::libprotobuf)
    # Get protobuf_generate_cpp function
    include(${Protobuf_DIR}/../../../share/protobuf/protobuf-generate.cmake OPTIONAL)
else()
    # Standard MODULE mode
    find_package(Protobuf REQUIRED)
    set(PROTOBUF_LIBRARIES ${Protobuf_LIBRARIES})
endif()

include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Generate Protobuf files
set(PROTO_FILES
    proto/channel.proto
    proto/message.proto
    proto/nmdb_channel_config.proto
    proto/cic_data.proto
)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

# NMDB library sources
set(NMDB_SOURCES
    src/channel_manager.cpp
    src/nmdb_channel_manager.cpp
    ${PROTO_SRCS}
)

# Add socket_server only on Unix-like systems
if(UNIX)
    list(APPEND NMDB_SOURCES src/socket_server.cpp)
endif()

# NMDB library headers
set(NMDB_HEADERS
    include/nmdb/socket_server.hpp
    include/nmdb/channel_manager.hpp
    include/nmdb/message_utils.hpp
    include/nmdb/nmdb_channel_manager.h
    ${PROTO_HDRS}
)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Create NMDB library
add_library(nmdb SHARED ${NMDB_SOURCES} ${NMDB_HEADERS})

# Link libraries
target_link_libraries(nmdb ${PROTOBUF_LIBRARIES})
if(UNIX)
    target_link_libraries(nmdb pthread)
endif()

# Install rules
install(TARGETS nmdb
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/nmdb
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)

install(FILES ${PROTO_HDRS}
    DESTINATION include/nmdb/proto
)

# Examples (optional)
option(BUILD_EXAMPLES "Build example programs" ON)
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Tests (optional)
option(BUILD_TESTS "Build tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
