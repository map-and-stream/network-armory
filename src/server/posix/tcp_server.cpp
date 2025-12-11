#include "tcp_server.h"

#include <algorithm>
#include <cstring>
#include "error.h"

Error TcpServer::listen() {
    // Clear old clients on restart
    for (int cfd : clients_) close(cfd);
    clients_.clear();

    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0) {
        Error err;
        err.set_code(ErrorCode::CONNECTION_FAILED)->set_message("Failed to create TCP socket");
        return err;
    }

    fcntl(server_fd_, F_SETFL, O_NONBLOCK);

    int opt = 1;
    setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(cfg_.port);

    if (::bind(server_fd_, (sockaddr*)&addr, sizeof(addr)) < 0) {
        Error err;
        if (errno == EADDRINUSE) {
            err.set_code(ErrorCode::PORT_IN_USE)->set_message("Port is already in use");
        } else {
            err.set_code(ErrorCode::CONNECTION_FAILED)->set_message("Bind failed");
        }
        return err;
    }

    if (::listen(server_fd_, SOMAXCONN) < 0) {
        Error err;
        err.set_code(ErrorCode::CONNECTION_FAILED)->set_message("Listen failed");
        return err;
    }

    running_ = true;
    run();
    return Error{};
}

void TcpServer::run() {
    while (running_) {
        fd_set readfds;
        FD_ZERO(&readfds);

        FD_SET(server_fd_, &readfds);
        int max_fd = server_fd_;

        for (int cfd : clients_) {
            FD_SET(cfd, &readfds);
            if (cfd > max_fd)
                max_fd = cfd;
        }

        timeval tv{};
        tv.tv_sec = 0;
        tv.tv_usec = 200000;  // 200 ms

        int activity = select(max_fd + 1, &readfds, nullptr, nullptr, &tv);
        if (activity < 0)
            continue;

        if (FD_ISSET(server_fd_, &readfds)) {
            accept_new_client();
        }

        handle_client_io(readfds);
    }
}

Error TcpServer::gracefull_shutdown() {
    running_ = false;
    if (server_fd_ >= 0)
        close(server_fd_);
    for (int cfd : clients_)
        close(cfd);
    clients_.clear();
}

void TcpServer::accept_new_client() {
    while (true) {
        int client_fd = accept(server_fd_, nullptr, nullptr);
        if (client_fd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // No more clients to accept
                return;
            }
            std::cerr << "[SERVER] accept() failed: " << strerror(errno) << std::endl;
            return;
        }

        fcntl(client_fd, F_SETFL, O_NONBLOCK);
        clients_.push_back(client_fd);
        std::cout << "[SERVER] New client accepted: fd=" << client_fd << std::endl;
    }
}

void TcpServer::handle_client_io(fd_set& readfds) {
    std::vector<int> to_remove;

    for (int cfd : clients_) {
        if (FD_ISSET(cfd, &readfds)) {
            char buffer[1024];
            int bytes = recv(cfd, buffer, sizeof(buffer), 0);

            if (bytes <= 0) {
                to_remove.push_back(cfd);
                continue;
            }

            recieveCallback_(cfd, std::vector<uint8_t>(buffer, buffer + bytes));

            // send(cfd, response);
        }
    }

    for (int cfd : to_remove) {
        close(cfd);
        clients_.erase(std::remove(clients_.begin(), clients_.end(), cfd), clients_.end());
    }
}

Error TcpServer::send(int fd, const std::vector<uint8_t>& data) {
    const uint8_t* buf = data.data();
    size_t total = 0;
    size_t len = data.size();

    while (total < len) {
        ssize_t sent = ::send(fd, buf + total, len - total, 0);
        if (sent <= 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                usleep(1000);
                continue;
            }
            break;
        }
        total += sent;
    }
}
