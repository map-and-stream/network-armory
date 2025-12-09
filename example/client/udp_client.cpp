#include <asio.hpp>
#include <iostream>
#include <memory>
#include <vector>

#include "client/asio/udp.h"

int main() {
    asio::io_context io;

    NetworkConfig cfg;
    cfg.ip = "127.0.0.1";
    cfg.port = 8084;

    auto udp = std::make_shared<UDPConnection>(io, cfg);

    Error err = udp->connect();
    if (err.code() != ErrorCode::NO_ERROR) {
        std::cerr << "Failed to connect: " << err.to_string() << std::endl;
        return 1;
    }

    std::cout << "[UDP Client] Ready\n";

    std::vector<uint8_t> msg = {'H', 'e', 'l', 'l', 'o', ' ', 'U', 'D', 'P'};
    udp->send_sync(msg);

    std::vector<uint8_t> recv_data;
    udp->recieve_sync(recv_data);

    std::cout << "[UDP Client] Received: ";
    for (auto c : recv_data) std::cout << c;
    std::cout << std::endl;

    return 0;
}
