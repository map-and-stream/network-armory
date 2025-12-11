#pragma once

#include <asio.hpp>
#include <functional>
#include <vector>

#include "client/client_interface.h"
#include "client/error.h"

class UDP : public ClientInterface {
  public:
    UDP(asio::io_context& ctx, const NetworkConfig& cfg);

    Error connect_async(std::function<void(Error)> callback) override;
    Error disconnect() override;
    Error send_async(const std::vector<uint8_t>& data,
                     std::function<void(Error)> callback) override;
    Error recieve_async(std::function<void(const std::vector<uint8_t>&, Error)> callback) override;

  private:
    asio::io_context& io_context_;
    asio::ip::udp::socket socket_;
    asio::ip::udp::endpoint server_endpoint_;
};
