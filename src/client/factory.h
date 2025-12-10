#pragma once

#include <string.h>

#include <memory>

#include "client_interface.h"
#include "tcp.h"

class ClientFactory {
  public:
    static std::shared_ptr<ClientInterface> create(NetworkConfig cfg) {
        switch (cfg.backend_type) {
            case BackendType::ASIO:
                switch (cfg.connection_type) {
                    case ConnectionType::TCP:
                        // return std::make_shared(TCP());
                }
        }
    }
};