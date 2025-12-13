#pragma once

#include <asio.hpp>
#include <atomic>
#include <functional>
#include <thread>
#include <vector>

#include "client/client_interface.h"
#include "error.h"

class TcpClientAsio : public ClientInterface {
  public:
    TcpClientAsio(const NetworkConfig& cfg);
    ~TcpClientAsio();

    Error connect() override;
    Error connect_async(std::function<void(Error)> callback) override;

    Error send_sync(const std::vector<uint8_t>& data) override;
    Error send_async(const std::vector<uint8_t>& data,
                     std::function<void(Error)> callback) override;

    Error recieve_sync(std::vector<uint8_t>& out) override;
    Error recieve_async(std::function<void(const std::vector<uint8_t>&, Error)> callback) override;

    Error disconnect() override;

  private:
    void run_io();
    void start_reconnect_loop();

  private:
    asio::io_context io_;
    asio::executor_work_guard<asio::io_context::executor_type> work_guard_;
    std::thread io_thread_;

    asio::ip::tcp::socket socket_;
    asio::strand<asio::io_context::executor_type> strand_;

    std::atomic<bool> reconnecting_{false};
};
