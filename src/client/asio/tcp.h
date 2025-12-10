#ifndef BOOST_TCP_CONNECTION_H
#define BOOST_TCP_CONNECTION_H

#include <asio.hpp>
#include <vector>

#include "client/network.h"  // Your real NetworkConfig

class TCPConnection {
  public:
    TCPConnection(asio::io_context& ctx, const NetworkConfig& cfg);

    Error connect();
    Error send_sync(const std::vector<uint8_t>& data);
    Error send_async(const std::vector<uint8_t>& data, std::function<void(Error)> callback);
    Error recieve_sync(std::vector<uint8_t>& out);
    Error recieve_async(std::function<void(const std::vector<uint8_t>&, Error)> callback);
    Error connect_async(std::function<void(Error)> callback);

  private:
    asio::io_context& io_context_;
    asio::ip::tcp::socket socket_;
    NetworkConfig cfg_;
};

#endif  // BOOST_TCP_CONNECTION_H
