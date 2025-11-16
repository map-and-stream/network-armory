#include <iostream>
#include <vector>
#include <memory>
#include <thread>
#include <chrono>
#include "boost/boost_network.h"
#include "network.h"

int main() {
    boost::asio::io_context io;

    BoostNetwork boostNet(io);

    // Example TCP Client
    std::shared_ptr<INetwork> tcp_client =
        boostNet.createTCPClient("127.0.0.1", 12345);

    // Example UDP
    std::shared_ptr<INetwork> udp =
        boostNet.createUDP("127.0.0.1", 43210);

    // Example Serial (assume "COM1" or "/dev/ttyS1", 9600 baud)
    std::shared_ptr<INetwork> serial =
        boostNet.createSerial("COM1", 9600);

    // Try sending and receiving if successfully created
    if (tcp_client) {
        tcp_client->asyncRead([](const std::vector<uint8_t>& data) {
            std::cout << "[TCP Client] Received: ";
            for (auto c : data)
                std::cout << std::hex << int(c) << " ";
            std::cout << std::dec << std::endl;
        });
        std::vector<uint8_t> tcp_msg = {'H', 'e', 'l', 'l', 'o', 'T', 'C', 'P'};
        tcp_client->send(tcp_msg);
    }

    if (udp) {
        udp->asyncRead([](const std::vector<uint8_t>& data) {
            std::cout << "[UDP] Received: ";
            for (auto c : data)
                std::cout << std::hex << int(c) << " ";
            std::cout << std::dec << std::endl;
        });
        std::vector<uint8_t> udp_msg = {'H', 'e', 'l', 'l', 'o', 'U', 'D', 'P'};
        udp->send(udp_msg);
    }

    if (serial) {
        serial->asyncRead([](const std::vector<uint8_t>& data) {
            std::cout << "[Serial] Received: ";
            for (auto c : data)
                std::cout << std::hex << int(c) << " ";
            std::cout << std::dec << std::endl;
        });
        std::vector<uint8_t> serial_msg = {'H', 'e', 'l', 'l', 'o', 'S', 'E', 'R'};
        serial->send(serial_msg);
    }

    // Run event loop in a background thread for demo
    std::thread io_thread([&io]() { io.run(); });

    // Let IO context run for a short period to demonstrate async
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Stop io_context and join thread
    io.stop();
    io_thread.join();

    return 0;
}
