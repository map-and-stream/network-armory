#include <arpa/inet.h>
#include <gtest/gtest.h>
#include <sys/socket.h>
#include <unistd.h>

#include <thread>

TEST(TcpTest, ClientServerCommunication) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(5000);

    bind(server_fd, (sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 1);

    std::thread server_thread([&]() {
        int client = accept(server_fd, nullptr, nullptr);
        char buffer[100];
        read(client, buffer, sizeof(buffer));
        std::string msg(buffer);
        EXPECT_EQ(msg, "hello");
        close(client);
    });

    // Client
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    connect(client_fd, (sockaddr*)&addr, sizeof(addr));
    send(client_fd, "hello", 5, 0);

    close(client_fd);
    server_thread.join();
    close(server_fd);
}
