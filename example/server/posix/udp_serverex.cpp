#include <iostream>

#include "server/posix/udp_server.h"

std::string callback_function(int client_id, const std::string& req) {
    std::cout << "callback Received from clientID[" << client_id << "] msg: " << req << std::endl;
    return "Echo: " + req + "\n";
}

int main() {
    UdpServer server(8084, callback_function);

    Error err = server.start();
    if (err.code() != ErrorCode::NO_ERROR) {
        std::cerr << err.to_string() << std::endl;
        return 1;
    }

    std::cout << "UDP Server started on port 8084\n";
    server.run();
}
