set(LIBRARY_NAME network_armory)

file(GLOB_RECURSE CONFIGURE_DEPENDS NETWORK_HEADERS 
    "${CMAKE_CURRENT_SOURCE_DIR}/client/*.h" 
    "${CMAKE_CURRENT_SOURCE_DIR}/client/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/server/posix/*.h" 
    "${CMAKE_CURRENT_SOURCE_DIR}/server/posix/*.hpp"
)
set(NETWORK_SOURCES
    client/asio/tcp.cpp 
    server/posix/tcp_server.cpp 
    client/asio/udp.cpp
    server/posix/udp_server.cpp
)

# Create a header-only "asio" target
add_library(asio INTERFACE)

# Add Asio include directory
target_include_directories(asio INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/third-party/asio/asio/include
)

# Define standalone mode
target_compile_definitions(asio INTERFACE ASIO_STANDALONE)

add_library(${LIBRARY_NAME} STATIC 
${NETWORK_SOURCES}
${NETWORK_HEADERS}
)

# Link Boost::asio (header-only), Boost::system, and threads
target_link_libraries(${LIBRARY_NAME} PUBLIC
    asio
)

# Optionally (if using Boost headers, e.g., for boost/asio.hpp)
target_include_directories(${LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Keep headers associated with this target for IDEs, but do not propagate
target_sources(${LIBRARY_NAME} PRIVATE ${NETWORK_HEADERS})

# Convenient namespaced alias for downstream targets
add_library(${LIB_ALIAS} ALIAS ${LIBRARY_NAME})
