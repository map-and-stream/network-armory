#include "tcp_client.h"

#include <cstddef>

#include "client/client_interface.h"
#include "client/error.h"
#include <iostream>

TcpClient::TcpClient(const NetworkConfig& cfg)
    : ClientInterface(cfg),
      serverIP(cfg.ip),
      serverPort(cfg.port),
      sock(-1),
      running(false),
      reconnectDelayMs(2000),
      delimiter() {}

Error TcpClient::connect() {
    bool result = internal_connect(true);
    Error err;
    if (!result) {
        err.set_code(ErrorCode::CONNECTION_FAILED);
    }
    return err;
}
Error TcpClient::connect_async(std::function<void(Error)> callback) {
    bool result = internal_connect(false);
    Error err;
    if (!result) {
        err.set_code(ErrorCode::CONNECTION_FAILED);
    }
    return err;
}

Error TcpClient::send_sync(const std::vector<uint8_t>& data) {
    bool result = sendMessage(data);
    Error err;
    if (!result) {
        err.set_code(ErrorCode::SEND_FAILED);
    }
    return err;
}

Error TcpClient::send_async(const std::vector<uint8_t>& data, std::function<void(Error)> callback) {
    return send_sync(data);
}

Error TcpClient::recieve_sync(std::vector<uint8_t>& out) {
    Error err;
    std::lock_guard<std::mutex> lock(sockMutex);
    if (sock < 0) {
        err.set_code(ErrorCode::RECEIVE_FAILED);  // TODO
        return err;
    }

    char buf[1024] = {0};
    int bytes = read(sock, buf, sizeof(buf));
    if (bytes <= 0) {
        err.set_code(ErrorCode::RECEIVE_FAILED);
        std::cerr << "Read failed or connection closed\n";
        return err;
    }
    out.assign(reinterpret_cast<uint8_t*>(buf), reinterpret_cast<uint8_t*>(buf) + bytes);
    return err;
}

Error TcpClient::recieve_async(std::function<void(const std::vector<uint8_t>&, Error)> callback) {
    Error err;
    if (running)
        return err;
    running = true;

    recvThread = std::thread([this, callback, err]() {
        while (running) {
            if (sock < 0 && !internal_connect(false)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(reconnectDelayMs));
                continue;
            }

            char buffer[1024];
            int bytes = read(sock, buffer, sizeof(buffer));

            if (bytes > 0) {
                std::vector<uint8_t> data(buffer, buffer + bytes);

                if (callback)
                    callback(data, err);
            } else if (bytes == 0) {
                reconnect();
            } else {
                if (errno != EAGAIN && errno != EWOULDBLOCK)
                    reconnect();
                std::this_thread::sleep_for(std::chrono::milliseconds(30));
            }
        }
    });
}

Error TcpClient::disconnect() {
    Error err;
    stop();
    std::lock_guard<std::mutex> lock(sockMutex);
    if (sock >= 0) {
        close(sock);
        sock = -1;
    }
    return err;
}

//------------------------------------------- PRIVATE //-------------------------------------------
bool TcpClient::sendMessage(const std::vector<uint8_t>& data) {
    std::lock_guard<std::mutex> lock(sockMutex);
    if (sock < 0)
        return false;

    // std::string data = message + delimiter;
    return ::send(sock, data.data(), data.size(), 0) >= 0;
}

bool TcpClient::internal_connect(bool isBlocking) {
    std::lock_guard<std::mutex> lock(sockMutex);

    if (sock >= 0)
        close(sock);
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        return false;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(serverPort);

    if (inet_pton(AF_INET, serverIP.c_str(), &addr.sin_addr) <= 0)
        return false;

    if (::connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(sock);
        sock = -1;
        return false;
    }

    int flags = fcntl(sock, F_GETFL, 0);
    if (!isBlocking)
        fcntl(sock, F_SETFL, flags | O_NONBLOCK);
    else
        fcntl(sock, F_SETFL, flags & ~O_NONBLOCK);

    std::cout << "Connected to server ✅" << std::endl;
    return true;
}

void TcpClient::stop() {
    running = false;
    if (recvThread.joinable())
        recvThread.join();
}