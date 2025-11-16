#pragma once

#include "network.h"
#pragma once

#include "factory.h"

class StdNetwork : public NetworkFactory {
  public:
    std::shared_ptr<INetwork> createTCPClient(const std::string& host, uint16_t port) override{}

    std::shared_ptr<INetwork> createTCPServer(uint16_t port) override{}

    std::shared_ptr<INetwork> createUDP(const std::string& host, uint16_t port) override{}

    std::shared_ptr<INetwork> createSerial(const std::string&, uint32_t) override {
        throw std::runtime_error("Serial not supported in std::net");
    }
};