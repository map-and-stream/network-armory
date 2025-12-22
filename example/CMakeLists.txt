# ---------- TCP Client (ASIO) ----------
add_executable(tcp_client_asio
    client/asio/tcp_client_asio.cpp
)
target_link_libraries(tcp_client_asio PRIVATE ${LIB_ALIAS})

# ---------- TCP Server (POSIX) ----------
add_executable(tcp_serverex
    server/posix/tcp_serverex.cpp
    ${CMAKE_SOURCE_DIR}/src/server/posix/tcp_server.cpp
)
target_link_libraries(tcp_serverex PRIVATE ${LIB_ALIAS})

# ---------- UDP Server (POSIX) ----------
add_executable(udp_serverex
    server/posix/udp_serverex.cpp
    ${CMAKE_SOURCE_DIR}/src/server/posix/udp_server.cpp
)
target_link_libraries(udp_serverex PRIVATE ${LIB_ALIAS})

# ---------- UDP Client (ASIO) ----------
add_executable(udp_client_asio
    client/asio/udp_client_asio.cpp
)
target_link_libraries(udp_client_asio PRIVATE ${LIB_ALIAS})

# ---------- TCP Client (POSIX) ----------
add_executable(tcp_client_posix
    client/posix/tcp_client_posix.cpp
)
target_link_libraries(tcp_client_posix PRIVATE ${LIB_ALIAS})


# ---------- HTTP SERVER (ASIO) ----------
add_executable(http_server_boost
    server/http/main.cpp
)
find_package(Boost REQUIRED)

target_link_libraries(http_server_boost PRIVATE ${LIB_ALIAS}     Boost::headers )

# ============================================
# Include Path (CRITICAL FIX)
# ============================================
# This exposes src/ so includes like:
#   #include "client/client_interface.h"
#   #include "factory.h"
#   #include "client/asio/tcp_client.h"
# all resolve correctly.
target_include_directories(network_armory PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${Boost_INCLUDE_DIRS}
)
