#include "server/posix/tcp_server.h"
#include <iostream>

int main() {
    int port = 8083;
    TcpServer server(
        port,
        [](const std::string& req) {
            std::cout << "Received: " << req << std::endl;
            return "Echo: " + req;
        }
    );

    if (!server.start()) {
        std::cerr << "Failed to start server\n";
        return 1;
    }

    std::cout << "Server started on port " << port <<"\n";
    server.run();
}