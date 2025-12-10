#include "udp.h"

#include <asio.hpp>
#include <string>
#include <vector>

UDPupdate::UDPupdate(asio::io_context& ctx, const NetworkConfig& cfg)
    : io_context_(ctx), socket_(ctx), server_endpoint_(), cfg_(cfg) {
    // Nothing else needed
}

Error UDPupdate::Open() {
    asio::error_code ec;

    // Resolve server endpoint
    server_endpoint_ = asio::ip::udp::endpoint(asio::ip::make_address(cfg_.ip, ec), cfg_.port);

    if (ec)
        return Error(ErrorCode::CONNECTION_FAILED, "Invalid IP");

    // Open UDP socket
    socket_.open(asio::ip::udp::v4(), ec);
    if (ec)
        return Error(ErrorCode::CONNECTION_FAILED, "Failed to open UDP socket");

    // CRITICAL: Bind to ANY local port so we can receive replies
    socket_.bind(asio::ip::udp::endpoint(asio::ip::udp::v4(), 0), ec);
    if (ec)
        return Error(ErrorCode::CONNECTION_FAILED, "Failed to bind UDP socket");

    return Error();
}

Error UDPupdate::send_async(const std::vector<uint8_t>& data, std::function<void(Error)> callback) {
    socket_.async_send_to(
        asio::buffer(data), server_endpoint_,
        [callback](const asio::error_code& ec, std::size_t /*bytes_transferred*/) {
            if (ec) {
                callback(Error(ErrorCode::SEND_FAILED, "UDP async send failed"));
            } else {
                callback(Error());
            }
        });
    return Error();
}

Error UDPupdate::recieve_async(std::function<void(const std::vector<uint8_t>&, Error)> callback) {
    auto buf = std::make_shared<std::vector<uint8_t>>(1024);
    auto sender_endpoint = std::make_shared<asio::ip::udp::endpoint>();

    socket_.async_receive_from(
        asio::buffer(*buf), *sender_endpoint,
        [buf, callback](const asio::error_code& ec, std::size_t bytes_received) {
            if (ec) {
                callback({}, Error(ErrorCode::RECEIVE_FAILED, "UDP async receive failed"));
            } else {
                buf->resize(bytes_received);
                callback(*buf, Error());
            }
        });
    return Error();
}
