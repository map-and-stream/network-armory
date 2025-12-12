#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "error.h"

enum class ServerType { TCP, UDP };
struct ServerConfig {
    int port;
    struct SSLConfig {
        std::string public_key;
    } ssl_config;
    enum class BackendType { ASIO, POSIX } backend_type = BackendType::POSIX;
    ServerType connection_type = ServerType::TCP;
};

class ServerInterface {
  public:
    using ReceiveCallback = std::function<void(int fd, const std::vector<uint8_t>&)>;
    using ClientConnectCallback = std::function<void(int fd, const std::string& ip)>;
    using ClientDisconnectCallback = std::function<void(int fd, const std::string& ip)>;

    ServerInterface(ServerConfig cfg, ReceiveCallback recieveCallback,
                    ClientConnectCallback clientConnectionCallback,
                    ClientDisconnectCallback clientDisconnectCallback)
        : cfg_(cfg),
          recieveCallback_(recieveCallback),
          clientConnectionCallback_(clientConnectionCallback),
          clientDisconnectCallback_(clientDisconnectCallback) {}
    ServerInterface() = delete;
    virtual ~ServerInterface() = default;

    virtual Error listen() = 0;

    virtual Error send(int fd, const std::vector<uint8_t>& data) = 0;

    virtual Error gracefull_shutdown() = 0;

  protected:
    ServerConfig cfg_;
    ReceiveCallback recieveCallback_;
    ClientConnectCallback clientConnectionCallback_;
    ClientDisconnectCallback clientDisconnectCallback_;
    bool running_ = false;
};
