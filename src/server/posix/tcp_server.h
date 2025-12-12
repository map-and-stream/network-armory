#pragma once

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include "error.h"
#include "server/server_interface.h"

class TcpServer : public ServerInterface {
    struct ClientInfo {
      public:
        int fd;
        std::string ip;
    };

  public:
    TcpServer(ServerConfig cfg, ReceiveCallback recieveCallback,
              ClientConnectCallback clientCallback,
              ClientDisconnectCallback clientDisconnectCallback)
        : ServerInterface(cfg, recieveCallback, clientCallback, clientDisconnectCallback) {}

    Error listen() override;  // bind to port and listen with error reporting

    Error send(int fd, const std::vector<uint8_t>& data) override;

    Error gracefull_shutdown() override;

  private:
    void accept_new_client();
    void handle_client_io(fd_set& readfds);

  private:
    void run();
    int server_fd_;
    std::vector<ClientInfo> clients_;
};
