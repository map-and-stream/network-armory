#pragma once
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <functional>
#include <string>
#include <unordered_map>

class UdpServer {
  public:
    using Callback = std::function<std::string(int client_id, const std::string&)>;

    UdpServer(int port, Callback cb);

    bool start();
    void run();
    void stop();

  private:
    int get_or_assign_client_id(const sockaddr_in& client);

    int port_;
    int sockfd_;
    bool running_ = false;
    Callback callback_;

    std::unordered_map<uint64_t, int> client_map_;
    int next_client_id_ = 1;
};
