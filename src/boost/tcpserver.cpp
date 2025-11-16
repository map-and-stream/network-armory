#include "tcpserver.h"
#include <iostream>

Server::Server(boost::asio::io_context& io_context, int port)
    : acceptor_(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
      deviceManager_(nullptr)
{
    acceptConnections();
}

Server::~Server() = default;

void Server::acceptConnections() {
    acceptor_.async_accept([this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
        if (!ec) {
            auto connection = std::make_shared<TCPConnection>(std::move(socket));
            tcp_connections_.push_back(connection);
            std::vector<std::string> topics;
            topics.push_back("all");
            std::cout << "Connected to server (Server): " << connection->description() << std::endl;
            acceptConnections(); // Continue to accept new connections
        } else {
            std::cerr << "Accept error (Server): " << ec.message() << std::endl; // Handle accept error
        }
    });
}
