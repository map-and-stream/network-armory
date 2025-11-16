#ifndef BOOST_TCP_SERVER_H
#define BOOST_TCP_SERVER_H

#include "network.h"
#include <boost/asio.hpp>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include "tcpconnection.h"

class DeviceManager;

class Server : public std::enable_shared_from_this<Server> {
public:
    using Ptr = std::shared_ptr<Server>;

    Server(boost::asio::io_context& io_context, int port);

    ~Server();

    // Optionally expose something like getConnections(), if needed
    // const std::vector<std::shared_ptr<TCPConnection>>& connections() const;

private:
    void acceptConnections();

    boost::asio::ip::tcp::acceptor acceptor_;
    std::vector<std::shared_ptr<TCPConnection>> tcp_connections_;
    DeviceManager* deviceManager_;
};

#endif // BOOST_TCP_SERVER_H
