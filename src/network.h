#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

enum class ConnectionType { TCP, UDP, Serial };
enum class BackendType { BoostAsio, StdNet };

class INetwork {
  public:
    using ReceiveCallback = std::function<void(const std::vector<uint8_t>& data)>;
    virtual ~INetwork() = default;

    virtual void send(const std::vector<uint8_t>& data) = 0;

    virtual void asyncRead(ReceiveCallback cb) = 0;

    virtual bool isConnected() const = 0;

    virtual ConnectionType type() const = 0;
    virtual BackendType backend() const = 0;

    virtual std::string description() const = 0;
};
