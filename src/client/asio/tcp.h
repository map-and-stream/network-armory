#ifndef BOOST_TCP_CONNECTION_H
#define BOOST_TCP_CONNECTION_H

#include <asio.hpp>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "client/network.h"  // ✅ Your real NetworkConfig
#include "error.h"           // ✅ Your real Error type

class TCPConnection {
  public:
    TCPConnection(asio::io_context& ctx, const NetworkConfig& cfg);

    Error connect();
    Error send_sync(const std::vector<uint8_t>& data);
    Error recieve_sync(std::vector<uint8_t>& out);

  private:
    asio::io_context& io_context_;
    asio::ip::tcp::socket socket_;
    NetworkConfig cfg_;
};

#endif  // BOOST_TCP_CONNECTION_H
