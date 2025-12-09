#include "udp_server.h"

#include <iostream>

UdpServer::UdpServer(int port) : port_(port) {}

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

void UdpServer::run() {
    char buffer[1024];
    sockaddr_in client{};
    socklen_t len = sizeof(client);

    while (running_) {
        int n = recvfrom(sockfd_, buffer, sizeof(buffer), 0, (sockaddr*)&client, &len);
        if (n <= 0)
            continue;

        std::string msg(buffer, n);
        std::cout << "Received: " << msg << std::endl;

        std::string reply = "Echo: " + msg;
        sendto(sockfd_, reply.c_str(), reply.size(), 0, (sockaddr*)&client, len);
    }
}

void UdpServer::stop() {
    running_ = false;
    close(sockfd_);
}
