set(LIBRARY_NAME network_armory)

# -----------------------------------------
# Collect headers and sources
# -----------------------------------------
file(GLOB_RECURSE CONFIGURE_DEPENDS NETWORK_HEADERS 
    "${CMAKE_CURRENT_SOURCE_DIR}/client/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/client/asio/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/client/posix/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/server/posix/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/server/posix/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/factory.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/error.h"
)

set(NETWORK_SOURCES
    client/asio/tcp_client.cpp
    client/posix/tcp_client.cpp
    server/posix/tcp_server.cpp
    client/asio/udp_client.cpp
    server/posix/udp_server.cpp
)

# -----------------------------------------
# ASIO header-only target
# -----------------------------------------
add_library(asio INTERFACE)

find_package(Threads REQUIRED)

# Add Asio include directory
target_include_directories(asio INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/third-party/asio/asio/include
)

target_compile_definitions(asio INTERFACE ASIO_STANDALONE)

# -----------------------------------------
# Main library
# -----------------------------------------
add_library(${LIBRARY_NAME} STATIC
    ${NETWORK_SOURCES}
    ${NETWORK_HEADERS}
)

# -----------------------------------------
# Include paths
# -----------------------------------------
target_include_directories(${LIBRARY_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/client
    ${CMAKE_CURRENT_SOURCE_DIR}/server
)

# -----------------------------------------
# Link ASIO + Threads
# -----------------------------------------
target_link_libraries(${LIBRARY_NAME} PUBLIC
    asio
    Threads::Threads
)

# -----------------------------------------
# Keep headers visible to IDEs
# -----------------------------------------
target_sources(${LIBRARY_NAME} PRIVATE ${NETWORK_HEADERS})

# -----------------------------------------
# Alias for convenience
# -----------------------------------------
add_library(${LIB_ALIAS} ALIAS ${LIBRARY_NAME})
