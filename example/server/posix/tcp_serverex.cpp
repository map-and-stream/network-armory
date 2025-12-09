#include <iostream>
#include "server/posix/tcp_server.h"

std::string callback_function(int client_id, const std::string& req) {
    std::cout << "callback Received from clientID[" << client_id << "] msg: " << req << std::endl;
    return "Echo: " + req + "\n";
}

int main() {
    TcpServer server(8083, callback_function);

    if (!server.start()) {
        std::cerr << "Failed to start TCP server\n";
        return 1;
    }

    std::cout << "TCP Server started on port 8083\n";
    server.run();
}
