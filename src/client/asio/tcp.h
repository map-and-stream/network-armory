#ifndef BOOST_TCP_CONNECTION_H
#define BOOST_TCP_CONNECTION_H

#include <asio.hpp>
#include <functional>
#include <vector>

#include "client/error.h"
#include "client/network.h"

class TCPConnection {
  public:
    TCPConnection(asio::io_context& ctx, const NetworkConfig& cfg);

    Error connect();
    Error connect_async(std::function<void(Error)> callback);

    Error send_sync(const std::vector<uint8_t>& data);
    Error send_async(const std::vector<uint8_t>& data, std::function<void(Error)> callback);

    Error recieve_sync(std::vector<uint8_t>& out);
    Error recieve_async(std::function<void(const std::vector<uint8_t>&, Error)> callback);

    void close();

  private:
    asio::io_context& io_context_;
    asio::ip::tcp::socket socket_;
    NetworkConfig cfg_;
};

#endif  // BOOST_TCP_CONNECTION_H
