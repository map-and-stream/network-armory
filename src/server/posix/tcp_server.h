#pragma once
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <functional>
#include <string>
#include <vector>

class TcpServer {
  public:
    using Callback = std::function<std::string(int client_id, const std::string&)>;

    TcpServer(int port, Callback cb);

    bool start();  // bind to port and start listen
    void run();
    void stop();

    void send_sync(int fd, const std::string& data);

  private:
    void accept_new_client();
    void handle_client_io(fd_set& readfds);

  private:
    int port_;
    int server_fd_;
    bool running_ = false;
    std::vector<int> clients_;
    Callback callback_;
};
