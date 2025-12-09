#ifndef BOOST_UDP_CONNECTION_H
#define BOOST_UDP_CONNECTION_H

#include <asio.hpp>
#include <vector>

#include "client/error.h"    // Error, ErrorCode
#include "client/network.h"  // NetworkConfig

class UDPConnection {
  public:
    UDPConnection(asio::io_context& ctx, const NetworkConfig& cfg);

    Error connect();  // For UDP: open + prepare endpoint
    Error send_sync(const std::vector<uint8_t>& data);
    Error recieve_sync(std::vector<uint8_t>& out);

  private:
    asio::io_context& io_context_;
    asio::ip::udp::socket socket_;
    asio::ip::udp::endpoint server_endpoint_;
    NetworkConfig cfg_;
};

#endif
