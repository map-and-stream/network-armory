#ifndef BOOST_UDP_CONNECTION_H
#define BOOST_UDP_CONNECTION_H

#include <boost/asio.hpp>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "network.h"

class UDPConnection : public INetwork, public std::enable_shared_from_this<UDPConnection> {
  public:
    using Ptr = std::shared_ptr<UDPConnection>;

    static Ptr create(boost::asio::io_context& io_context, const std::string& host,
                      unsigned short port);

    UDPConnection(boost::asio::io_context& io_context, const std::string& host,
                  unsigned short port);

    ~UDPConnection() override;

    void send(const std::vector<uint8_t>& data) override;

    void asyncRead(ReceiveCallback cb) override;

    bool isConnected() const override;

    ConnectionType type() const override { return ConnectionType::UDP; }

    BackendType backend() const override { return BackendType::BoostAsio; }

    std::string description() const override;

  private:
    void connect(const std::string& host, unsigned short port);

    // Optionally, you could define a handleRead like in TCPConnection
    void handleRead(const boost::system::error_code& error, std::size_t bytes_transferred);

    boost::asio::io_context& io_context_;
    boost::asio::ip::udp::socket socket_;
    boost::asio::ip::udp::endpoint remote_endpoint_;
    std::vector<uint8_t> read_buffer_;
    ReceiveCallback read_callback_;
};

#endif  // BOOST_UDP_CONNECTION_H
