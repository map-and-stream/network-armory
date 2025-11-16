#include "udpconnection.h"

#include <boost/asio/ip/udp.hpp>
#include <boost/system/error_code.hpp>
#include <iostream>

// Factory method - not used directly in header, but implemented for completeness.
UDPConnection::Ptr UDPConnection::create(boost::asio::io_context& io_context,
                                         const std::string& host, unsigned short port) {
    return std::make_shared<UDPConnection>(io_context, host, port);
}

UDPConnection::UDPConnection(boost::asio::io_context& io_context, const std::string& host,
                             unsigned short port)
    : socket_(io_context, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 0)),
      io_context_(io_context),
      read_buffer_(2048)  // pre-allocate a reasonable buffer size for UDP datagram
{
    connect(host, port);
}

void UDPConnection::connect(const std::string& host, unsigned short port) {
    boost::asio::ip::udp::resolver resolver(io_context_);
    try {
        auto results = resolver.resolve(host, std::to_string(port));
        remote_endpoint_ = *results.begin();
        std::cout << "[UDPConnection] Connected to: " << description() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[UDPConnection] Error resolving UDP endpoint: " << e.what() << std::endl;
    }
}

UDPConnection::~UDPConnection() = default;

void UDPConnection::send(const std::vector<uint8_t>& data) {
    // UDP is connectionless; just send the datagram
    boost::system::error_code ec;
    auto bytes_sent = socket_.send_to(boost::asio::buffer(data), remote_endpoint_, 0, ec);
    if (ec) {
        std::cerr << "[UDPConnection] Error sending data: " << ec.message() << std::endl;
    }
}

void UDPConnection::asyncRead(ReceiveCallback cb) {
    read_callback_ = cb;
    auto self = shared_from_this();
    socket_.async_receive_from(
        boost::asio::buffer(read_buffer_), remote_endpoint_,
        [this, self](const boost::system::error_code& ec, std::size_t bytes_transferred) {
            handleRead(ec, bytes_transferred);
        });
}

void UDPConnection::handleRead(const boost::system::error_code& error,
                               std::size_t bytes_transferred) {
    if (!error) {
        if (read_callback_) {
            // Trim buffer to amount actually received
            std::vector<uint8_t> data(read_buffer_.begin(),
                                      read_buffer_.begin() + bytes_transferred);
            read_callback_(data);
        }
        // Continue receiving asynchronously
        asyncRead(read_callback_);
    } else {
        std::cerr << "[UDPConnection] Error during receive: " << error.message() << std::endl;
    }
}

bool UDPConnection::isConnected() const {
    // UDP is usually "connected" -- always return true
    return socket_.is_open();
}

std::string UDPConnection::description() const {
    try {
        return remote_endpoint_.address().to_string() + ":" +
               std::to_string(remote_endpoint_.port());
    } catch (...) {
        return "Not connected";
    }
}
