#pragma once
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <functional>
#include <string>
#include <unordered_map>

#include "client/error.h"

class UdpServer {
  public:
    using Callback = std::function<std::string(int, const std::string&)>;

    UdpServer(int port, Callback cb);

    void run();
    void stop();

    void send_async(int fd, const std::string& data, std::function<void()> callback);
    int get_or_assign_client_id(const sockaddr_in& client);
    Error start();

  private:
    int port_;
    int sockfd_;
    bool running_ = false;
    Callback callback_;

    std::unordered_map<uint64_t, int> client_map_;
    int next_client_id_ = 1;
};
