#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "error.h"

enum class ConnectionType { TCP, UDP, Serial };
enum class BackendType { ASIO, POSIX };
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
    BackendType backend_type = BackendType::ASIO;
    WorkingMode working_mode = WorkingMode::ASYNC;
    ConnectionType connection_type = ConnectionType::TCP;
    struct AutoConnect {
        int retryTime_ms;  // based on milliseconds
        int retry_count;   // -1 means unlimited
    } auto_connect;
    bool keep_alive;
};

class ClientInterface {
  public:
    using ReceiveCallback = std::function<void(const std::vector<uint8_t>& data)>;

    ClientInterface(NetworkConfig cfg) : cfg_(cfg) {}
    ClientInterface() = delete;
    virtual ~ClientInterface() = default;

    virtual Error connect() {
        Error err;
        err.set_code(ErrorCode::NOT_IMPLEMENTED);
        return err;
    }
    virtual Error connect_async(std::function<void(Error)> callback) = 0;

    virtual Error send_sync(const std::vector<uint8_t>& data) {
        Error err;
        err.set_code(ErrorCode::NOT_IMPLEMENTED);
        return err;
    }

    virtual Error send_async(const std::vector<uint8_t>& data,
                             std::function<void(Error)> callback) = 0;

    virtual Error recieve_sync(std::vector<uint8_t>& recieve_data) {
        Error err;
        err.set_code(ErrorCode::NOT_IMPLEMENTED);
        return err;
    }

    virtual Error recieve_async(
        std::function<void(const std::vector<uint8_t>&, Error)> callback) = 0;

    bool is_connected() const { return is_connected_; };

    std::string description() const { return cfg_.ip + ":" + std::to_string(cfg_.port); }

  protected:
    NetworkConfig cfg_;
    bool is_connected_ = false;
};

template <class T>
ClientInterface* create(NetworkConfig cfg) {
    return new T(cfg);
}
// INetwork* net = create<BOOST_TCP>(cfg);
