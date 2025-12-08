#include "tcp_server.h"

#include <algorithm>
TcpServer::TcpServer(int port, Callback cb) : port_(port), callback_(cb) {}

bool TcpServer::start() {
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0)
        return false;

    // Make server socket nonblocking
    fcntl(server_fd_, F_SETFL, O_NONBLOCK);

    int opt = 1;
    setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port_);

    if (::bind(server_fd_, (sockaddr*)&addr, sizeof(addr)) < 0)
        return false;

    if (::listen(server_fd_, SOMAXCONN) < 0)
        return false;

    running_ = true;
    return true;
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
        tv.tv_usec = 200000;  // 200 ms tick

        int activity = select(max_fd + 1, &readfds, nullptr, nullptr, &tv);
        if (activity < 0)
            continue;

        // New connection?
        if (FD_ISSET(server_fd_, &readfds)) {
            accept_new_client();
        }

        // Existing clients
        handle_client_io(readfds);
    }
}

void TcpServer::stop() {
    running_ = false;
    close(server_fd_);
    for (int cfd : clients_) close(cfd);
}

void TcpServer::accept_new_client() {
    int client_fd = accept(server_fd_, nullptr, nullptr);
    if (client_fd < 0)
        return;
    fcntl(client_fd, F_SETFL, O_NONBLOCK);
    clients_.push_back(client_fd);
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

            std::string request(buffer, bytes);
            std::string response = callback_(cfd, request);

            send_sync(cfd, response);
        }
    }

    // Remove closed clients
    for (int cfd : to_remove) {
        close(cfd);
        clients_.erase(std::remove(clients_.begin(), clients_.end(), cfd), clients_.end());
    }
}

void TcpServer::send_sync(int fd, const std::string& data) {
    const char* buf = data.c_str();
    size_t total = 0;
    size_t len = data.size();

    while (total < len) {
        ssize_t sent = send(fd, buf + total, len - total, 0);
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