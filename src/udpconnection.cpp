// UDPConnection.cpp
#include "udpconnection.h"

UDPConnection::UDPConnection(boost::asio::io_context &io_context, const std::string &host, int port)
    : socket_(io_context, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 0)) {
    boost::asio::ip::udp::resolver resolver(io_context);
    try {
        remote_endpoint_ = *resolver.resolve(host, std::to_string(port)).begin();
        std::cout << "resolve UDP Connection (UDPConnection): " << description() << std::endl;
    } catch (std::exception& e) {
        std::cerr << "Error resolve UDP connection (UDPConnection): " << e.what() << "\n";
    }

}

UDPConnection::~UDPConnection()
{

}

void UDPConnection::send(const std::string& message, const std::string &topic) {
    auto self = shared_from_this();
    size_t byte_send = socket_.send_to(boost::asio::buffer(message), remote_endpoint_);
    if (byte_send > 0) {
        std::cout << "Writed data (UDPConnection): " << std::endl;
    } else {
        std::cerr << "Write error: (UDPConnection)" << std::endl;
    }
}

void UDPConnection::asyncRead(std::function<void(const std::string&, const std::string&)> callback) {
    read_callback = callback;
    auto self = shared_from_this();
    socket_.async_receive_from(boost::asio::buffer(read_buffer),
                              remote_endpoint_,
                              std::bind(&UDPConnection::handleRead, this,
                                        std::placeholders::_1, std::placeholders::_2));
}

bool UDPConnection::isConnected() const {
    // For UDP, return true as it does not have a persistent connection.
    return true;
}

std::string UDPConnection::description() const
{
    return remote_endpoint_.address().to_string() + ":" + std::to_string(remote_endpoint_.port());
}

void UDPConnection::handleRead(const boost::system::error_code& error, std::size_t bytes_transferred) {
    if (!error) {
        // Create a std::string from the receive buffer
        std::string message(read_buffer.data(), bytes_transferred);
        if (read_callback) {
            read_callback(message, "all"); // Call the user-defined callback
        }
        std::cout << " read (UDPConnection): " << std::endl;
        // Continue receiving asynchronously
        asyncRead(read_callback);
    } else {
        std::cerr << "Error during receive (UDPConnection): " << error.message() << std::endl;
    }
}
