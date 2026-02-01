cmake_minimum_required(VERSION 3.15)
project(nmdb-c VERSION 0.1.0 LANGUAGES C)

# C Standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Sources
set(SOURCES
    src/nmdb_client.c
)

# Headers
set(HEADERS
    include/nmdb_client.h
)

# Include directories
include_directories(include)

# Create library
add_library(nmdb-c SHARED ${SOURCES} ${HEADERS})
target_link_libraries(nmdb-c pthread)

# Install rules
install(TARGETS nmdb-c
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES ${HEADERS}
    DESTINATION include
)

# Examples
option(BUILD_EXAMPLES "Build example programs" ON)
if(BUILD_EXAMPLES)
    add_executable(basic_client examples/basic_client.c)
    target_link_libraries(basic_client nmdb-c)
endif()
