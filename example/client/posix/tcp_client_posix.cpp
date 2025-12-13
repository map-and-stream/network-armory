#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "client/posix/tcp_client.h"
#include "error.h"

// Synchronous TCP client example
void tcp_client_sync() {
    NetworkConfig cfg;
    cfg.ip = "127.0.0.1";
    cfg.port = 8083;
    TcpClientPosix client(cfg);

    Error err = client.connect();
    if (err.code() != ErrorCode::NO_ERROR) {
        std::cerr << "Connect failed: " << err.message() << std::endl;
        exit(1);
    }

    std::string msg = "Hello Server!";
    std::vector<uint8_t> msg_vec(msg.begin(), msg.end());
    err = client.send_sync(msg_vec);
    if (err.code() != ErrorCode::NO_ERROR) {
        std::cerr << "Send failed: " << err.message() << std::endl;
    }

    std::vector<uint8_t> receive_data;
    err = client.recieve_sync(receive_data);
    if (err.code() == ErrorCode::NO_ERROR) {
        std::string server_reply(receive_data.begin(), receive_data.end());
        std::cout << "Server reply: " << server_reply << std::endl;
    } else {
        std::cerr << "Receive failed: " << err.message() << std::endl;
    }

    client.disconnect();
}

// Callback for send/receive completion (Error-only version)
void operation_callback(Error err) {
    if (err.code() != ErrorCode::NO_ERROR)
        std::cerr << "[Error] " << err.message() << std::endl;
}

// Callback for asynchronous receive data
void data_receive_callback(const std::vector<uint8_t>& data, Error err) {
    if (err.code() == ErrorCode::NO_ERROR) {
        std::string server_reply(data.begin(), data.end());
        std::cout << "[Reply] " << server_reply << std::endl;
    } else {
        std::cerr << "[Receive error] " << err.message() << std::endl;
    }
}

// Asynchronous TCP client example with interactive input
void tcp_async_callback() {
    NetworkConfig cfg;
    cfg.ip = "127.0.0.1";
    cfg.port = 8083;
    TcpClientPosix client(cfg);

    Error err = client.connect_async(operation_callback);
    if (err.code() != ErrorCode::NO_ERROR) {
        std::cerr << "Async connect failed: " << err.message() << std::endl;
        exit(1);
    }

    // Initial message to server
    std::string msg = "Hello Server!";
    std::vector<uint8_t> msg_vec(msg.begin(), msg.end());
    client.send_async(msg_vec, operation_callback);

    // Start async receive (runs in a background thread)
    err = client.recieve_async(data_receive_callback);
    if (err.code() != ErrorCode::NO_ERROR) {
        std::cerr << "Async receive start failed: " << err.message() << std::endl;
        client.disconnect();
        return;
    }

    // Interactive loop: send user input to server
    std::string input;
    while (std::getline(std::cin, input)) {
        if (input.empty())
            continue;
        std::vector<uint8_t> input_vec(input.begin(), input.end());
        client.send_async(input_vec, operation_callback);
    }

    client.disconnect();
}

int main() {
    // Uncomment below for sync mode
    // tcp_client_sync();

    // Run async mode (Non-blocking, auto-connect, background receive)
    tcp_async_callback();

    return 0;
}
