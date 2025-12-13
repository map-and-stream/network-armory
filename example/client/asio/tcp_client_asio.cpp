#include <atomic>
#include <iostream>
#include <thread>

#include "client/asio/tcp_client.h"

int main() {
    NetworkConfig cfg;
    cfg.ip = "127.0.0.1";
    cfg.port = 8083;

    TcpClientAsio client(cfg);

    std::atomic<bool> connected{false};

    // Receive loop
    std::function<void()> arm_receive;
    arm_receive = [&]() {
        client.recieve_async([&](const std::vector<uint8_t>& data, Error err) {
            if (err.code() != ErrorCode::NO_ERROR) {
                std::cout << "[CLIENT] Lost connection, reconnecting...\n";
                connected = false;
                return;  // reconnect loop inside client will handle it
            }

            std::string msg(data.begin(), data.end());
            std::cout << "[CLIENT] Received: " << msg << std::endl;

            arm_receive();  // continue receiving
        });
    };

    // Connect asynchronously
    client.connect_async([&](Error err) {
        if (err.code() == ErrorCode::NO_ERROR) {
            std::cout << "[CLIENT] Connected to server\n";
            connected = true;
            arm_receive();
        } else {
            std::cout << "[CLIENT] Initial connect failed, waiting...\n";
            // reconnect loop will start automatically
        }
    });

    // Periodic send loop
    while (true) {
        if (connected) {
            std::vector<uint8_t> msg = {'H', 'e', 'l', 'l', 'o', '\n'};
            Error err = client.send_sync(msg);

            if (err.code() != ErrorCode::NO_ERROR) {
                std::cout << "[CLIENT] Send failed, connection lost\n";
                connected = false;
            } else {
                std::cout << "[CLIENT] Sent message\n";
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    return 0;
}
