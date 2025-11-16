#ifndef BOOST_TCP_CONNECTION_H
#define BOOST_TCP_CONNECTION_H

#include "network.h"
#include <boost/asio.hpp>
#include <functional>
#include <memory>
#include <string>
#include <vector>

class TCPConnection : public INetwork, public std::enable_shared_from_this<TCPConnection> {
public:
    using Ptr = std::shared_ptr<TCPConnection>;

    static Ptr create(boost::asio::io_context& io_context, const std::string& host, unsigned short port);

    explicit TCPConnection(boost::asio::ip::tcp::socket socket);

    ~TCPConnection() override;

    void send(const std::vector<uint8_t>& data) override;

    void asyncRead(ReceiveCallback cb) override;

    bool isConnected() const override;

    ConnectionType type() const override { return ConnectionType::TCP; }

    BackendType backend() const override { return BackendType::BoostAsio; }

    std::string description() const override;

private:
    void connect(const std::string& host, unsigned short port);

    void handleRead(const boost::system::error_code& error, std::size_t bytes_transferred);

    boost::asio::ip::tcp::socket socket_;
    std::vector<uint8_t> read_buffer_;
    ReceiveCallback read_callback_;
};

#endif
