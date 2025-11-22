#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "error.h"

enum class ConnectionType { TCP, UDP, Serial };
enum class BackendType { BoostAsio, StdNet };
enum class WorkingMode {
    SYNC,
    ASYNC
};  // check in implementation of boost asio  if in connect specify sync async then updae using
    // working mode or not
struct NetworkConfig {
    std::string ip;
    int port;
    struct SSLConfig {
        std::string public_key;
    } ssl_config;
    WorkingMode mode;
    struct AutoConnect {
        int retryTime_ms;  // based on milliseconds
        int retry_count;   // -1 means unlimited
    } auto_connect;
    bool keep_alive;
};

class INetwork {
  public:
    using ReceiveCallback = std::function<void(const std::vector<uint8_t>& data)>;

    INetwork(NetworkConfig cfg) : cfg_(cfg) {}
    INetwork() = delete;
    virtual ~INetwork() = default;

    virtual Error connect() = 0;

    virtual Error send_sync(const std::vector<uint8_t>& data) = 0;

    virtual Error send_async(const std::vector<uint8_t>& data) = 0;

    virtual Error recieve_sync(std::vector<uint8_t>& recieve_data) = 0;

    virtual Error recieve_async(ReceiveCallback cb) = 0;

    bool is_connected() const { return is_connected_; };

    std::string description() const { return cfg_.ip + ":" + std::to_string(cfg_.port); }

  protected:
    NetworkConfig cfg_;
    bool is_connected_ = false;
};

template <class T>
INetwork* create(NetworkConfig cfg) {
    return new T(cfg);
}
// INetwork* net = create<BOOST_TCP>(cfg);
