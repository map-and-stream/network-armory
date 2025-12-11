#include "client/asio/udp_client.h"

#include <asio.hpp>
#include <atomic>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

int main() {
    asio::io_context io;

    NetworkConfig cfg;
    cfg.ip = "127.0.0.1";
    cfg.port = 8084;

    auto udp = std::make_shared<UdpClient>(io, cfg);

    Error err = udp->connect();
    if (err.code() != ErrorCode::NO_ERROR) {
        std::cerr << "Failed to Open Socket: " << err.to_string() << std::endl;
        return 1;
    }

    std::cout << "[UDP Client] Ready\n";

    std::atomic<bool> recv_done{false};
    std::vector<uint8_t> recv_data;

    udp->recieve_async([&](const std::vector<uint8_t>& data, Error err) {
        std::cerr << "[CLIENT] received: " << std::string(data.begin(), data.end())
                  << " err=" << (int)err.code() << std::endl;

        recv_data = data;
        recv_done = true;
    });

    std::vector<uint8_t> msg = {'H', 'e', 'l', 'l', 'o', ' ', 'U', 'D', 'P'};
    udp->send_async(msg, [&](Error err) {
        std::cerr << "[CLIENT] send callback err=" << (int)err.code() << std::endl;
    });

    std::thread io_thread([&]() { io.run(); });

    while (!recv_done) std::this_thread::sleep_for(std::chrono::milliseconds(1));

    io.stop();
    io_thread.join();

    std::cout << "[UDP Client] Final Received: ";
    for (auto c : recv_data) std::cout << c;
    std::cout << std::endl;

    return 0;
}
