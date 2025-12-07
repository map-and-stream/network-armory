# #---------------- client
# add_executable(tcp_client_example client/main.cpp)
# target_link_libraries(tcp_client_example PRIVATE ${LIB_ALIAS})

# #---------------- server
# #------ posix
# add_executable(tcp_posix_server_example server/posix/main.cpp)
# target_link_libraries(tcp_posix_server_example PRIVATE ${LIB_ALIAS})
# #------ boost
# target_include_directories(network_armory PUBLIC
#     ${CMAKE_SOURCE_DIR}/src/client/asio
# )

#---------------- client
add_executable(tcp_client_example
    client/tcp_client_example.cpp
)
target_link_libraries(tcp_client_example PRIVATE ${LIB_ALIAS})

#---------------- server (POSIX)
add_executable(tcp_posix_server_example
    server/posix/tcp_posix_server_example.cpp
    ${CMAKE_SOURCE_DIR}/src/server/posix/tcp_server.cpp
)
target_link_libraries(tcp_posix_server_example PRIVATE ${LIB_ALIAS})

#---------------- UDP server
add_executable(udp_posix_server_example
    server/posix/udp_posix_server_example.cpp
    ${CMAKE_SOURCE_DIR}/src/server/posix/udp_server.cpp
)
target_link_libraries(udp_posix_server_example PRIVATE ${LIB_ALIAS})

#---------------- UDP client
add_executable(udp_client_example
    client/udp_client_example.cpp
)
target_link_libraries(udp_client_example PRIVATE ${LIB_ALIAS})

#------ boost include path
target_include_directories(network_armory PUBLIC
    ${CMAKE_SOURCE_DIR}/src/client/asio
)
