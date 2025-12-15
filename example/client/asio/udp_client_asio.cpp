#include <asio.hpp>
#include <atomic>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include "client/client_interface.h"
#include "client/asio/udp_client.h"

int main() {
    auto io = std::make_shared<asio::io_context>();

    NetworkConfig cfg;
    cfg.ip   = "127.0.0.1";
    cfg.port = 8084;
    cfg.connection_type = ClientType::UDP;

    auto udp = std::make_shared<UdpClient>(cfg, io);
    
    std::atomic<bool> recv_done{false};
    std::vector<uint8_t> recv_data;

    Error err = udp->connect_async([&](Error err) {
        if (err.code() != ErrorCode::NO_ERROR) {
            std::cerr << "Failed to open socket: " << err.to_string() << std::endl;
        } else {
            std::cout << "[UDP Client] Ready\n";

            udp->recieve_async([&](const std::vector<uint8_t>& data, Error err2) {
                std::cerr << "[CLIENT] received: " << std::string(data.begin(), data.end())
                          << " err=" << static_cast<int>(err2.code()) << std::endl;
                recv_data = data;
                recv_done = true;
            });

            std::vector<uint8_t> msg = {'H', 'e', 'l', 'l', 'o', ' ', 'U', 'D', 'P'};
            udp->send_async(msg, [&](Error err3) {
                std::cerr << "[CLIENT] send callback err=" << static_cast<int>(err3.code()) << std::endl;
            });
        }
    });

    if (err.code() != ErrorCode::NO_ERROR) {
        std::cerr << "Failed to start async connect: " << err.to_string() << std::endl;
        return 1;
    }

    std::thread io_thread([&]() { io->run(); });

    while (!recv_done)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

    io->stop();
    io_thread.join();

    std::cout << "[UDP Client] Final Received: ";
    for (auto c : recv_data)
        std::cout << c;
    std::cout << std::endl;

    return 0;
}
