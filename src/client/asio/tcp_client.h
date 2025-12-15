#pragma once

#include <asio.hpp>
#include <atomic>
#include <memory>
#include <vector>

#include "client/client_interface.h"
#include "error.h"

class TcpClientAsio : public ClientInterface, public std::enable_shared_from_this<TcpClientAsio> {
  public:
    TcpClientAsio(const NetworkConfig& cfg, std::shared_ptr<asio::io_context> io);

    ~TcpClientAsio() override;

    Error connect() override;
    Error connect_async(AsyncCallback callback) override;

    Error send_sync(const std::vector<uint8_t>& data) override;
    Error send_async(const std::vector<uint8_t>& data, AsyncCallback callback) override;

    Error recieve_sync(std::vector<uint8_t>& out) override;
    Error recieve_async(ReceiveCallback callback) override;

    Error disconnect() override;

  private:
    void start_reconnect_loop();

  private:
    std::shared_ptr<asio::io_context> io_;
    asio::ip::tcp::socket socket_;
    asio::strand<asio::io_context::executor_type> strand_;

    std::atomic<bool> reconnecting_{false};
};
