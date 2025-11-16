// DeviceManager.h
#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <boost/asio.hpp>
#include <functional>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include "connection.h"
#include "serialconnection.h"
#include "tcpserver.h"
#include "udpconnection.h"

class DeviceManager {
  public:
    // Meyer's singleton instance accessor
    static DeviceManager* Instance() {
        static DeviceManager instance;
        return &instance;
    }

    bool startTCPServer(int port);
    bool addTCPConnection(const std::string& host, unsigned short port,
                          std::vector<std::string> topics, bool isStandard);
    bool addUDPConnection(const std::string& host, unsigned short port,
                          std::vector<std::string> topics, bool isStandard);
    bool addSerialConnection(const std::string& portname, unsigned short baud_rate,
                             std::vector<std::string> topics, bool isStandard);
    bool addBrokerConnection(const std::string& host, unsigned short port,
                             std::vector<std::string> topics, bool isStandard);

    void setDataCallback(std::function<void(const std::string&, const std::string&)> callback);

    std::vector<std::shared_ptr<Connection>> getConnections() const;
    std::vector<bool> getProtocolType() const;
    size_t GetNDevices() { return connections.size(); }

    void addConnection(std::shared_ptr<Connection> connection, std::vector<std::string> topics,
                       bool isStandard);
    void removeConnection(std::shared_ptr<Connection> connection);

    void setReadFunction();

  private:
    // Disallow copying and assignment
    DeviceManager(const DeviceManager&) = delete;
    DeviceManager& operator=(const DeviceManager&) = delete;

    void start_timer(std::shared_ptr<Connection> connection, std::vector<std::string> topics);
    void print(std::shared_ptr<Connection>, std::vector<std::string> topics);
    void stop();
    void run_io_context();

    DeviceManager();
    ~DeviceManager();

    boost::asio::io_context io_context_;
    boost::asio::steady_timer timer_;
    std::thread ioThread;
    std::thread brokerThread;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work;
    bool isRunning;

    std::function<void(const std::string&, const std::string&)> data_callback;

    std::vector<std::pair<std::shared_ptr<Connection>, bool>> connections;
    std::shared_ptr<Server> tcp_server;
    TCPConnection::Ptr tcp_connection;
    UDPConnection::Ptr udp_connection;
    SerialConnection::Ptr serial_connection;
};

#endif  // DEVICEMANAGER_H
