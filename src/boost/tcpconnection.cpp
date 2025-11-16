// TCPConnection.cpp
#include "tcpconnection.h"
#include "thread"

#include <stdexcept>

TCPConnection::TCPConnection(boost::asio::ip::tcp::socket socket)
    : socket_(std::move(socket))
{
}

TCPConnection::~TCPConnection()
{
}

void TCPConnection::connect(const std::string &host, unsigned short port)
{
    // Ensure self is held as a shared_ptr
    auto self(shared_from_this());
    boost::asio::ip::tcp::resolver resolver(socket_.get_executor());

    resolver.async_resolve(host, std::to_string(port),
                           [this, self](const boost::system::error_code& ec, boost::asio::ip::tcp::resolver::results_type results) {
        if (!ec) {
            // Try to connect to the resolved endpoint
            boost::asio::async_connect(socket_, results,
                                       [this, self](const boost::system::error_code& ec, const boost::asio::ip::tcp::endpoint&) {
                if (!ec) {
                    std::cout << "Connected to server (TCPConnection): " << description() << std::endl;
                } else {
                    std::cerr << "Error connecting to server (TCPConnection): " << ec.message() << std::endl;
                }
            });
        } else {
            std::cerr << "Error resolving host (TCPConnection): " << ec.message() << std::endl;
        }
    });
    //with help mohammad imani very goooooood
    std::this_thread::sleep_for(std::chrono::seconds(1));
}


void TCPConnection::send(const std::string& message, const std::string &topic) {
    auto self(shared_from_this());
    //    boost::asio::write(socket, boost::asio::buffer(message));
    boost::asio::async_write(socket_, boost::asio::buffer(message.data(), message.length()),
                             [this, self](boost::system::error_code ec, std::size_t/*length*/) {
        if (!ec) {
            std::cout << "Writed data (TCPConnection): " << std::endl;
        } else {
            std::cerr << "Write error: (TCPConnection)" << ec.message() << std::endl;
        }
    });
}

void TCPConnection::asyncRead(std::function<void(const std::string&, const std::string&)> callback) {
    read_callback = callback;
    auto self = shared_from_this();
    socket_.async_read_some(boost::asio::buffer(read_buffer),
                            std::bind(&TCPConnection::handleRead, this,
                                      std::placeholders::_1, std::placeholders::_2));
}

bool TCPConnection::isConnected() const {
    auto self = shared_from_this();
    return socket_.is_open();
}

std::string TCPConnection::description() const
{
    boost::asio::ip::tcp::endpoint remote_endpoint = socket_.remote_endpoint();
    return remote_endpoint.address().to_string() + ":" + std::to_string(remote_endpoint.port());
}

void TCPConnection::handleRead(const boost::system::error_code& error, std::size_t bytes_transferred) {
    if (!error) {
        // Create a std::string from the read buffer
        std::string message(read_buffer.data(), bytes_transferred);
        if (read_callback) {
            read_callback(message, "all"); // Call the user-defined callback
        }
        std::cout << " read (TCPConnection): " << std::endl;
        // Continue reading asynchronously
        asyncRead(read_callback);
    } else {
        std::cerr << "Error during read (TCPConnection): " << error.message() << std::endl;
        socket_.close(); // Close the socket on error
    }
}
