#pragma once

#include <asio.hpp>
#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "error.h"
#include "server/server_interface.h"

// ASIO TCP Server implementation inheriting from ServerInterface
class TcpServerAsio : public ServerInterface {
  public:
    TcpServerAsio(ServerConfig cfg, ReceiveCallback receiveCallback,
                  ClientConnectCallback clientConnectCallback,
                  ClientDisconnectCallback clientDisconnectCallback);

    ~TcpServerAsio() override;

    // Listen and start the server
    Error listen() override;

    // Send data to client by "fd" (Here, fd is actually the internal connection id)
    Error send(int fd, const std::vector<uint8_t>& data) override;

    // Send data to client by IP (send to first matching IP)
    Error send(const std::string& ip, const std::vector<uint8_t>& data) override;

    Error gracefull_shutdown() override;

  private:
    void do_accept();
    void do_read(int conn_id, std::shared_ptr<asio::ip::tcp::socket> sock, std::string client_ip);

  private:
    asio::io_context io_context_;
    asio::ip::tcp::acceptor acceptor_;
    asio::ip::tcp::socket socket_;
    std::thread io_thread_;

    std::atomic<int> next_conn_id_{1};
    std::unordered_map<int, std::shared_ptr<asio::ip::tcp::socket>> connections_;
    std::unordered_map<int, std::string> connections_ip_;
    std::mutex connections_mutex_;
};
