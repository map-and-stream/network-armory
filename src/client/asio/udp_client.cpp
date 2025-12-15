#include "client/asio/udp_client.h"

UdpClient::UdpClient(const NetworkConfig& cfg,
                     std::shared_ptr<asio::io_context> io)
    : ClientInterface(cfg),
      io_(std::move(io)),
      socket_(*io_),
      server_endpoint_()
{
}

// ====================== CONNECT (SYNC) ======================

Error UdpClient::connect() {
    asio::error_code ec;

    auto addr = asio::ip::make_address(cfg_.ip, ec);
    if (ec) {
        Error err;
        err.set_code(ErrorCode::INVALID_ADDRESS)->set_message("Invalid IP");
        return err;
    }

    server_endpoint_ = asio::ip::udp::endpoint(addr, cfg_.port);

    socket_.open(asio::ip::udp::v4(), ec);
    if (ec) {
        Error err;
        err.set_code(ErrorCode::CONNECTION_FAILED)->set_message("Failed to open UDP socket");
        return err;
    }

    socket_.bind(asio::ip::udp::endpoint(asio::ip::udp::v4(), 0), ec);
    if (ec) {
        Error err;
        err.set_code(ErrorCode::CONNECTION_FAILED)->set_message("Failed to bind UDP socket");
        return err;
    }

    is_connected_ = true;
    return Error{};
}

// ====================== CONNECT (ASYNC) ======================

Error UdpClient::connect_async(AsyncCallback callback) {
    Error err = connect();
    callback(err);
    return err;
}

// ====================== DISCONNECT ======================

Error UdpClient::disconnect() {
    asio::error_code ec;
    socket_.close(ec);
    is_connected_ = false;
    return Error{};
}

// ====================== SEND (ASYNC) ======================

Error UdpClient::send_async(const std::vector<uint8_t>& data,
                            AsyncCallback callback)
{
    auto self = shared_from_this();

    socket_.async_send_to(
        asio::buffer(data), server_endpoint_,
        [self, callback](const asio::error_code& ec, std::size_t) {
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

// ====================== RECEIVE (ASYNC) ======================

Error UdpClient::recieve_async(ReceiveCallback callback) {
    auto self = shared_from_this();
    auto buf    = std::make_shared<std::vector<uint8_t>>(1024);
    auto sender = std::make_shared<asio::ip::udp::endpoint>();

    socket_.async_receive_from(
        asio::buffer(*buf), *sender,
        [self, buf, callback](const asio::error_code& ec, std::size_t bytes) {
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
