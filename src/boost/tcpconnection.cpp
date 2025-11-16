#include "tcpconnection.h"

#include <boost/asio/connect.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/system/error_code.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

// Constructor taking an existing socket.
TCPConnection::TCPConnection(boost::asio::ip::tcp::socket socket)
    : socket_(std::move(socket)) {}

TCPConnection::~TCPConnection() = default;

// Factory method: create and connect a new instance.
TCPConnection::Ptr TCPConnection::create(boost::asio::io_context& io_context, const std::string& host, unsigned short port) {
    auto connection = std::make_shared<TCPConnection>(boost::asio::ip::tcp::socket(io_context));
    // Initiate async connect:
    boost::asio::ip::tcp::resolver resolver(io_context);
    auto results = resolver.resolve(host, std::to_string(port));
    boost::asio::async_connect(
        connection->socket_, results,
        [connection](const boost::system::error_code& ec, const boost::asio::ip::tcp::endpoint&) {
            if (ec) {
                std::cerr << "[TCPConnection] Error connecting: " << ec.message() << std::endl;
            }
        }
    );
    return connection;
}

// Send data (vector of bytes) to the connected host.
void TCPConnection::send(const std::vector<uint8_t>& data) {
    auto self = shared_from_this();
    boost::asio::async_write(
        socket_, boost::asio::buffer(data),
        [this, self](const boost::system::error_code& ec, std::size_t /*length*/) {
            if (ec) {
                std::cerr << "[TCPConnection] Error sending data: " << ec.message() << std::endl;
            }
        }
    );
}

// Initiates asynchronous read operation; callback with received data.
void TCPConnection::asyncRead(ReceiveCallback cb) {
    auto self = shared_from_this();
    read_buffer_.resize(4096); // Adjust buffer size as needed
    socket_.async_read_some(
        boost::asio::buffer(read_buffer_),
        [this, self, cb](const boost::system::error_code& ec, std::size_t bytes_transferred) {
            if (!ec) {
                std::vector<uint8_t> data(read_buffer_.begin(), read_buffer_.begin() + bytes_transferred);
                cb(data);
                // Continue reading asynchronously for next data
                this->asyncRead(cb);
            } else {
                std::cerr << "[TCPConnection] Error during read: " << ec.message() << std::endl;
                socket_.close();
            }
        }
    );
}

bool TCPConnection::isConnected() const {
    return socket_.is_open();
}

std::string TCPConnection::description() const {
    try {
        boost::asio::ip::tcp::endpoint remote_endpoint = socket_.remote_endpoint();
        return remote_endpoint.address().to_string() + ":" + std::to_string(remote_endpoint.port());
    } catch (const std::exception&) {
        return "Not connected";
    }
}
