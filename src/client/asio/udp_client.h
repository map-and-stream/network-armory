#pragma once

#include <asio.hpp>
#include <functional>
#include <memory>
#include <vector>

#include "client/client_interface.h"
#include "error.h"

class UdpClient :
    public ClientInterface,
    public std::enable_shared_from_this<UdpClient>
{
public:
    UdpClient(const NetworkConfig& cfg,
              std::shared_ptr<asio::io_context> io);

    Error connect() override;
    Error connect_async(AsyncCallback callback) override;
    Error disconnect() override;

    Error send_async(const std::vector<uint8_t>& data,
                     AsyncCallback callback) override;

    Error recieve_async(ReceiveCallback callback) override;

    // We can keep send_sync / recieve_sync as NOT_IMPLEMENTED
    // from base class.

private:
    std::shared_ptr<asio::io_context> io_;
    asio::ip::udp::socket socket_;
    asio::ip::udp::endpoint server_endpoint_;
};
