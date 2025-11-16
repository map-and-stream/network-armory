// Connection.h
#ifndef CONNECTION_H
#define CONNECTION_H

#include <string>
#include <functional>
#include <array>

enum class DeviceType {serial = 1, tcp = 2, udp = 3, broker = 4};

class Connection {
public:
    virtual ~Connection() = default;
    virtual void send(const std::string& message, const std::string& topic = "all") = 0;
    virtual void asyncRead(std::function<void(const std::string&,  const std::string&)> callback) = 0; // New method
    virtual bool isConnected() const = 0;
    virtual DeviceType type() const = 0;
    virtual std::string description() const = 0;
protected:
    std::array<char, 1024> read_buffer; // Buffer for incoming data
    std::function<void(const std::string&, const std::string&)> read_callback; // Callback function for data
};

#endif // CONNECTION_H
