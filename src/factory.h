#pragma once

#include <memory>
#include <thread>
#include <asio.hpp>

#include "client/client_interface.h"
#include "client/asio/tcp_client.h"
#include "client/asio/udp_client.h"
// #include "client/posix/tcp_client_posix.h"   // future
// #include "client/posix/udp_client_posix.h"   // future

class ClientFactory {
public:
    static std::shared_ptr<ClientInterface> create(const NetworkConfig& cfg) {
        // Shared io_context for all ASIO clients
        static auto io = std::make_shared<asio::io_context>();

        static auto work = std::make_shared<
            asio::executor_work_guard<asio::io_context::executor_type>
        >(asio::make_work_guard(*io));

        static std::thread io_thread([] {
            io->run();
        });

        // -----------------------------
        // BACKEND SELECTION
        // -----------------------------
        switch (cfg.backend_type) {

            case NetworkConfig::BackendType::ASIO:
                switch (cfg.connection_type) {
                    case ClientType::TCP:
                        return std::make_shared<TcpClientAsio>(cfg, io);

                    case ClientType::UDP:
                        return std::make_shared<UdpClient>(cfg, io);

                    default:
                        return nullptr;
                }

            case NetworkConfig::BackendType::POSIX:
                // TODO: implement POSIX clients
                // return std::make_shared<TcpClientPosix>(cfg);
                // return std::make_shared<UdpClientPosix>(cfg);
                return nullptr;
        }

        return nullptr;
    }
};
