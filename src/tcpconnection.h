// TCPConnection.h
#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include "connection.h"
#include <boost/asio.hpp>
#include <iostream>
#include <functional>
#include <memory>
#include <array>
#include <vector>


class TCPConnection :  public std::enable_shared_from_this<TCPConnection> , public Connection {
public:
    using Ptr = std::shared_ptr<TCPConnection>;
    static Ptr create(boost::asio::io_context& io_context, const std::string& host, unsigned short port) {
        auto connection = Ptr(new TCPConnection(boost::asio::ip::tcp::socket(io_context)));
        connection->connect(host, port);
        return connection;
    }

    TCPConnection(boost::asio::ip::tcp::socket socket);
    ~TCPConnection() override;
    void send(const std::string& message, const std::string& topic = "all") override;
    void asyncRead(std::function<void(const std::string&, const std::string&)> callback) override; // New: async read method
    bool isConnected() const override;
    DeviceType type() const override{ return DeviceType::tcp;}

    std::string description() const override ;
private:
    void connect(const std::string& host, unsigned short port);
    void handleRead(const boost::system::error_code& error, std::size_t bytes_transferred);
    boost::asio::ip::tcp::socket socket_;
};

#endif // TCPCONNECTION_H
