#include "client/asio/tcp.h"

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>
#include <vector>

#include "client/error.h"
#include "client/network.h"
#include "server/posix/tcp_server.h"

extern std::string last_server_received;

std::string server_test_callback(int, const std::string& req) {
    last_server_received = req;
    return "reply:" + req;
}

// SYNC TEST
void run_tcp_test() {
    const int kTestPort = 50999;
    last_server_received.clear();

    std::atomic<bool> server_ready{false};
    std::unique_ptr<TcpServer> server_ptr;

    std::thread server_thread([&]() {
        server_ptr = std::make_unique<TcpServer>(kTestPort, server_test_callback);

        if (!server_ptr->start()) {
            server_ready = true;
            return;
        }

        server_ready = true;
        server_ptr->run();
    });

    while (!server_ready) std::this_thread::sleep_for(std::chrono::milliseconds(5));

    asio::io_context io_ctx;
    NetworkConfig cfg{"127.0.0.1", kTestPort};
    TCPConnection client(io_ctx, cfg);

    Error e = client.connect();
    ASSERT_EQ(e.code(), ErrorCode::NO_ERROR);

    std::vector<uint8_t> to_send{'h', 'e', 'l', 'l', 'o'};
    e = client.send_sync(to_send);
    ASSERT_EQ(e.code(), ErrorCode::NO_ERROR);

    std::vector<uint8_t> recv;
    Error recv_err = client.recieve_sync(recv);
    ASSERT_EQ(recv_err.code(), ErrorCode::NO_ERROR);

    ASSERT_EQ(last_server_received, "hello");
    ASSERT_EQ(std::string(recv.begin(), recv.end()), "reply:hello");

    server_ptr->stop();
    server_thread.join();
}

// ASYNC TEST
void run_tcp_async_test() {
    const int kTestPort = 51000;
    last_server_received.clear();

    std::atomic<bool> server_ready{false};
    std::unique_ptr<TcpServer> server_ptr;

    std::thread server_thread([&]() {
        server_ptr = std::make_unique<TcpServer>(kTestPort, server_test_callback);
        if (!server_ptr->start()) {
            server_ready = true;
            return;
        }
        server_ready = true;
        server_ptr->run();
    });

    while (!server_ready) std::this_thread::sleep_for(std::chrono::milliseconds(5));

    asio::io_context io_ctx;
    auto guard = asio::make_work_guard(io_ctx);

    NetworkConfig cfg{"127.0.0.1", kTestPort};
    TCPConnection client(io_ctx, cfg);

    std::atomic<bool> connect_done{false};
    std::atomic<bool> send_done{false};
    std::atomic<bool> recv_done{false};

    Error connect_err, send_err, recv_err;
    std::vector<uint8_t> recv_data;

    std::thread io_thread([&]() { io_ctx.run(); });

    client.connect_async([&](Error err) {
        connect_err = err;
        connect_done = true;

        if (err.code() == ErrorCode::NO_ERROR) {
            std::vector<uint8_t> msg{'h', 'e', 'l', 'l', 'o'};
            client.send_async(msg, [&](Error err2) {
                send_err = err2;
                send_done = true;

                client.recieve_async([&](const std::vector<uint8_t>& data, Error err3) {
                    recv_data = data;
                    recv_err = err3;
                    recv_done = true;
                });
            });
        }
    });

    while (!connect_done) std::this_thread::sleep_for(std::chrono::milliseconds(1));
    while (!send_done) std::this_thread::sleep_for(std::chrono::milliseconds(1));
    while (!recv_done) std::this_thread::sleep_for(std::chrono::milliseconds(1));

    guard.reset();
    io_ctx.stop();
    io_thread.join();
    server_ptr->stop();
    server_thread.join();

    ASSERT_EQ(connect_err.code(), ErrorCode::NO_ERROR);
    ASSERT_EQ(send_err.code(), ErrorCode::NO_ERROR);
    ASSERT_EQ(recv_err.code(), ErrorCode::NO_ERROR);

    ASSERT_EQ(last_server_received, "hello");
    ASSERT_EQ(std::string(recv_data.begin(), recv_data.end()), "reply:hello");
}
