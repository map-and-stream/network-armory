#include "udp.h"

UDP::UDP(asio::io_context& ctx, const NetworkConfig& cfg)
    : io_context_(ctx), socket_(ctx), server_endpoint_(), cfg_(cfg) {}

Error UDP::Open() {
    asio::error_code ec;

    server_endpoint_ = asio::ip::udp::endpoint(asio::ip::make_address(cfg_.ip, ec), cfg_.port);

    if (ec) {
        Error err;
        err.set_code(ErrorCode::INVALID_ADDRESS)->set_message("Invalid IP address");
        return err;
    }

    socket_.open(asio::ip::udp::v4(), ec);
    if (ec) {
        Error err;
        err.set_code(ErrorCode::CONNECTION_FAILED)->set_message("Failed to open UDP socket");
        return err;
    }

    socket_.bind(asio::ip::udp::endpoint(asio::ip::udp::v4(), 0), ec);
    if (ec) {
        Error err;
        err.set_code(ErrorCode::CONNECTION_FAILED)->set_message("Failed to bind UDP client socket");
        return err;
    }

    return Error{};
}

Error UDP::send_async(const std::vector<uint8_t>& data, std::function<void(Error)> callback) {
    socket_.async_send_to(
        asio::buffer(data), server_endpoint_, [callback](const asio::error_code& ec, std::size_t) {
            if (ec) {
                Error err;
                err.set_code(ErrorCode::SEND_FAILED)->set_message("UDP async send failed");
                callback(err);
            } else {
                callback(Error{});
            }
        });

    return Error{};
}

Error UDP::recieve_async(std::function<void(const std::vector<uint8_t>&, Error)> callback) {
    auto buf = std::make_shared<std::vector<uint8_t>>(1024);
    auto sender = std::make_shared<asio::ip::udp::endpoint>();

    socket_.async_receive_from(
        asio::buffer(*buf), *sender,
        [buf, callback](const asio::error_code& ec, std::size_t bytes) {
            if (ec) {
                Error err;
                err.set_code(ErrorCode::RECEIVE_FAILED)->set_message("UDP async receive failed");
                callback({}, err);
            } else {
                buf->resize(bytes);
                callback(*buf, Error{});
            }
        });

    return Error{};
}
