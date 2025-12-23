#include "udp_server.h"

UdpServer::UdpServer(int port, Callback cb) : port_(port), callback_(cb) {}

Error UdpServer::start() {
    sockfd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd_ < 0) {
        Error err;
        err.set_code(ErrorCode::CONNECTION_FAILED)->set_message("Failed to create UDP socket");
        return err;
    }

    // Set recv timeout
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100000;
    setsockopt(sockfd_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port_);

    if (bind(sockfd_, (sockaddr*)&addr, sizeof(addr)) < 0) {
        Error err;
        if (errno == EADDRINUSE) {
            err.set_code(ErrorCode::PORT_IN_USE)->set_message("Port is already in use");
        } else {
            err.set_code(ErrorCode::CONNECTION_FAILED)
                ->set_message("Failed to bind UDP server socket");
        }
        return err;
    }

    running_ = true;

    // Start server thread
    worker_ = std::thread([this]() { this->run(); });

    Error ok;
    ok.set_code(ErrorCode::NO_ERROR);
    return ok;
}

int UdpServer::get_or_assign_client_id(const sockaddr_in& client) {
    uint64_t key = (uint64_t(client.sin_addr.s_addr) << 16) | client.sin_port;

    if (!client_map_.count(key))
        client_map_[key] = next_client_id_++;

    return client_map_[key];
}

void UdpServer::run() {
    char buffer[1024];
    sockaddr_in client{};
    socklen_t len = sizeof(client);

    while (running_) {
        int n = recvfrom(sockfd_, buffer, sizeof(buffer), 0, (sockaddr*)&client, &len);

        if (!running_)
            break;

        if (n < 0)
            continue;

        std::string req(buffer, n);
        int client_id = get_or_assign_client_id(client);

        std::string reply = callback_(client_id, req);

        sendto(sockfd_, reply.c_str(), reply.size(), 0, (sockaddr*)&client, len);
    }
}

void UdpServer::send_async(int fd, const std::string& data, std::function<void()> callback) {
    sockaddr_in target{};
    bool found = false;

    for (auto& kv : client_map_) {
        if (kv.second == fd) {
            uint64_t key = kv.first;
            target.sin_family = AF_INET;
            target.sin_addr.s_addr = key >> 16;
            target.sin_port = key & 0xFFFF;
            found = true;
            break;
        }
    }

    if (found) {
        sendto(sockfd_, data.c_str(), data.size(), 0, (sockaddr*)&target, sizeof(target));
    }

    if (callback)
        callback();
}

void UdpServer::stop() {
    running_ = false;

    if (sockfd_ >= 0) {
        close(sockfd_);
        sockfd_ = -1;
    }

    if (worker_.joinable())
        worker_.join();
}
