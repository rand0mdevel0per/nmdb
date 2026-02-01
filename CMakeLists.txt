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
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Find Protobuf
find_package(Protobuf REQUIRED)
include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Generate Protobuf files
set(PROTO_FILES
    proto/channel.proto
    proto/message.proto
)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

# NMDB library sources
set(NMDB_SOURCES
    src/socket_server.cpp
    src/channel_manager.cpp
    ${PROTO_SRCS}
)

# NMDB library headers
set(NMDB_HEADERS
    include/nmdb/socket_server.hpp
    include/nmdb/channel_manager.hpp
    include/nmdb/message_utils.hpp
    ${PROTO_HDRS}
)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Create NMDB library
add_library(nmdb SHARED ${NMDB_SOURCES} ${NMDB_HEADERS})
target_link_libraries(nmdb ${Protobuf_LIBRARIES} pthread)

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
