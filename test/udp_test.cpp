#include "client/asio/udp.h"

#include <atomic>
#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "client/error.h"
#include "client/network.h"
#include "server/posix/udp_server.h"

extern std::string last_udp_server_received;

std::string udp_server_test_callback(int, const std::string& req) {
    std::cerr << "SERVER RECEIVED: " << req << std::endl;
    last_udp_server_received = req;
    return "reply:" + req;
}

bool run_udp_project_test() {
    const int kUdpTestPort = 55000;
    last_udp_server_received.clear();

    std::atomic<bool> server_ready{false};
    std::unique_ptr<UdpServer> server_ptr;

    // Start UDP server
    std::thread server_thread([&]() {
        server_ptr = std::make_unique<UdpServer>(kUdpTestPort, udp_server_test_callback);
        if (!server_ptr->start()) {
            server_ready = true;
            return;
        }
        server_ready = true;
        server_ptr->run();
    });

    while (!server_ready) std::this_thread::sleep_for(std::chrono::milliseconds(5));

    // Create client
    asio::io_context io_ctx;
    NetworkConfig cfg{"127.0.0.1", kUdpTestPort};
    UDPupdate client(io_ctx, cfg);

    if (client.Open().code() != ErrorCode::NO_ERROR)
        return false;

    std::atomic<bool> recv_done{false};
    std::atomic<bool> send_done{false};
    std::vector<uint8_t> recv_data;
    Error recv_err;
    Error send_err;

    // Run io_context in background
    std::thread io_thread([&]() { io_ctx.run(); });

    // Arm async receive BEFORE sending
    client.recieve_async([&](const std::vector<uint8_t>& data, Error err) {
        std::cerr << "[CLIENT] received: " << std::string(data.begin(), data.end())
                  << " err=" << (int)err.code() << std::endl;

        recv_data = data;
        recv_err = err;
        recv_done = true;
    });

    // end async
    std::vector<uint8_t> msg{'h', 'e', 'l', 'l', 'o'};
    client.send_async(msg, [&](Error err) {
        std::cerr << "[CLIENT] send callback err=" << (int)err.code() << std::endl;
        send_err = err;
        send_done = true;
    });

    // Wait for both operations
    while (!send_done) std::this_thread::sleep_for(std::chrono::milliseconds(1));
    while (!recv_done) std::this_thread::sleep_for(std::chrono::milliseconds(1));

    // Cleanup
    io_ctx.stop();
    io_thread.join();
    server_ptr->stop();
    server_thread.join();

    // Validate
    return send_err.code() == ErrorCode::NO_ERROR && recv_err.code() == ErrorCode::NO_ERROR &&
           last_udp_server_received == "hello" &&
           std::string(recv_data.begin(), recv_data.end()) == "reply:hello";
}
