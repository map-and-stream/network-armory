#include "serialconnection.h"
#include <boost/asio/write.hpp>
#include <iostream>
#include <memory>

SerialConnection::SerialConnection(boost::asio::io_context& io_context, const std::string& port, uint32_t baud)
    : serial_(io_context), read_buffer_(256), port_name_(port), baud_rate_(baud)
{
    open(port, baud);
}

SerialConnection::~SerialConnection() = default;

void SerialConnection::open(const std::string& port, uint32_t baud)
{
    try {
        serial_.open(port);
        serial_.set_option(boost::asio::serial_port_base::baud_rate(baud));
        // Optionally, more settings (parity, stop bits, etc) could be set here.
    } catch(const std::exception& e) {
        std::cerr << "[SerialConnection] Failed to open port " << port << ": " << e.what() << std::endl;
    }
}

void SerialConnection::send(const std::vector<uint8_t>& data)
{
    if (serial_.is_open()) {
        auto self = shared_from_this();
        boost::asio::async_write(serial_, boost::asio::buffer(data),
            [this, self](const boost::system::error_code& ec, std::size_t /*length*/) {
                if (ec) {
                    std::cerr << "[SerialConnection] Write error: " << ec.message() << std::endl;
                }
            }
        );
    }
}

void SerialConnection::asyncRead(ReceiveCallback cb)
{
    read_callback_ = cb;
    auto self = shared_from_this();
    serial_.async_read_some(
        boost::asio::buffer(read_buffer_),
        [this, self](const boost::system::error_code& error, std::size_t bytes_transferred) {
            handleRead(error, bytes_transferred);
        }
    );
}

bool SerialConnection::isConnected() const
{
    return serial_.is_open();
}

std::string SerialConnection::description() const
{
    boost::asio::serial_port_base::baud_rate baudrate;
    try {
        serial_.get_option(baudrate);
    } catch (...) {
        baudrate = boost::asio::serial_port_base::baud_rate(0);
    }
    return port_name_ + ":" + std::to_string(baudrate.value());
}

void SerialConnection::handleRead(const boost::system::error_code& error, std::size_t bytes_transferred)
{
    if (!error) {
        std::vector<uint8_t> data(read_buffer_.begin(), read_buffer_.begin() + bytes_transferred);
        if (read_callback_) {
            read_callback_(data);
        }
        asyncRead(read_callback_);
    } else {
        std::cerr << "[SerialConnection] Error during read: " << error.message() << std::endl;
        serial_.close(); // Close the serial port on error
    }
}
