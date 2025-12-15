#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "error.h"

enum class ClientType { TCP, UDP, Serial };

struct NetworkConfig {
    std::string ip;
    int port;

    struct SSLConfig {
        std::string public_key;
    } ssl_config;

    enum class BackendType { ASIO, POSIX } backend_type = BackendType::ASIO;

    ClientType connection_type = ClientType::TCP;

    struct AutoConnect {
        int retryTime_ms = 2000;  // milliseconds
        int retry_count = -1;     // -1 means unlimited
    } auto_connect;

    bool keep_alive = true;
};

class ClientInterface {
  public:
    using ReceiveCallback = std::function<void(const std::vector<uint8_t>&, Error)>;
    using AsyncCallback   = std::function<void(Error)>;

    explicit ClientInterface(NetworkConfig cfg) : cfg_(std::move(cfg)) {}
    ClientInterface() = delete;
    virtual ~ClientInterface() = default;

    virtual Error connect() {
        Error err;
        err.set_code(ErrorCode::NOT_IMPLEMENTED);
        return err;
    }

    virtual Error connect_async(AsyncCallback callback) = 0;

    virtual Error disconnect() = 0;

    virtual Error send_sync(const std::vector<uint8_t>& data) {
        Error err;
        err.set_code(ErrorCode::NOT_IMPLEMENTED);
        return err;
    }

    virtual Error send_async(const std::vector<uint8_t>& data, AsyncCallback callback) = 0;

    virtual Error recieve_sync(std::vector<uint8_t>& recieve_data) {
        Error err;
        err.set_code(ErrorCode::NOT_IMPLEMENTED);
        return err;
    }

    virtual Error recieve_async(ReceiveCallback callback) = 0;

    bool is_connected() const { return is_connected_; }

    std::string description() const { return cfg_.ip + ":" + std::to_string(cfg_.port); }

  protected:
    NetworkConfig cfg_;
    bool is_connected_ = false;
};
