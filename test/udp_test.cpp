#include <arpa/inet.h>
#include <gtest/gtest.h>
#include <sys/socket.h>
#include <unistd.h>

#include <thread>

TEST(UdpTest, SendReceive) {
    int server_fd = socket(AF_INET, SOCK_DGRAM, 0);
    int client_fd = socket(AF_INET, SOCK_DGRAM, 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(6000);

    bind(server_fd, (sockaddr*)&addr, sizeof(addr));

    std::thread server_thread([&]() {
        char buffer[100] = {0};  // **clear buffer**
        sockaddr_in client_addr{};
        socklen_t len = sizeof(client_addr);

        int received =
            recvfrom(server_fd, buffer, sizeof(buffer) - 1, 0, (sockaddr*)&client_addr, &len);

        buffer[received] = '\0';  // **null‑terminate**

        EXPECT_STREQ(buffer, "ping");
    });

    sendto(client_fd, "ping", strlen("ping"), 0, (sockaddr*)&addr, sizeof(addr));

    server_thread.join();
    close(server_fd);
    close(client_fd);
}
