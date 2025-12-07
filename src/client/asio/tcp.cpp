// #include "tcp.h"

// #include <asio.hpp>
// #include <iostream>
// #include <memory>
// #include <string>
// #include <vector>

// #include "client/error.h"

// // Factory method: create and asynchronously connect a new instance.
// TCPConnection::Ptr TCPConnection::create(asio::io_context& io_context, const std::string& host,
//                                          unsigned short port) {
//     auto connection =
//         std::make_shared<TCPConnection>(io_context, asio::ip::tcp::socket(io_context));
//     connection->connect_impl(host, port);
//     return connection;
// }

// TCPConnection::TCPConnection(asio::io_context& io_context, asio::ip::tcp::socket socket)
//     : INetwork(NetworkConfig{}), io_context_(io_context), socket_(std::move(socket)) {}

// TCPConnection::~TCPConnection() = default;

// void TCPConnection::connect_impl(const std::string& host, unsigned short port) {
//     auto self = shared_from_this();
//     asio::ip::tcp::resolver resolver(io_context_);
//     auto results = resolver.resolve(host, std::to_string(port));
//     asio::async_connect(
//         socket_, results,
//         [this, self](const asio::error_code& ec, const asio::ip::tcp::endpoint& /*endpoint*/) {
//             if (ec) {
//                 std::cerr << "[TCPConnection] Error connecting: " << ec.message() << std::endl;
//                 is_connected_ = false;
//             } else {
//                 is_connected_ = true;
//             }
//         });
// }

// Error TCPConnection::connect() {
//     // Simple synchronous connect (example, could be improved)
//     Error err;
//     try {
//         asio::ip::tcp::resolver resolver(io_context_);
//         auto endpoints = resolver.resolve(cfg_.ip, std::to_string(cfg_.port));
//         asio::connect(socket_, endpoints);
//         is_connected_ = true;
//         return *err.set_code(ErrorCode::NO_ERROR);
//     } catch (const std::exception& ex) {
//         is_connected_ = false;
//         return *err.set_code(ErrorCode::CONNECTION_FAILED);
//     }
// }

// Error TCPConnection::send_sync(const std::vector<uint8_t>& data) {
//     Error err;
//     if (!is_connected_)
//         return *err.set_code(ErrorCode::NOT_CONNECTED);
//     asio::error_code ec;
//     asio::write(socket_, asio::buffer(data), ec);
//     if (ec) {
//         std::cerr << "[TCPConnection] send_sync error: " << ec.message() << std::endl;
//         return *err.set_code(ErrorCode::SEND_FAILED);
//     }
//     return err;
// }

// Error TCPConnection::send_async(const std::vector<uint8_t>& data) {
//     Error err;
//     if (!is_connected_)
//         return *err.set_code(ErrorCode::NOT_CONNECTED);
//     auto self = shared_from_this();
//     asio::async_write(socket_, asio::buffer(data),
//                       [this, self](const asio::error_code& ec, std::size_t /*length*/) {
//                           if (ec) {
//                               std::cerr << "[TCPConnection] send_async error: " << ec.message()
//                                         << std::endl;
//                           }
//                       });
//     return err;
// }

// Error TCPConnection::recieve_sync(std::vector<uint8_t>& recieve_data) {
//     Error err;
//     if (!is_connected_)
//         return *err.set_code(ErrorCode::NOT_CONNECTED);
//     asio::error_code ec;
//     recieve_data.resize(4096);
//     std::size_t len = socket_.read_some(asio::buffer(recieve_data), ec);
//     if (ec) {
//         std::cerr << "[TCPConnection] recieve_sync error: " << ec.message() << std::endl;
//         return *err.set_code(ErrorCode::RECEIVE_FAILED);
//     }
//     recieve_data.resize(len);
//     return err;
// }

// Error TCPConnection::recieve_async(ReceiveCallback cb) {
//     Error err;
//     if (!is_connected_)
//         return *err.set_code(ErrorCode::NOT_CONNECTED);
//     read_callback_ = cb;
//     read_buffer_.resize(4096);
//     auto self = shared_from_this();
//     socket_.async_read_some(
//         asio::buffer(read_buffer_),
//         [this, self](const asio::error_code& ec, std::size_t bytes_transferred) {
//             handleRead(ec, bytes_transferred);
//         });
//     return err;
// }

// void TCPConnection::handleRead(const asio::error_code& error, std::size_t bytes_transferred) {
//     if (!error) {
//         std::vector<uint8_t> data(read_buffer_.begin(), read_buffer_.begin() + bytes_transferred);
//         if (read_callback_)
//             read_callback_(data);
//         // Continue receiving
//         recieve_async(read_callback_);
//     } else {
//         std::cerr << "[TCPConnection] handleRead error: " << error.message() << std::endl;
//         is_connected_ = false;
//         socket_.close();
//     }
// }
#include "tcp.h"

#include <asio.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "client/error.h"   // ✅ Your real Error type

TCPConnection::TCPConnection(asio::io_context& ctx, const NetworkConfig& cfg)
    : io_context_(ctx), socket_(ctx), cfg_(cfg) {}

Error TCPConnection::connect() {
    asio::error_code ec;

    asio::ip::tcp::endpoint ep(
        asio::ip::make_address(cfg_.ip, ec),
        cfg_.port
    );

    if (ec)
        return Error(ErrorCode::CONNECTION_FAILED, "Invalid IP");

    socket_.connect(ep, ec);

    if (ec)
        return Error(ErrorCode::CONNECTION_FAILED, "Connection failed");

    return Error();
}

Error TCPConnection::send_sync(const std::vector<uint8_t>& data) {
    asio::error_code ec;
    asio::write(socket_, asio::buffer(data), ec);

    if (ec)
        return Error(ErrorCode::SEND_FAILED, "Send failed");

    return Error();
}

Error TCPConnection::recieve_sync(std::vector<uint8_t>& out) {
    asio::error_code ec;
    uint8_t buf[1024];

    size_t n = socket_.read_some(asio::buffer(buf), ec);

    if (ec)
        return Error(ErrorCode::RECEIVE_FAILED, "Receive failed");

    out.assign(buf, buf + n);
    return Error();
}
