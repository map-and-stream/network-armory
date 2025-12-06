#include "server/posix/tcp_server.h"
#include <iostream>

/*
    connect to server by nc -> nc 127.0.0.1 8083
*/

std::string callback_function(int client_id, const std::string& req){
    std::cout << "callback Received from clientID["<<client_id<<"] msg: " << req << std::endl;
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