// SerialConnection.cpp
#include "serialconnection.h"

SerialConnection::SerialConnection(boost::asio::io_context &io_context, const std::string& portname, unsigned int baud_rate)
    : serial(io_context), port_name(portname){
    serial.open(portname);
    serial.set_option(boost::asio::serial_port_base::baud_rate(baud_rate));
}

SerialConnection::~SerialConnection()
{

}

void SerialConnection::send(const std::string& message, const std::string &topic) {

    if (serial.is_open()) {
        auto self = shared_from_this();
        boost::asio::async_write(serial, boost::asio::buffer(message.data(), message.length()),
                                 [this, self](boost::system::error_code ec, std::size_t/*length*/) {
            if (!ec) {
                std::cout << "Writed data (SerialConnection): " << std::endl;
            } else {
                std::cerr << "Write error: (SerialConnection)" << ec.message() << std::endl;
            }
        });
    }
}

void SerialConnection::asyncRead(std::function<void(const std::string&, const std::string&)> callback)  {
    read_callback = callback;
    auto self(shared_from_this());
    serial.async_read_some(boost::asio::buffer(read_buffer),
                           std::bind(&SerialConnection::handleRead, this,
                                     std::placeholders::_1, std::placeholders::_2));
}

bool SerialConnection::isConnected() const {
    auto self(shared_from_this());
    return serial.is_open();
}

std::string SerialConnection::description() const
{
    auto self(shared_from_this());
    boost::asio::serial_port_base::baud_rate baudrate;
    serial.get_option(baudrate);
    return port_name + ":" + std::to_string(baudrate.value());
}

void SerialConnection::handleRead(const boost::system::error_code& error, std::size_t bytes_transferred) {
    if (!error) {
        // Create a std::string from the read buffer
        std::string message(read_buffer.data(), bytes_transferred);
        if (read_callback) {
            read_callback(message, "all"); // Call the user-defined callback
        }
        std::cout << " read (SerialConnection): " << std::endl;
        // Continue reading asynchronously
        asyncRead(read_callback);
    } else {
        std::cerr << "Error during read (SerialConnection): " << error.message() << std::endl;
        serial.close(); // Close the serial port on error
    }
}
