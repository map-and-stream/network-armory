#include <gtest/gtest.h>

#include "tcp_test.cpp"
#include "udp_test.cpp"

std::string last_server_received;
std::string last_udp_server_received;

TEST(TcpNetworkTest, ProjectTcpConnection) {
    run_tcp_test();
}

TEST(UdpNetworkTest, ProjectUdpConnection) {
    ASSERT_TRUE(run_udp_project_test());
}
