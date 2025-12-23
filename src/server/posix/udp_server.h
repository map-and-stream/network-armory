#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <functional>
#include <string>
#include <thread>
#include <unordered_map>

#include "error.h"

class UdpServer {
  public:
    using Callback = std::function<std::string(int, const std::string&)>;

    UdpServer(int port, Callback cb);

    Error start();  // starts worker thread
    void stop();    // stops server and joins thread

    void run();  // internal loop (still public but not needed externally)
    void send_async(int fd, const std::string& data, std::function<void()> callback);

    int get_or_assign_client_id(const sockaddr_in& client);

  private:
    int port_;
    int sockfd_ = -1;
    std::atomic<bool> running_{false};
    Callback callback_;

    std::unordered_map<uint64_t, int> client_map_;
    int next_client_id_ = 1;

    std::thread worker_;
};
