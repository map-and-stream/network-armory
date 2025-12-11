#include "client/asio/tcp_client.h"

#include <asio.hpp>
#include <atomic>
#include <iostream>
#include <memory>
#include <vector>

int main() {
    asio::io_context io;

    NetworkConfig cfg;
    cfg.ip = "127.0.0.1";
    cfg.port = 8083;

    auto tcp = std::make_shared<TcpClientAsio>(io, cfg);
    std::atomic<bool> connected{false};

    // Receive loop
    std::function<void()> arm_receive;
    arm_receive = [&]() {
        tcp->recieve_async([&](const std::vector<uint8_t>& data, Error err) {
            if (err.code() != ErrorCode::NO_ERROR) {
                std::cout << "[CLIENT] Server disconnected, waiting...\n";
                connected = false;
                return;
            }

            std::string msg(data.begin(), data.end());
            std::cout << "[CLIENT] Received: " << msg << std::endl;

            arm_receive();
        });
    };

    // Active loop with timer
    asio::steady_timer timer(io);
    std::function<void()> active_loop;

    active_loop = [&]() {
        timer.expires_after(std::chrono::seconds(5));
        timer.async_wait([&](const asio::error_code&) {
            if (!connected) {
                tcp->disconnect();
                Error err = tcp->connect();
                if (err.code() == ErrorCode::NO_ERROR) {
                    std::cout << "[CLIENT] Connected to server\n";
                    connected = true;
                    arm_receive();
                } else {
                    std::cout << "[CLIENT] Server OFF, waiting...\n";
                }
            }

            if (connected) {
                std::vector<uint8_t> msg = {'H', 'e', 'l', 'l', 'o', '\n'};
                Error err = tcp->send_sync(msg);
                if (err.code() != ErrorCode::NO_ERROR) {
                    std::cout << "[CLIENT] Send failed, server OFF\n";
                    connected = false;
                } else {
                    std::cout << "[CLIENT] Sent message\n";
                }
            }

            active_loop();
        });
    };

    active_loop();  // start loop
    io.run();
    return 0;
}
