#---------------- client
add_executable(tcp_client_example client/main.cpp)
target_link_libraries(tcp_client_example PRIVATE ${LIB_ALIAS})

#---------------- server
#------ posix
add_executable(tcp_posix_server_example server/posix/main.cpp)
target_link_libraries(tcp_posix_server_example PRIVATE ${LIB_ALIAS})
#------ boost
