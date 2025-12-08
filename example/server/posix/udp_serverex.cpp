#include "server/posix/udp_server.h"
#include <iostream>

int main() {
    UdpServer server(8084);

    if (!server.start()) {
        std::cerr << "Failed to start UDP server\n";
        return 1;
    }

    std::cout << "UDP Server started on port 8084\n";
    server.run();
}
