#pragma once

#include <boost/asio.hpp>
#include <memory>

#include "factory.h"

class BoostNetwork : public NetworkFactory {
  public:
    BoostNetwork(boost::asio::io_context& ctx);

    std::shared_ptr<INetwork> createTCPClient(const std::string& host, uint16_t port) override {}

    std::shared_ptr<INetwork> createTCPServer(uint16_t port) override {}

    std::shared_ptr<INetwork> createUDP(const std::string& host, uint16_t port) override {}

    std::shared_ptr<INetwork> createSerial(const std::string& port, uint32_t baud) override {}

  private:
    boost::asio::io_context& io_;
};
