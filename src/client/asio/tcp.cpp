#include "tcp.h"

#include <asio.hpp>
#include <string>
#include <vector>

#include "client/error.h"  // Your real Error type

TCPConnection::TCPConnection(asio::io_context& ctx, const NetworkConfig& cfg)
    : io_context_(ctx), socket_(ctx), cfg_(cfg) {}

Error TCPConnection::connect() {
    asio::error_code ec;

    asio::ip::tcp::endpoint ep(asio::ip::make_address(cfg_.ip, ec), cfg_.port);

    if (ec)
        return Error(ErrorCode::CONNECTION_FAILED, "Invalid IP");

    socket_.connect(ep, ec);

    if (ec)
        return Error(ErrorCode::CONNECTION_FAILED, "Connection failed");

    return Error();
}
Error TCPConnection::connect_async(std::function<void(Error)> callback) {
    asio::ip::tcp::endpoint ep(asio::ip::tcp::endpoint(asio::ip::make_address(cfg_.ip), cfg_.port));

    socket_.async_connect(ep, [callback](const asio::error_code& ec) {
        if (ec)
            callback(Error(ErrorCode::CONNECTION_FAILED, "Async connect failed"));
        else
            callback(Error());
    });

    return Error();
}

Error TCPConnection::send_sync(const std::vector<uint8_t>& data) {
    asio::error_code ec;
    asio::write(socket_, asio::buffer(data), ec);

    if (ec)
        return Error(ErrorCode::SEND_FAILED, "Send failed");

    return Error();
}

Error TCPConnection::send_async(const std::vector<uint8_t>& data,
                                std::function<void(Error)> callback) {
    asio::async_write(socket_, asio::buffer(data),
                      [callback](const asio::error_code& ec, std::size_t /*bytes_transferred*/) {
                          if (ec) {
                              callback(Error(ErrorCode::SEND_FAILED, "Async send failed"));
                          } else {
                              callback(Error());
                          }
                      });
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

Error TCPConnection::recieve_async(
    std::function<void(const std::vector<uint8_t>&, Error)> callback) {
    auto buf = std::make_shared<std::vector<uint8_t>>(1024);
    socket_.async_read_some(asio::buffer(*buf), [buf, callback](const asio::error_code& ec,
                                                                std::size_t bytes_transferred) {
        if (ec) {
            callback({}, Error(ErrorCode::RECEIVE_FAILED, "Async receive failed"));
        } else {
            buf->resize(bytes_transferred);
            callback(*buf, Error());
        }
    });
    return Error();
}
