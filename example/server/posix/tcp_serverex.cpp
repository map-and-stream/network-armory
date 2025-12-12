#include <iostream>

#include "server/posix/tcp_server.h"
#include "server/server_interface.h"

/*
you can run simple client by run command => nc 127.0.0.1 8083
*/

void receive_callback(int fd, const std::vector<uint8_t>& data) {
    std::cout << "Received from clientID[" << fd
              << "] msg: " << std::string(data.begin(), data.end()) << std::endl;
}

void client_connect_callback(int fd, const std::string& ip) {
    std::cout << "Client connected  fd[" << fd << "] ip[" << ip << "]" << std::endl;
}

void client_disconnect_callback(int fd, const std::string& ip) {
    std::cout << "Client disconnected  fd[" << fd << "] ip[" << ip << "]" << std::endl;
}

int main() {
    ServerConfig cfg;
    cfg.port = 8083;
    TcpServer server(cfg, receive_callback, client_connect_callback, client_disconnect_callback);

    Error err = server.listen();
    if (err.code() != ErrorCode::NO_ERROR) {
        std::cerr << err.to_string() << std::endl;
        return 1;
    }

    std::cout << "TCP Server started on port 8083\n";
    server.gracefull_shutdown();
    return 0;
}
