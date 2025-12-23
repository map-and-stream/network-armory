#include <gtest/gtest.h>

#include <asio.hpp>
#include <atomic>
#include <string>
#include <thread>
#include <vector>

#include "client/asio/udp_client.h"
#include "client/client_interface.h"
#include "factory.h"
#include "server/posix/tcp_server.h"
#include "server/posix/udp_server.h"
#include "server/server_interface.h"

// ====================== Test 1: Construct TCP Client via factory ======================

TEST(NetworkBasicTest, ConstructTCPClient) {
    NetworkConfig cfg{"127.0.0.1", 12345};
    cfg.connection_type = ClientType::TCP;

    auto conn = ClientFactory::create(cfg);
    ASSERT_NE(conn, nullptr);
}

// ====================== Test 2: TCP Connect Fails With Bad IP (Sync) ==================

TEST(NetworkConnectionTest, TCPConnectInvalidIp) {
    NetworkConfig cfg{"invalid_ip", 12345};
    cfg.connection_type = ClientType::TCP;

    auto conn = ClientFactory::create(cfg);
    ASSERT_NE(conn, nullptr);

    Error err = conn->connect();
    ASSERT_NE(err.code(), ErrorCode::NO_ERROR);
}

// ====================== Test 3: TCP Connect Fails With Bad IP (Async) =================

TEST(NetworkConnectionTest, TCPConnectAsyncInvalidIp) {
    NetworkConfig cfg{"invalid_ip", 12345};
    cfg.connection_type = ClientType::TCP;

    auto conn = ClientFactory::create(cfg);
    ASSERT_NE(conn, nullptr);

    std::atomic<bool> done{false};
    Error result;

    conn->connect_async([&](Error e) {
        result = e;
        done = true;
    });

    for (int i = 0; i < 40 && !done; ++i) std::this_thread::sleep_for(std::chrono::milliseconds(5));

    ASSERT_TRUE(done);
    ASSERT_NE(result.code(), ErrorCode::NO_ERROR);
}

// ====================== Test 4: Construct UDP Client (direct) =========================

TEST(NetworkBasicTest, ConstructUDPClient) {
    auto io = std::make_shared<asio::io_context>();
    NetworkConfig cfg{"127.0.0.1", 5555};
    cfg.connection_type = ClientType::UDP;

    UdpClient udp(cfg, io);
}

// ====================== Test 5: UDP Connect Fails With Bad IP =========================

TEST(NetworkConnectionTest, UDPOpenInvalidIp) {
    auto io = std::make_shared<asio::io_context>();
    NetworkConfig cfg{"invalid_ip", 45555};
    cfg.connection_type = ClientType::UDP;

    UdpClient udp(cfg, io);
    Error err = udp.connect();
    ASSERT_NE(err.code(), ErrorCode::NO_ERROR);
}

// ====================== Test 6: Can Construct and Start UdpServer =====================

TEST(NetworkServerTest, CanConstructAndStartUdpServer) {
    int port = 60555;
    UdpServer server(port, [](int, const std::string&) { return "OK"; });
    Error err = server.start();
    ASSERT_EQ(err.code(), ErrorCode::NO_ERROR);
    server.stop();
}

// ====================== Test 7: UdpServer Unique Client Ids ===========================

