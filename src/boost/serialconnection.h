#ifndef BOOST_SERIAL_CONNECTION_H
#define BOOST_SERIAL_CONNECTION_H

#include "network.h"
#include <boost/asio.hpp>
#include <functional>
#include <memory>
#include <string>
#include <vector>

class SerialConnection : public INetwork, public std::enable_shared_from_this<SerialConnection> {
public:
    using Ptr = std::shared_ptr<SerialConnection>;

    // Factory method:
    static Ptr create(boost::asio::io_context& io_context, const std::string& port, uint32_t baud);

    SerialConnection(boost::asio::io_context& io_context, const std::string& port, uint32_t baud);

    ~SerialConnection() override;

    void send(const std::vector<uint8_t>& data) override;

    void asyncRead(ReceiveCallback cb) override;

    bool isConnected() const override;

    ConnectionType type() const override { return ConnectionType::Serial; }

    BackendType backend() const override { return BackendType::BoostAsio; }

    std::string description() const override;

private:
    void open(const std::string& port, uint32_t baud);

    void handleRead(const boost::system::error_code& error, std::size_t bytes_transferred);

    boost::asio::serial_port serial_;
    std::vector<uint8_t> read_buffer_;
    ReceiveCallback read_callback_;
    std::string port_name_;
    uint32_t baud_rate_;
};

#endif // BOOST_SERIAL_CONNECTION_H
