#include "tcp_client.h"

TcpClientAsio::TcpClientAsio(const NetworkConfig& cfg)
    : ClientInterface(cfg),
      work_guard_(asio::make_work_guard(io_)),
      socket_(io_),
      strand_(asio::make_strand(io_)) {
    io_thread_ = std::thread([this] { run_io(); });
}

TcpClientAsio::~TcpClientAsio() {
    disconnect();
    work_guard_.reset();
    io_.stop();
    if (io_thread_.joinable())
        io_thread_.join();
}

void TcpClientAsio::run_io() {
    try {
        io_.run();
    } catch (...) {
        // log or ignore
    }
}

// ====================== CONNECT (SYNC) ======================

Error TcpClientAsio::connect() {
    asio::error_code ec;

    // recreate socket to clear any previous error state
    socket_ = asio::ip::tcp::socket(io_);

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

// ====================== CONNECT (ASYNC) ======================

Error TcpClientAsio::connect_async(std::function<void(Error)> callback) {
    asio::error_code ec;
    auto addr = asio::ip::make_address(cfg_.ip, ec);

    if (ec) {
        Error err;
        err.set_code(ErrorCode::INVALID_ADDRESS)->set_message("Invalid IP");
        callback(err);
        return Error{};
    }

    asio::ip::tcp::endpoint ep(addr, cfg_.port);

    // recreate socket
    socket_ = asio::ip::tcp::socket(io_);

    socket_.async_connect(
        ep, asio::bind_executor(strand_, [this, callback](const asio::error_code& ec2) {
            if (ec2) {
                Error err;
                err.set_code(ErrorCode::CONNECTION_FAILED)->set_message("Async connect failed");
                callback(err);
                start_reconnect_loop();
            } else {
                reconnecting_ = false;
                callback(Error{});
            }
        }));

    return Error{};
}

// ====================== RECONNECT LOOP ======================

void TcpClientAsio::start_reconnect_loop() {
    if (reconnecting_.exchange(true))
        return;  // already reconnecting

    asio::post(strand_, [this] {
        int delay = 1;

        auto attempt_ptr = std::make_shared<std::function<void()>>();
        *attempt_ptr = [this, attempt_ptr, delay]() mutable {
            if (!reconnecting_)
                return;

            // recreate socket for every attempt
            socket_ = asio::ip::tcp::socket(io_);

            asio::error_code ec;
            auto addr = asio::ip::make_address(cfg_.ip, ec);
            if (ec) {
                // invalid address is fatal, stop reconnecting
                reconnecting_ = false;
                return;
            }

            asio::ip::tcp::endpoint ep(addr, cfg_.port);

            socket_.async_connect(
                ep, asio::bind_executor(
                        strand_, [this, attempt_ptr, delay](const asio::error_code& ec2) mutable {
                            if (!ec2) {
                                reconnecting_ = false;
                                return;
                            }

                            int new_delay = std::min(delay * 2, 30);

                            auto timer_ptr = std::make_shared<asio::steady_timer>(
                                io_, std::chrono::seconds(new_delay));

                            timer_ptr->async_wait(asio::bind_executor(
                                strand_, [attempt_ptr, timer_ptr](const asio::error_code&) mutable {
                                    (*attempt_ptr)();
                                }));
                        }));
        };

        (*attempt_ptr)();
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
                                std::function<void(Error)> callback) {
    asio::async_write(
        socket_, asio::buffer(data),
        asio::bind_executor(strand_, [callback](const asio::error_code& ec, std::size_t) {
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

Error TcpClientAsio::recieve_async(
    std::function<void(const std::vector<uint8_t>&, Error)> callback) {
    auto buf = std::make_shared<std::vector<uint8_t>>(1024);

    socket_.async_read_some(
        asio::buffer(*buf),
        asio::bind_executor(
            strand_, [this, buf, callback](const asio::error_code& ec, std::size_t n) {
                if (ec) {
                    Error err;
                    err.set_code(ErrorCode::RECEIVE_FAILED)->set_message("Async receive failed");
                    callback(std::vector<uint8_t>{}, err);
                    start_reconnect_loop();
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

    return Error{};
}
