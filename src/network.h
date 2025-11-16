#pragma once

#include <iostream>
#include <string>

enum class NetworkLevel { info, warn, error };

enum class NetworkType { standard, booost };
enum class SinkType { Basic, Daily, Hourly };
struct NetworkConfig {
    std::string fileName;  // Log file name                   // How many rotated files to keep (for
                           // rotating_file_sink)
};

// --- Abstract Logger Interface ---
class INetwork {
  public:
    virtual ~INetwork() = default;
    INetwork(NetworkConfig cfg) {}

    virtual void info(const std::string& message) = 0;

  protected:
    NetworkConfig currentLog;
};
