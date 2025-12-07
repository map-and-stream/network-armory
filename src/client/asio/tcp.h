// #ifndef BOOST_TCP_CONNECTION_H
// #define BOOST_TCP_CONNECTION_H

// #include <asio.hpp>
// #include <functional>
// #include <memory>
// #include <string>
// #include <vector>

// #include "client/network.h"

// class TCPConnection : public INetwork, public std::enable_shared_from_this<TCPConnection> {
//   public:
//     using Ptr = std::shared_ptr<TCPConnection>;

//     static Ptr create(asio::io_context& io_context, const std::string& host, unsigned short port);

//     TCPConnection(asio::io_context& io_context, asio::ip::tcp::socket socket);
//     ~TCPConnection() override;

//     // --- INetwork interface overrides ---
//     Error connect() override;
//     Error send_sync(const std::vector<uint8_t>& data) override;
//     Error send_async(const std::vector<uint8_t>& data) override;
//     Error recieve_sync(std::vector<uint8_t>& recieve_data) override;
//     Error recieve_async(ReceiveCallback cb) override;

//   private:
//     void connect_impl(const std::string& host, unsigned short port);
//     void handleRead(const asio::error_code& error, std::size_t bytes_transferred);

//     asio::io_context& io_context_;
//     asio::ip::tcp::socket socket_;
//     std::vector<uint8_t> read_buffer_;
//     ReceiveCallback read_callback_;
// };

// #endif
#ifndef BOOST_TCP_CONNECTION_H
#define BOOST_TCP_CONNECTION_H

#include <asio.hpp>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "client/network.h"   // ✅ Your real NetworkConfig
#include "error.h"            // ✅ Your real Error type

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

#endif // BOOST_TCP_CONNECTION_H
