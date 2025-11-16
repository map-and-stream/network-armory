// SerialConnection.h
#ifndef SERIALCONNECTION_H
#define SERIALCONNECTION_H

#include "connection.h"
#include <boost/asio.hpp>
#include <iostream>
#include <functional>

class SerialConnection : public std::enable_shared_from_this<SerialConnection> , public Connection {
public:
    using Ptr = std::shared_ptr<SerialConnection>;
    static Ptr create(boost::asio::io_context& io_context, const std::string& portname, unsigned int baud_rate) {
        return Ptr(new SerialConnection(io_context, portname, baud_rate));
    }
    SerialConnection(boost::asio::io_context& io_context, const std::string& portname, unsigned int baud_rate);
    ~SerialConnection() override;
    void send(const std::string& message, const std::string& topic = "all") override;
    void asyncRead(std::function<void(const std::string&, const std::string&)> callback) override; // New: async read method
    bool isConnected() const override;
    DeviceType type() const override { return DeviceType::serial;}
    std::string description() const override ;

private:
    void handleRead(const boost::system::error_code& error, std::size_t bytes_transferred);
    boost::asio::serial_port serial;
    std::string port_name;
};

#endif // SERIALCONNECTION_H
