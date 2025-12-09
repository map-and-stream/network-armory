#include "udp_server.h"

UdpServer::UdpServer(int port, Callback cb) : port_(port), callback_(cb) {}

bool UdpServer::start() {
    sockfd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd_ < 0)
        return false;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port_);

    if (bind(sockfd_, (sockaddr*)&addr, sizeof(addr)) < 0)
        return false;

    running_ = true;
    return true;
}

int UdpServer::get_or_assign_client_id(const sockaddr_in& client) {
    uint64_t key = (uint64_t(client.sin_addr.s_addr) << 16) | client.sin_port;

    if (client_map_.count(key) == 0) {
        client_map_[key] = next_client_id_++;
    }
    return client_map_[key];
}

void UdpServer::run() {
    char buffer[1024];
    sockaddr_in client{};
    socklen_t len = sizeof(client);

    while (running_) {
        int n = recvfrom(sockfd_, buffer, sizeof(buffer), 0, (sockaddr*)&client, &len);
        if (n <= 0)
            continue;

        std::string req(buffer, n);

        int client_id = get_or_assign_client_id(client);

        // Only callback output
        std::string reply = callback_(client_id, req);

        sendto(sockfd_, reply.c_str(), reply.size(), 0, (sockaddr*)&client, len);
    }
}

void UdpServer::stop() {
    running_ = false;
    close(sockfd_);
}
