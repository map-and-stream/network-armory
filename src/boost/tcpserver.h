// TCPServer.h
#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <boost/asio.hpp>
#include <thread>
#include <vector>
#include "tcpconnection.h"
#include <boost/bind/bind.hpp>

class DeviceManager;

class Server  {
public:
    Server(boost::asio::io_context& io_context, int port);
private:
    void acceptConnections();
    boost::asio::ip::tcp::acceptor acceptor; // TCP acceptor
    std::vector<std::shared_ptr<TCPConnection>> tcp_connections_;
    DeviceManager *deviceManager;
};

#endif // TCPSERVER_H
