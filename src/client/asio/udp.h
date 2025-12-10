#pragma once

#include <asio.hpp>
#include <functional>
#include <vector>

#include "client/error.h"
#include "client/network.h"

class UDPupdate {
  public:
    UDPupdate(asio::io_context& ctx, const NetworkConfig& cfg);

    Error Open();
    Error send_async(const std::vector<uint8_t>& data, std::function<void(Error)> callback);
    Error recieve_async(std::function<void(const std::vector<uint8_t>&, Error)> callback);

  private:
    asio::io_context& io_context_;
    asio::ip::udp::socket socket_;
    asio::ip::udp::endpoint server_endpoint_;
    NetworkConfig cfg_;
};
