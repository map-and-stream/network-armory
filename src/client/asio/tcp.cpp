#include "tcp.h"

#include "client/client_interface.h"
#include "client/error.h"

TCP::TCP(asio::io_context& ctx, const NetworkConfig& cfg)
    : ClientInterface(cfg), io_context_(ctx), socket_(ctx) {}

Error TCP::connect() {
    asio::error_code ec;

    auto addr = asio::ip::make_address(cfg_.ip, ec);
    if (ec) {
        Error err;
        err.set_code(ErrorCode::INVALID_ADDRESS)->set_message("Invalid IP");
        return err;
    }

    asio::ip::tcp::endpoint ep(addr, cfg_.port);
    socket_.connect(ep, ec);

    if (ec) {
        Error err;
        err.set_code(ErrorCode::SERVER_UNAVAILABLE)->set_message("Server is offline");
        return err;
    }

    return Error{};
}

Error TCP::connect_async(std::function<void(Error)> callback) {
    asio::error_code ec;

    auto addr = asio::ip::make_address(cfg_.ip, ec);
    if (ec) {
        Error err;
        err.set_code(ErrorCode::INVALID_ADDRESS)->set_message("Invalid IP");
        callback(err);
        return Error{};
    }

    asio::ip::tcp::endpoint ep(addr, cfg_.port);

    socket_.async_connect(ep, [callback](const asio::error_code& ec2) {
        if (ec2) {
            Error err;
            err.set_code(ErrorCode::CONNECTION_FAILED)->set_message("Async connect failed");
            callback(err);
        } else {
            callback(Error{});
        }
    });

    return Error{};
}

Error TCP::send_sync(const std::vector<uint8_t>& data) {
    asio::error_code ec;
    asio::write(socket_, asio::buffer(data), ec);

    if (ec) {
        Error err;
        err.set_code(ErrorCode::SEND_FAILED)->set_message("Send failed");
        return err;
    }

    return Error{};
}

Error TCP::send_async(const std::vector<uint8_t>& data, std::function<void(Error)> callback) {
    asio::async_write(
        socket_, asio::buffer(data),
        [callback](const asio::error_code& ec, std::size_t /*bytes_transferred*/) {
            if (ec) {
                Error err;
                err.set_code(ErrorCode::SEND_FAILED)->set_message("Async send failed");
                callback(err);
            } else {
                callback(Error{});
            }
        });
    return Error{};
}

Error TCP::recieve_sync(std::vector<uint8_t>& out) {
    asio::error_code ec;
    uint8_t buf[1024];

    size_t n = socket_.read_some(asio::buffer(buf), ec);

    if (ec) {
        Error err;
        err.set_code(ErrorCode::RECEIVE_FAILED)->set_message("Receive failed");
        return err;
    }

    out.assign(buf, buf + n);
    return Error{};
}

Error TCP::recieve_async(std::function<void(const std::vector<uint8_t>&, Error)> callback) {
    auto buf = std::make_shared<std::vector<uint8_t>>(1024);
    socket_.async_read_some(asio::buffer(*buf), [buf, callback](const asio::error_code& ec,
                                                                std::size_t bytes_transferred) {
        if (ec) {
            Error err;
            err.set_code(ErrorCode::RECEIVE_FAILED)->set_message("Async receive failed");
            callback({}, err);
        } else {
            buf->resize(bytes_transferred);
            callback(*buf, Error{});
        }
    });
    return Error{};
}

Error TCP::disconnect() {
    asio::error_code ec;
    socket_.close(ec);
    Error err;
    if (ec.value() != 0) {
        err.set_code(ErrorCode::DISCONNECTION_FAILED);
    }
    return err;
}
