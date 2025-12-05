#include "server/posix/tcp_server.h"
#include <iostream>

std::string callback_function(const std::string& req){
    std::cout << "callback Received: " << req << std::endl;
    return "Echo: " + req;  
}

int main() {
    int port = 8083;
    TcpServer server(
        port,
        callback_function
    );

    if (!server.start()) {
        std::cerr << "Failed to start server\n";
        return 1;
    }

    std::cout << "Server started on port " << port <<"\n";
    server.run();
}