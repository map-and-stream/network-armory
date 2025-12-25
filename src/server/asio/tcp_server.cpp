#include "tcp_server.h"

TcpServerAsio::TcpServerAsio(
    ServerConfig cfg,
    ReceiveCallback receiveCallback,
    ClientConnectCallback clientConnectCallback,
    ClientDisconnectCallback clientDisconnectCallback
)
    : ServerInterface(cfg, receiveCallback, clientConnectCallback, clientDisconnectCallback),
      acceptor_(io_context_),
      socket_(io_context_) {}

TcpServerAsio::~TcpServerAsio() {
    gracefull_shutdown();
}

// Listen and start the server
Error TcpServerAsio::listen() {
    try {
        asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), cfg_.port);
        acceptor_.open(endpoint.protocol());
        acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
        acceptor_.bind(endpoint);
        acceptor_.listen();
        running_ = true;
        do_accept();

        io_thread_ = std::thread([this]() {
            io_context_.run();
        });
        return Error();
    } catch (const std::exception& ex) {
        return *Error().set_code(ErrorCode::PORT_IN_USE)->set_message(ex.what());
    }
}

// Send data to client by "fd" (Here, fd is actually the internal connection id)
Error TcpServerAsio::send(int fd, const std::vector<uint8_t>& data) {
    std::lock_guard<std::mutex> lock(connections_mutex_);
    auto it = connections_.find(fd);
    if (it == connections_.end()) {
        return *Error().set_code(ErrorCode::NOT_CONNECTED)->set_message("Connection not found.");
    }
    auto sock = it->second;
    auto buf = std::make_shared<std::vector<uint8_t>>(data);

    asio::async_write(
        *sock, asio::buffer(*buf),
        [buf](std::error_code /*ec*/, std::size_t /*bytes_transferred*/) {
            // handle errors/logging here if required
        });
    return Error();
}

// Send data to client by IP (send to first matching IP)
Error TcpServerAsio::send(const std::string& ip, const std::vector<uint8_t>& data) {
    std::lock_guard<std::mutex> lock(connections_mutex_);
    for (const auto& kv : connections_ip_) {
        if (kv.second == ip) {
            return send(kv.first, data);
        }
    }
    return *Error().set_code(ErrorCode::NOT_CONNECTED)->set_message("IP not found.");
}

Error TcpServerAsio::gracefull_shutdown() {
    if (!running_)
        return Error();

    running_ = false;
    try {
        {
            std::lock_guard<std::mutex> lock(connections_mutex_);
            for (auto& kv : connections_) {
                asio::error_code ec;
                kv.second->shutdown(asio::ip::tcp::socket::shutdown_both, ec);
                kv.second->close(ec);
            }
            connections_.clear();
            connections_ip_.clear();
        }
        asio::error_code ignored_ec;
        acceptor_.close(ignored_ec);
        socket_.close(ignored_ec);
        io_context_.stop();
        if (io_thread_.joinable()) {
            io_thread_.join();
        }
        return Error();
    } catch (const std::exception& ex) {
        return *Error().set_code(ErrorCode::DISCONNECTION_FAILED)->set_message(ex.what());
    }
}

void TcpServerAsio::do_accept() {
    acceptor_.async_accept(socket_, [this](std::error_code ec) {
        if (!running_) return;

        if (!ec) {
            int conn_id = next_conn_id_++;
            auto new_socket = std::make_shared<asio::ip::tcp::socket>(std::move(socket_));
            std::string client_ip = new_socket->remote_endpoint().address().to_string();
            {
                std::lock_guard<std::mutex> lock(connections_mutex_);
                connections_[conn_id] = new_socket;
                connections_ip_[conn_id] = client_ip;
            }
            if (clientConnectionCallback_) {
                clientConnectionCallback_(conn_id, client_ip);
            }
            do_read(conn_id, new_socket, client_ip);
        }
        if (running_) {
            do_accept();
        }
    });
}

void TcpServerAsio::do_read(int conn_id, std::shared_ptr<asio::ip::tcp::socket> sock, std::string client_ip) {
    auto buf = std::make_shared<std::vector<uint8_t>>(4096);
    sock->async_receive(
        asio::buffer(*buf),
        [this, sock, buf, conn_id, client_ip](std::error_code ec, std::size_t bytes_transferred) {
            if (!ec && bytes_transferred > 0) {
                buf->resize(bytes_transferred);
                if (recieveCallback_) {
                    recieveCallback_(conn_id, client_ip, *buf);
                }
                do_read(conn_id, sock, client_ip);
            } else {
                // Connection closed or error
                {
                    std::lock_guard<std::mutex> lock(connections_mutex_);
                    connections_.erase(conn_id);
                    connections_ip_.erase(conn_id);
                }
                if (clientDisconnectCallback_) {
                    clientDisconnectCallback_(conn_id, client_ip);
                }
            }
        });
}
