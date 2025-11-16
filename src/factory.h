#pragma once

#include <string.h>

#include "network.h"

class NetworkFactory {
  public:
    static INetwork* createNetwork(NetworkType type, NetworkConfig cfg) {
        if (type == NetworkType::standard) {
        } else if (type == NetworkType::booost) {
        } else {
            throw std::invalid_argument("Invalid logger type");
        }
    }
};