// UDPConnection.h
#ifndef UDPCONNECTION_H
#define UDPCONNECTION_H

#include "connection.h"
#include <boost/asio.hpp>
#include <iostream>
#include <functional>

class UDPConnection : public std::enable_shared_from_this<UDPConnection> , public Connection {
public:
    using Ptr = std::shared_ptr<UDPConnection>;
    static Ptr create(boost::asio::io_context& io_context, const std::string& host, unsigned short port) {
        return Ptr(new UDPConnection(io_context, host, port));
    }

    UDPConnection(boost::asio::io_context& io_context, const std::string& host, int port);
    ~UDPConnection() override;
    void send(const std::string& message, const std::string& topic = "all") override;
    void asyncRead(std::function<void(const std::string&, const std::string&)> callback) override; // New: async receive method
    bool isConnected() const override;
    DeviceType type() const override{ return DeviceType::udp;}
    std::string description() const override;

private:
    void handleRead(const boost::system::error_code& error, std::size_t bytes_transferred);
    boost::asio::ip::udp::endpoint remote_endpoint_;
    boost::asio::ip::udp::socket socket_;
};

#endif // UDPCONNECTION_H
