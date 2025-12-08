#pragma once
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <string>

class UdpServer {
  public:
    UdpServer(int port);
    bool start();
    void run();
    void stop();

  private:
    int port_;
    int sockfd_;
    bool running_ = false;
};
