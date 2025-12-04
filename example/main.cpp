#include <iostream>
#include <vector>
#include <memory>
#include <thread>
#include <chrono>
#include "client/asio/tcp.h"

int main() {
    asio::io_context io_context;

    // Prepare NetworkConfig for TCP connection
    NetworkConfig cfg;
    cfg.ip = "127.0.0.1";
    cfg.port = 12345;
    cfg.mode = WorkingMode::ASYNC;
    cfg.keep_alive = false;

    // Create a TCPConnection via the factory function (returns INetwork*)
    std::shared_ptr<TCPConnection> tcp =
        std::make_shared<TCPConnection>(io_context, asio::ip::tcp::socket(io_context));
    // Optionally, you may use a proper custom factory for dynamic NetworkConfig

    // Connect to the server
    Error err = tcp->connect();
    if (err.code() != ErrorCode::NO_ERROR) {
        std::cerr << "Failed to connect: " << err.to_string() << std::endl;
        return 1;
    }

    std::cout << "[TCP Client] Connected to "
              << cfg.ip << ":" << cfg.port << std::endl;

    // Prepare data to send
    std::vector<uint8_t> message = {'H', 'e', 'l', 'l', 'o', ' ', 'T', 'C', 'P'};

    // Send synchronously
    Error send_err = tcp->send_sync(message);
    if (send_err.code() != ErrorCode::NO_ERROR) {
        std::cerr << "Send failed: " << send_err.to_string() << std::endl;
    } else {
        std::cout << "[TCP Client] Sent: ";
        for (auto c : message)
            std::cout << c;
        std::cout << std::endl;
    }

    // Receive synchronously
    std::vector<uint8_t> recv_data;
    Error recv_err = tcp->recieve_sync(recv_data);
    if (recv_err.code() != ErrorCode::NO_ERROR) {
        std::cerr << "Receive failed: " << recv_err.to_string() << std::endl;
    } else {
        std::cout << "[TCP Client] Received: ";
        for (auto c : recv_data)
            std::cout << c;
        std::cout << std::endl;
    }

    // Demonstrate async receive (and re-use send_async)
    tcp->recieve_async([](const std::vector<uint8_t>& data){
        std::cout << "[TCP Client Async] Received: ";
        for (auto c : data)
            std::cout << c;
        std::cout << std::endl;
    });

    tcp->send_async({'A','S','Y','N','C','!','\n'});

    // Run event loop in background thread for demonstration
    std::thread io_thread([&io_context]() { io_context.run(); });

    std::this_thread::sleep_for(std::chrono::seconds(2)); // Allow time for I/O

    io_context.stop();
    io_thread.join();

    return 0;
}
