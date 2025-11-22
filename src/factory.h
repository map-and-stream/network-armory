#pragma once

#include <string.h>

#include <memory>

#include "network.h"

class NetworkFactory {
  public:
    static NetworkFactory* createNetwork(NetworkConfig type, LogConfig cfg) {}
    virtual ~NetworkFactory() = default;

    virtual std::shared_ptr<INetwork> createTCPClient(const std::string& host, uint16_t port) = 0;

    virtual std::shared_ptr<INetwork> createTCPServer(uint16_t port) = 0;

    virtual std::shared_ptr<INetwork> createUDP(const std::string& host, uint16_t port) = 0;

    virtual std::shared_ptr<INetwork> createSerial(const std::string& port, uint32_t baud) = 0;
};