#pragma once

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <string>
#include <thread>
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

    // Bind to port and start the server loop in a background thread
    Error listen() override;

    // Send data to client by fd
    Error send(int fd, const std::vector<uint8_t>& data) override;

    // Send data to client by IP (linear search over clients_)
    Error send(const std::string& ip, const std::vector<uint8_t>& data) override;

    // Stop server, close sockets, join worker thread
    Error gracefull_shutdown() override;

  private:
    void accept_new_client();
    void handle_client_io(fd_set& readfds);
    void run();  // main event loop (private)

  private:
    int server_fd_ = -1;
    std::vector<ClientInfo> clients_;

    std::thread worker_;
    std::atomic<bool> stop_{false};
};
