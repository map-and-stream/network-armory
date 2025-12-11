#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "error.h"
#include "client/posix/tcp_client.h"

void tcp_client_sync() {
    NetworkConfig cfg;
    cfg.ip = "127.0.0.1";
    cfg.port = 8083;
    TcpClientPosix client(cfg);

    Error err = client.connect();
    if (err.code() != ErrorCode::NO_ERROR) {
        exit(1);
    }

    std::string msg = "Hello Server!";
    std::vector<uint8_t> msg_vec(msg.begin(), msg.end());
    client.send_sync(msg_vec);

    std::vector<uint8_t> recieve_data;
    client.recieve_sync(recieve_data);
    std::string server_reply(recieve_data.begin(), recieve_data.end());
    std::cout << "Server reply: " << server_reply << std::endl;

    client.disconnect();
}

void recieve_callback(Error err) {
    std::cout << err.message();
}

void recieve_data(const std::vector<uint8_t>& data, Error err) {
    if (err.code() == ErrorCode::NO_ERROR) {
        std::string server_reply(data.begin(), data.end());
        std::cout << server_reply;
    } else
        std::cout << err.message();
}

void tcp_async_callback() {
    NetworkConfig cfg;
    cfg.ip = "127.0.0.1";
    cfg.port = 8083;
    TcpClientPosix client(cfg);

    Error err = client.connect_async(recieve_callback);
    if (err.code() != ErrorCode::NO_ERROR) {
        exit(1);
    }

    std::string msg = "Hello Server!";
    std::vector<uint8_t> msg_vec(msg.begin(), msg.end());
    client.send_async(msg_vec, recieve_callback);

    err = client.recieve_async(recieve_data);

    std::string input;
    while (std::getline(std::cin, input)) {
        std::cout << "getline => " << input << "\n";
        std::vector<uint8_t> msg_vec(input.begin(), input.end());
        client.send_async(msg_vec, recieve_callback);
    }


    client.disconnect();
}

int main() {
    // tcp_client_sync();
    // tcp_async_in_sync_mode();
    tcp_async_callback();  // BEST CHOICE. NON-BLOCKING, ASYNC, AUTO-CONNECT

    return 0;
}
