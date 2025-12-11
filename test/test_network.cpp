#include <gtest/gtest.h>

#include <thread>

#include "server/posix/tcp_server.h"
#include "server/posix/udp_server.h"
#include "server/server_interface.h"
#include "tcp_client.h"
#include "udp_client.h"

// ---------------- TCPConnection Tests ----------------

void receive_callback(int fd, const std::vector<uint8_t>& data) {
    std::cout << "Received from clientID[" << fd << "] msg: " << std::string(data.begin(), data.end()) << std::endl;
}

void client_connect_callback(int fd) {
    std::cout << "Client connected: " << fd << std::endl;
}

void client_disconnect_callback(int fd) {
    std::cout << "Client disconnected: " << fd << std::endl;
}

TEST(TcpNetworkApiTest, CanConstructTCPConnection) {
    asio::io_context io;
    NetworkConfig cfg{"127.0.0.1", 12345};
    TcpClientAsio conn(io, cfg);
}

TEST(TcpNetworkApiTest, TCPConnectHandlesBadIp) {
    asio::io_context io;
    NetworkConfig cfg{"bad_ip", 12345};
    TcpClientAsio conn(io, cfg);
    Error err = conn.connect();
    ASSERT_NE(err.code(), ErrorCode::NO_ERROR);
}

TEST(TcpNetworkApiTest, TCPAsyncConnectHandlesBadIp) {
    asio::io_context io;
    NetworkConfig cfg{"bad_ip", 12345};
    TcpClientAsio conn(io, cfg);

    std::atomic<bool> done{false};
    Error result;

    conn.connect_async([&](Error e) {
        result = e;
        done = true;
    });

    io.run();
    ASSERT_NE(result.code(), ErrorCode::NO_ERROR);
}

// ---------------- UDPupdate Tests ----------------

TEST(UdpNetworkApiTest, CanConstructUDPupdate) {
    asio::io_context io;
    NetworkConfig cfg{"127.0.0.1", 9999};
    UdpClient udp(io, cfg);
}

TEST(UdpNetworkApiTest, UDPOpenHandlesBadIp) {
    asio::io_context io;
    NetworkConfig cfg{"bad_ip", 33333};
    UdpClient udp(io, cfg);
    Error err = udp.connect();
    ASSERT_NE(err.code(), ErrorCode::NO_ERROR);
}

// ---------------- UdpServer Tests ----------------

TEST(UdpServerApiTest, CanConstructAndStartUdpServer) {
    int test_port = 56123;

    UdpServer server(test_port, [&](int, const std::string&) { return "pong"; });

    Error err = server.start();
    ASSERT_EQ(err.code(), ErrorCode::NO_ERROR);
    server.stop();
}

TEST(UdpServerApiTest, CreatesUniqueIdsPerClient) {
    UdpServer server(0, [&](int, const std::string&) { return "pong"; });

    sockaddr_in c1{}, c2{};
    c1.sin_family = AF_INET;
    c1.sin_addr.s_addr = htonl(0x7F000001);
    c1.sin_port = htons(1111);
    c2.sin_family = AF_INET;
    c2.sin_addr.s_addr = htonl(0x7F000001);
    c2.sin_port = htons(2222);

    int id1 = server.get_or_assign_client_id(c1);
    int id2 = server.get_or_assign_client_id(c2);
    int id3 = server.get_or_assign_client_id(c1);

    ASSERT_TRUE(id1 != 0 && id2 != 0);
    ASSERT_EQ(id1, id3);
    ASSERT_NE(id1, id2);
}

// ---------------- TcpServer Tests ----------------

TEST(TcpServerApiTest, CanConstructAndStartTcpServer) {
    ServerConfig cfg;
    cfg.port = 56234;

    TcpServer server(cfg, receive_callback, client_connect_callback, client_disconnect_callback);

    Error err = server.listen();
    ASSERT_EQ(err.code(), ErrorCode::NO_ERROR);
    server.gracefull_shutdown();
}