TEST(NetworkServerTest, UdpServerUniqueIds) {
    UdpServer server(0, [&](int, const std::string&) { return "ping"; });

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

// ====================== Test 8: Can Construct and Start TcpServer =====================

TEST(NetworkServerTest, CanConstructAndStartTcpServer) {
    ServerConfig cfg;
    cfg.port = 60666;

    auto rx = [](int, const std::string&, const std::vector<uint8_t>&) {};
    auto on_con = [](int, const std::string&) {};
    auto on_disc = [](int, const std::string&) {};

    TcpServer server(cfg, rx, on_con, on_disc);
    Error err = server.listen();
    ASSERT_EQ(err.code(), ErrorCode::NO_ERROR);
    server.gracefull_shutdown();
}

// ====================== Test 9: TcpServer SendSync on Bad Fd Does Not Crash ===========

TEST(NetworkServerTest, TcpServerSendSyncBadFd) {
    ServerConfig cfg;
    cfg.port = 60667;

    auto rx = [](int, const std::string&, const std::vector<uint8_t>&) {};
    auto on_con = [](int, const std::string&) {};
    auto on_disc = [](int, const std::string&) {};

    TcpServer server(cfg, rx, on_con, on_disc);
    Error err = server.listen();
    ASSERT_EQ(err.code(), ErrorCode::NO_ERROR);

    server.send(-1, std::vector<uint8_t>{'t', 'e', 's', 't'});
    server.gracefull_shutdown();
}

// ====================== Test 10: UDP Async Send/Receive with server ===================

TEST(NetworkFeatureTest, UDPAsyncSendReceive) {
    int port = 60777;
    std::string last_received;

    UdpServer server(port, [&](int, const std::string& req) {
        last_received = req;
        return "Echo:" + req;
    });

    Error server_err = server.start();
    ASSERT_EQ(server_err.code(), ErrorCode::NO_ERROR);

    std::thread srv_thread([&]() { server.run(); });

    auto io = std::make_shared<asio::io_context>();
    NetworkConfig cfg{"127.0.0.1", port};
    cfg.connection_type = ClientType::UDP;

    UdpClient udp(cfg, io);
    Error err = udp.connect();
    ASSERT_EQ(err.code(), ErrorCode::NO_ERROR);

    std::atomic<bool> done{false};
    std::string final_result;

    udp.recieve_async([&](const std::vector<uint8_t>& data, Error e) {
        ASSERT_EQ(e.code(), ErrorCode::NO_ERROR);
        final_result.assign(data.begin(), data.end());
        done = true;
    });

    std::vector<uint8_t> msg = {'T', 'e', 's', 't'};
    udp.send_async(msg, [&](Error e) { ASSERT_EQ(e.code(), ErrorCode::NO_ERROR); });

    io->run();

    ASSERT_TRUE(done);
    ASSERT_EQ(final_result, "Echo:Test");

    server.stop();
    srv_thread.join();
}

// ====================== Test 11: TCP Sync Send/Receive =================================

TEST(NetworkFeatureTest, TCPSyncSendReceive) {
    ServerConfig cfg;
    cfg.port = 60880;

    std::string last_received;

    TcpServer* srv_ptr = nullptr;
    auto rx_cb = [&](int fd, const std::string&, const std::vector<uint8_t>& data) {
        last_received = std::string(data.begin(), data.end());
        std::vector<uint8_t> resp{'E', 'c', 'h', 'o', ':'};
        resp.insert(resp.end(), data.begin(), data.end());
        srv_ptr->send(fd, resp);
    };

    auto on_con = [](int, const std::string&) {};
    auto on_disc = [](int, const std::string&) {};

    TcpServer server(cfg, rx_cb, on_con, on_disc);
    srv_ptr = &server;

    Error err = server.listen();
    ASSERT_EQ(err.code(), ErrorCode::NO_ERROR);

    NetworkConfig client_cfg{"127.0.0.1", cfg.port};
    client_cfg.connection_type = ClientType::TCP;

    auto conn = ClientFactory::create(client_cfg);
    ASSERT_NE(conn, nullptr);

    err = conn->connect();
    ASSERT_EQ(err.code(), ErrorCode::NO_ERROR);

    std::vector<uint8_t> msg = {'O', 'K'};
    err = conn->send_sync(msg);
    ASSERT_EQ(err.code(), ErrorCode::NO_ERROR);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    std::vector<uint8_t> out;
    err = conn->recieve_sync(out);
    ASSERT_EQ(err.code(), ErrorCode::NO_ERROR);
    ASSERT_EQ(std::string(out.begin(), out.end()), "Echo:OK");

    server.gracefull_shutdown();
}

// ====================== Test 12: TCP Async Send/Receive =================================

TEST(NetworkFeatureTest, TCPAsyncSendReceive) {
    ServerConfig cfg;
    cfg.port = 60881;

    std::string last_received;
    TcpServer* srv_ptr = nullptr;

    auto rx_cb = [&](int fd, const std::string&, const std::vector<uint8_t>& data) {
        last_received = std::string(data.begin(), data.end());
        std::vector<uint8_t> resp{'E', 'c', 'h', 'o', ':'};
        resp.insert(resp.end(), data.begin(), data.end());
        srv_ptr->send(fd, resp);
    };

    auto on_con = [](int, const std::string&) {};
    auto on_disc = [](int, const std::string&) {};

    TcpServer server(cfg, rx_cb, on_con, on_disc);
    srv_ptr = &server;

    Error err = server.listen();
    ASSERT_EQ(err.code(), ErrorCode::NO_ERROR);

    NetworkConfig client_cfg{"127.0.0.1", cfg.port};
    client_cfg.connection_type = ClientType::TCP;

    auto conn = ClientFactory::create(client_cfg);
    ASSERT_NE(conn, nullptr);

    std::atomic<bool> done{false};
    std::string final_result;

    conn->connect_async([&](Error err1) {
        ASSERT_EQ(err1.code(), ErrorCode::NO_ERROR);

        std::vector<uint8_t> msg = {'Y', 'a', 'y'};
        conn->send_async(msg, [&](Error err2) {
            ASSERT_EQ(err2.code(), ErrorCode::NO_ERROR);

            conn->recieve_async([&](const std::vector<uint8_t>& data, Error err3) {
                ASSERT_EQ(err3.code(), ErrorCode::NO_ERROR);
                final_result.assign(data.begin(), data.end());
                done = true;
            });
        });
    });

    for (int i = 0; i < 200 && !done; ++i)
        std::this_thread::sleep_for(std::chrono::milliseconds(5));

    ASSERT_TRUE(done);
    ASSERT_EQ(final_result, "Echo:Yay");

    server.gracefull_shutdown();
}
