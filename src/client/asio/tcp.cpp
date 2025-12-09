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
