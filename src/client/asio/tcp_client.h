#pragma once

#include <asio.hpp>
#include <functional>
#include <vector>

#include "client/client_interface.h"
#include "error.h"

class TcpClientAsio : public ClientInterface {
  public:
    TcpClientAsio(asio::io_context& ctx, const NetworkConfig& cfg);

    Error connect() override;
    Error connect_async(std::function<void(Error)> callback) override;

    Error send_sync(const std::vector<uint8_t>& data) override;
    Error send_async(const std::vector<uint8_t>& data,
                     std::function<void(Error)> callback) override;

    Error recieve_sync(std::vector<uint8_t>& out) override;
    Error recieve_async(std::function<void(const std::vector<uint8_t>&, Error)> callback) override;

    Error disconnect() override;

  private:
    asio::io_context& io_context_;
    asio::ip::tcp::socket socket_;
};
