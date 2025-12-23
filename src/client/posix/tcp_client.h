#pragma once

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/tcp.h>  // For TCP_KEEPIDLE, TCP_KEEPINTVL, TCP_KEEPCNT
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <functional>
#include <mutex>
#include <string>
#include <thread>

#include "client/client_interface.h"
#include "error.h"

class TcpClientPosix : public ClientInterface {
  public:
    TcpClientPosix(const NetworkConfig& cfg);

    Error connect() override;
    Error connect_async(std::function<void(Error)> callback) override;

    Error send_sync(const std::vector<uint8_t>& data) override;
    Error send_async(const std::vector<uint8_t>& data,
                     std::function<void(Error)> callback) override;

    Error recieve_sync(std::vector<uint8_t>& out) override;
    Error recieve_async(ReceiveCallback callback) override;

    Error disconnect() override;

  private:
    bool sendMessage(const std::vector<uint8_t>& data);
    bool internal_connect(bool isBlocking);
    void stop();
    void setDelimiter(char d) { delimiter = d; }
    void setReconnectDelay(int ms) { reconnectDelayMs = ms; }
    void reconnect() {
        close(sock);
        sock = -1;
        std::this_thread::sleep_for(std::chrono::milliseconds(reconnectDelayMs));
    }
    void set_keep_alive_options(int idle = 30, int interval = 10, int count = 3);

  private:
    std::string serverIP;
    int serverPort;
    int sock;
    std::atomic<bool> running;
    std::thread recvThread;
    std::string recvBuffer;
    int reconnectDelayMs;
    char delimiter;

    std::mutex sockMutex;
};