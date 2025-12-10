#include "udp_server.h"

UdpServer::UdpServer(int port, Callback cb) : port_(port), callback_(cb) {}

bool UdpServer::start() {
    sockfd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd_ < 0)
        return false;

    // SET A TIMEOUT FOR recvfrom SO SERVER CAN EXIT
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100000;  // 100 ms
    setsockopt(sockfd_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

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

        if (!running_)
            break;  // after timeout, exit cleanly

        if (n < 0) {
            // timeout error (EAGAIN/EWOULDBLOCK)
            continue;
        }

        std::string req(buffer, n);
        int client_id = get_or_assign_client_id(client);

        std::string reply = callback_(client_id, req);

        sendto(sockfd_, reply.c_str(), reply.size(), 0, (sockaddr*)&client, len);
    }
}

void UdpServer::send_async(int fd, const std::string& data, std::function<void()> callback) {
    // Since UDP is connectionless, 'fd' is ignored; there's no fixed client socket.
    // For real async, you'd use threads or non-blocking IO/event loops.
    // This implementation just sends on the main socket and immediately calls callback.

    // We'll assume 'fd' is actually the client identifier, and we need to map back to their
    // sockaddr_in. Find the client sockaddr_in by reverse lookup.
    sockaddr_in target{};
    bool found = false;
    for (const auto& kv : client_map_) {
        if (kv.second == fd) {
            // Recompose key to sockaddr_in
            uint64_t key = kv.first;
            in_addr_t addr_part = key >> 16;
            in_port_t port_part = key & 0xFFFF;

            target.sin_family = AF_INET;
            target.sin_addr.s_addr = addr_part;
            target.sin_port = port_part;
            found = true;
            break;
        }
    }
    if (!found) {
        if (callback)
            callback();
        return;
    }

    sendto(sockfd_, data.c_str(), data.size(), 0, (sockaddr*)&target, sizeof(target));

    if (callback)
        callback();
}

void UdpServer::stop() {
    running_ = false;
    close(sockfd_);
}