TEST(TcpServerApiTest, SendSyncDoesNotCrashOnInvalidFd) {
    ServerConfig cfg;
    cfg.port = 56235;

    TcpServer server(cfg, receive_callback, client_connect_callback, client_disconnect_callback);

    Error err = server.listen();
    ASSERT_EQ(err.code(), ErrorCode::NO_ERROR);

    server.send(-1, std::vector<uint8_t>{'d', 'a', 't', 'a'});
    server.gracefull_shutdown();
}

// ---------------- UDP Async Send/Receive (requires server) ----------------

TEST(UdpNetworkApiTest, UDPAsyncSendReceive) {
    int port = 60010;

    std::string last_received;

    UdpServer server(port, [&](int, const std::string& req) {
        last_received = req;
        return "Echo:" + req;
    });

    Error server_err = server.start();
    ASSERT_EQ(server_err.code(), ErrorCode::NO_ERROR);
    std::thread server_thread([&]() { server.run(); });

    asio::io_context io;
    NetworkConfig cfg{"127.0.0.1", port};
    UdpClient udp(io, cfg);

    Error err = udp.connect();
    ASSERT_EQ(err.code(), ErrorCode::NO_ERROR);

    std::atomic<bool> done{false};
    std::string final_result;

    udp.recieve_async([&](const std::vector<uint8_t>& data, Error e) {
        ASSERT_EQ(e.code(), ErrorCode::NO_ERROR);
        final_result.assign(data.begin(), data.end());
        done = true;
    });

    std::vector<uint8_t> msg = {'h', 'i'};
    udp.send_async(msg, [&](Error e) { ASSERT_EQ(e.code(), ErrorCode::NO_ERROR); });

    io.run();

    ASSERT_TRUE(done);
    ASSERT_EQ(final_result, "Echo:hi");

    server.stop();
    server_thread.join();
}

// ---------------- TCP Sync Send/Receive ----------------

TEST(TcpNetworkApiTest, TCPSyncSendReceive) {
    ServerConfig cfg;
    cfg.port = 60001;

    std::string last_received;

    TcpServer server(cfg, receive_callback, client_connect_callback, client_disconnect_callback);

    Error err = server.listen();
    ASSERT_EQ(err.code(), ErrorCode::NO_ERROR);

    asio::io_context io;
    NetworkConfig cfg1{"127.0.0.1", cfg.port};
    TcpClientAsio conn(io, cfg1);

    err = conn.connect();
    ASSERT_EQ(err.code(), ErrorCode::NO_ERROR);

    std::vector<uint8_t> msg = {'H', 'i'};
    err = conn.send_sync(msg);
    ASSERT_EQ(err.code(), ErrorCode::NO_ERROR);

    std::vector<uint8_t> out;
    err = conn.recieve_sync(out);
    ASSERT_EQ(err.code(), ErrorCode::NO_ERROR);

    ASSERT_EQ(std::string(out.begin(), out.end()), "Echo:Hi");

    server.gracefull_shutdown();
}

// ---------------- TCP Async Send/Receive ----------------

TEST(TcpNetworkApiTest, TCPAsyncSendReceive) {
    ServerConfig cfg;
    cfg.port = 60002;

    std::string last_received;

    TcpServer server(cfg, receive_callback, client_connect_callback, client_disconnect_callback);

    Error err = server.listen();
    ASSERT_EQ(err.code(), ErrorCode::NO_ERROR);

    asio::io_context io;
    NetworkConfig cfg1{"127.0.0.1", cfg.port};
    TcpClientAsio conn(io, cfg1);

    std::atomic<bool> done{false};
    std::string final_result;

    conn.connect_async([&](Error err) {
        ASSERT_EQ(err.code(), ErrorCode::NO_ERROR);

        std::vector<uint8_t> msg = {'B', 'y', 'e'};
        conn.send_async(msg, [&](Error err2) {
            ASSERT_EQ(err2.code(), ErrorCode::NO_ERROR);

            conn.recieve_async([&](const std::vector<uint8_t>& data, Error err3) {
                ASSERT_EQ(err3.code(), ErrorCode::NO_ERROR);
                final_result.assign(data.begin(), data.end());
                done = true;
            });
        });
    });

    io.run();

    ASSERT_TRUE(done);
    ASSERT_EQ(final_result, "Echo:Bye");

    server.gracefull_shutdown();
}
