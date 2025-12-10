#include <asio.hpp>
#include <atomic>
#include <iostream>
#include <memory>
#include <vector>

#include "client/asio/tcp.h"

int main() {
    asio::io_context io_context;

    NetworkConfig cfg;
    cfg.ip = "127.0.0.1";
    cfg.port = 8083;

    auto tcp = std::make_shared<TCPConnection>(io_context, cfg);

    Error err = tcp->connect();
    if (err.code() != ErrorCode::NO_ERROR) {
        std::cerr << "Failed to connect: " << err.to_string() << std::endl;
        return 1;
    }

    std::cout << "[TCP Client] Connected\n";

    std::atomic<bool> recv_done{false};
    std::vector<uint8_t> recv_data;

    // Arm async receive BEFORE sending
    tcp->recieve_async([&](const std::vector<uint8_t>& data, Error err) {
        std::cerr << "[CLIENT] received: " << std::string(data.begin(), data.end())
                  << " err=" << (int)err.code() << std::endl;

        recv_data = data;
        recv_done = true;
    });

    // Send async
    std::vector<uint8_t> message = {'H', 'e', 'l', 'l', 'o', '\n'};
    tcp->send_async(message, [&](Error err) {
        std::cerr << "[CLIENT] send callback err=" << (int)err.code() << std::endl;
    });

    // Run event loop
    std::thread io_thread([&]() { io_context.run(); });

    // Wait for receive
    while (!recv_done) std::this_thread::sleep_for(std::chrono::milliseconds(1));

    io_context.stop();
    io_thread.join();

    std::cout << "[TCP Client] Final Received: ";
    for (auto c : recv_data) std::cout << c;
    std::cout << std::endl;

    return 0;
}
