#include "udp.h"

#include <asio.hpp>
#include <string>
#include <vector>

UDPConnection::UDPConnection(asio::io_context& ctx, const NetworkConfig& cfg)
    : io_context_(ctx), socket_(ctx), cfg_(cfg) {}

Error UDPConnection::connect() {
    asio::error_code ec;

    // Resolve server endpoint
    server_endpoint_ = asio::ip::udp::endpoint(asio::ip::make_address(cfg_.ip, ec), cfg_.port);

    if (ec)
        return Error(ErrorCode::CONNECTION_FAILED, "Invalid IP");

    // Open UDP socket
    socket_.open(asio::ip::udp::v4(), ec);
    if (ec)
        return Error(ErrorCode::CONNECTION_FAILED, "Failed to open UDP socket");

    return Error();
}

Error UDPConnection::send_sync(const std::vector<uint8_t>& data) {
    asio::error_code ec;

    socket_.send_to(asio::buffer(data), server_endpoint_, 0, ec);

    if (ec)
        return Error(ErrorCode::SEND_FAILED, "UDP send failed");

    return Error();
}

Error UDPConnection::recieve_sync(std::vector<uint8_t>& out) {
    asio::error_code ec;
    uint8_t buf[1024];

    asio::ip::udp::endpoint from;
    size_t n = socket_.receive_from(asio::buffer(buf), from, 0, ec);

    if (ec)
        return Error(ErrorCode::RECEIVE_FAILED, "UDP receive failed");

    out.assign(buf, buf + n);
    return Error();
}
