#---------------- TCP client
add_executable(tcp_client
    client/tcp_client.cpp
)
target_link_libraries(tcp_client PRIVATE ${LIB_ALIAS})

#---------------- TCP server
add_executable(tcp_serverex
    server/posix/tcp_serverex.cpp
    ${CMAKE_SOURCE_DIR}/src/server/posix/tcp_server.cpp
)
target_link_libraries(tcp_serverex PRIVATE ${LIB_ALIAS})

#---------------- UDP server
add_executable(udp_serverex
    server/posix/udp_serverex.cpp
    ${CMAKE_SOURCE_DIR}/src/server/posix/udp_server.cpp
)
target_link_libraries(udp_serverex PRIVATE ${LIB_ALIAS})

#---------------- UDP client
add_executable(udp_client
    client/udp_client.cpp
)
target_link_libraries(udp_client PRIVATE ${LIB_ALIAS})

#------ boost include path
target_include_directories(network_armory PUBLIC
    ${CMAKE_SOURCE_DIR}/src/client/asio
)
