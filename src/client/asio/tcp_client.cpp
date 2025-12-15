#include "client/asio/tcp_client.h"

TcpClientAsio::TcpClientAsio(const NetworkConfig& cfg,
                             std::shared_ptr<asio::io_context> io)
    : ClientInterface(cfg),
      io_(std::move(io)),
      socket_(*io_),
      strand_(asio::make_strand(*io_)) {}

TcpClientAsio::~TcpClientAsio() {
    disconnect();
}

// ====================== CONNECT (SYNC) ======================

Error TcpClientAsio::connect() {
    asio::error_code ec;

    socket_ = asio::ip::tcp::socket(*io_);

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

    is_connected_ = true;
    return Error{};
}

// ====================== CONNECT (ASYNC) ======================

Error TcpClientAsio::connect_async(AsyncCallback callback) {
    asio::error_code ec;
    auto addr = asio::ip::make_address(cfg_.ip, ec);

    if (ec) {
        Error err;
        err.set_code(ErrorCode::INVALID_ADDRESS)->set_message("Invalid IP");
        callback(err);
        return err;
    }

    asio::ip::tcp::endpoint ep(addr, cfg_.port);

    socket_ = asio::ip::tcp::socket(*io_);

    auto self = shared_from_this();
    socket_.async_connect(
        ep,
        asio::bind_executor(
            strand_,
            [self, callback](const asio::error_code& ec2) {
                if (ec2) {
                    Error err;
                    err.set_code(ErrorCode::CONNECTION_FAILED)->set_message("Async connect failed");
                    callback(err);
                    self->start_reconnect_loop();
                } else {
                    self->reconnecting_ = false;
                    self->is_connected_ = true;
                    callback(Error{});
                }
            }));

    return Error{};
}

// ====================== RECONNECT LOOP ======================

void TcpClientAsio::start_reconnect_loop() {
    if (reconnecting_.exchange(true))
        return;

    auto self = shared_from_this();

    asio::post(strand_, [self] {
        struct State {
            int delay = 1;
            std::function<void()> attempt;
        };
        auto state = std::make_shared<State>();

        state->attempt = [self, state]() mutable {
            if (!self->reconnecting_)
                return;

            self->socket_ = asio::ip::tcp::socket(*self->io_);

            asio::error_code ec;
            auto addr = asio::ip::make_address(self->cfg_.ip, ec);
            if (ec) {
                self->reconnecting_ = false;
                return;
            }

            asio::ip::tcp::endpoint ep(addr, self->cfg_.port);

            self->socket_.async_connect(
                ep,
                asio::bind_executor(
                    self->strand_,
                    [self, state](const asio::error_code& ec2) mutable {
                        if (!ec2) {
                            self->reconnecting_ = false;
                            self->is_connected_ = true;
                            return;
                        }

                        state->delay = std::min(state->delay * 2, 30);
                        auto timer = std::make_shared<asio::steady_timer>(
                            *self->io_, std::chrono::seconds(state->delay));

                        timer->async_wait(
                            asio::bind_executor(
                                self->strand_,
                                [state, timer](const asio::error_code&) mutable {
                                    state->attempt();
                                }));
                    }));
        };

        state->attempt();
    });
}

// ====================== SEND (SYNC) ======================

Error TcpClientAsio::send_sync(const std::vector<uint8_t>& data) {
    asio::error_code ec;
    asio::write(socket_, asio::buffer(data), ec);

    if (ec) {
        Error err;
        err.set_code(ErrorCode::SEND_FAILED)->set_message("Send failed");
        return err;
    }

    return Error{};
}

// ====================== SEND (ASYNC) ======================

Error TcpClientAsio::send_async(const std::vector<uint8_t>& data,
                                AsyncCallback callback) {
    auto self = shared_from_this();

    asio::async_write(
        socket_,
        asio::buffer(data),
        asio::bind_executor(
            strand_,
            [self, callback](const asio::error_code& ec, std::size_t) {
                if (ec) {
                    Error err;
                    err.set_code(ErrorCode::SEND_FAILED)->set_message("Async send failed");
                    callback(err);
                } else {
                    callback(Error{});
                }
            }));

    return Error{};
}

// ====================== RECEIVE (SYNC) ======================

Error TcpClientAsio::recieve_sync(std::vector<uint8_t>& out) {
    asio::error_code ec;
    uint8_t buf[1024];

    std::size_t n = socket_.read_some(asio::buffer(buf), ec);

    if (ec) {
        Error err;
        err.set_code(ErrorCode::RECEIVE_FAILED)->set_message("Receive failed");
        return err;
    }

    out.assign(buf, buf + n);
    return Error{};
}

// ====================== RECEIVE (ASYNC) ======================

Error TcpClientAsio::recieve_async(ReceiveCallback callback) {
    auto self = shared_from_this();
    auto buf = std::make_shared<std::vector<uint8_t>>(1024);

    socket_.async_read_some(
        asio::buffer(*buf),
        asio::bind_executor(
            strand_,
            [self, buf, callback](const asio::error_code& ec, std::size_t n) {
                if (ec) {
                    Error err;
                    err.set_code(ErrorCode::RECEIVE_FAILED)->set_message("Async receive failed");
                    callback(std::vector<uint8_t>{}, err);
                    self->start_reconnect_loop();
                } else {
                    buf->resize(n);
                    callback(*buf, Error{});
                }
            }));

    return Error{};
}

// ====================== DISCONNECT ======================

Error TcpClientAsio::disconnect() {
    reconnecting_ = false;

    asio::error_code ec;
    socket_.close(ec);

    if (ec) {
        Error err;
        err.set_code(ErrorCode::DISCONNECTION_FAILED)->set_message("Disconnection failed");
        return err;
    }

    is_connected_ = false;
    return Error{};
}
