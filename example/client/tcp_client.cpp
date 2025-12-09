#include <asio.hpp>
#include <iostream>
#include <memory>
#include <vector>
#include "client/asio/tcp.h"

int main() {
    asio::io_context io_context;

    NetworkConfig cfg;
    cfg.ip = "127.0.0.1";
    cfg.port = 8083;

    std::shared_ptr<TCPConnection> tcp = std::make_shared<TCPConnection>(io_context, cfg);

    Error err = tcp->connect();
    if (err.code() != ErrorCode::NO_ERROR) {
        std::cerr << "Failed to connect: " << err.to_string() << std::endl;
        return 1;
    }

    std::cout << "[TCP Client] Connected\n";

    std::vector<uint8_t> message = {'H', 'e', 'l', 'l', 'o', '\n'};
    tcp->send_sync(message);

    std::vector<uint8_t> recv_data;
    tcp->recieve_sync(recv_data);

    std::cout << "[TCP Client] Received: ";
    for (auto c : recv_data) std::cout << c;
    std::cout << std::endl;

    return 0;
}
