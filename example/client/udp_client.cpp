#include <asio.hpp>
#include <iostream>
#include <vector>

int main() {
    asio::io_context io;

    asio::ip::udp::endpoint server_ep(asio::ip::make_address("127.0.0.1"), 8084);

    asio::ip::udp::socket sock(io);
    sock.open(asio::ip::udp::v4());

    std::vector<uint8_t> msg = {'H', 'e', 'l', 'l', 'o', ' ', 'U', 'D', 'P'};

    sock.send_to(asio::buffer(msg), server_ep);
    std::cout << "Sent UDP message\n";

    uint8_t buf[1024];
    asio::ip::udp::endpoint from;
    size_t n = sock.receive_from(asio::buffer(buf), from);

    std::cout << "Received: ";
    for (size_t i = 0; i < n; i++) std::cout << buf[i];
    std::cout << std::endl;

    return 0;
}
