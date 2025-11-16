#include "boost_network.h"

#include "serialconnection.h"
#include "tcpconnection.h"
#include "tcpserver.h"
#include "udpconnection.h"

BoostNetwork::BoostNetwork(boost::asio::io_context& ctx) : io_(ctx) {}

BoostNetwork::~BoostNetwork() = default;

std::shared_ptr<INetwork> BoostNetwork::createTCPClient(const std::string& host, uint16_t port) {
    // Use TCPConnection::create factory for proper async connect handling
    return TCPConnection::create(io_, host, port);
}

std::shared_ptr<INetwork> BoostNetwork::createTCPServer(uint16_t port) {
    // Server does not derive from INetwork, so we cannot return it here directly
    // Returning nullptr or throw, or implement a wrapper if necessary
    // For now, return nullptr to avoid invalid conversion
    return nullptr;
}

std::shared_ptr<INetwork> BoostNetwork::createUDP(const std::string& host, uint16_t port) {
    // Use UDPConnection::create factory
    return UDPConnection::create(io_, host, port);
}

std::shared_ptr<INetwork> BoostNetwork::createSerial(const std::string& port, uint32_t baud) {
    // Use SerialConnection::create factory method
    return SerialConnection::create(io_, port, baud);
}

SerialConnection::Ptr SerialConnection::create(boost::asio::io_context& io, const std::string& port,
                                               unsigned int baud) {
    return std::make_shared<SerialConnection>(io, port, baud);
}
