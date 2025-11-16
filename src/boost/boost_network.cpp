#include "boost_network.h"

#include "boost/tcpserver.h"

DeviceManager::DeviceManager()
    : io_context_(),
      timer_(io_context_, std::chrono::seconds(1)),
      work(boost::asio::make_work_guard(io_context_)),
      isRunning(false) {}

DeviceManager::~DeviceManager() {
    stop();
}

void DeviceManager::stop() {
    isRunning = false;
    io_context_.stop();
    if (ioThread.joinable())
        ioThread.join();
}

void DeviceManager::run_io_context() {
    if (!isRunning) {
        isRunning = true;
        ioThread = std::thread([this]() {
            try {
                io_context_.run();
            } catch (const std::exception& e) {
                std::cerr << "IO Context Exception: " << e.what() << std::endl;
            }
        });
    }
}

std::vector<std::shared_ptr<Connection> > DeviceManager::getConnections() const {
    std::vector<std::shared_ptr<Connection> > _connections;
    for (const auto& pair : connections) {
        _connections.push_back(pair.first);
    }
    return _connections;
}

std::vector<bool> DeviceManager::getProtocolType() const {
    std::vector<bool> booleans;
    for (const auto& pair : connections) {
        booleans.push_back(pair.second);
    }
    return booleans;
}

void DeviceManager::start_timer(std::shared_ptr<Connection> connection,
                                std::vector<std::string> topics) {
    timer_.async_wait(std::bind(&DeviceManager::print, this, connection, topics));
}

bool DeviceManager::startTCPServer(int port) {
    try {
        tcp_server = std::make_shared<Server>(std::ref(io_context_), port);  // Listen on port 12345
        run_io_context();
        std::cout << "TCP Server started on port " << port << std::endl;
        return true;
    } catch (std::exception& e) {
        std::cerr << "Error creating TCP acceptor : " << e.what() << "\n";
        return false;
    }
}

bool DeviceManager::addTCPConnection(const std::string& host, unsigned short port,
                                     std::vector<std::string> topics, bool isStandard) {
    try {
        tcp_connection = TCPConnection::create(std::ref(io_context_), host, port);
        run_io_context();
        addConnection(tcp_connection, topics, isStandard);
        return true;
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return false;
    }
}

bool DeviceManager::addUDPConnection(const std::string& host, unsigned short port,
                                     std::vector<std::string> topics, bool isStandard) {
    try {
        udp_connection = UDPConnection::create(std::ref(io_context_), host, port);
        run_io_context();
        addConnection(udp_connection, topics, isStandard);
        return true;
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return false;
    }
}

bool DeviceManager::addSerialConnection(const std::string& portname, unsigned short baud_rate,
                                        std::vector<std::string> topics, bool isStandard) {
    try {
        //        serial_connection = SerialConnection::create(std::ref(io_context_), portname,
        //        baud_rate);
        serial_connection =
            std::make_shared<SerialConnection>(std::ref(io_context_), portname, baud_rate);
        run_io_context();
        addConnection(serial_connection, topics, isStandard);
        std::cout << "serial port opened " << serial_connection->description() << std::endl;
        return true;
    } catch (std::exception& e) {
        std::cerr << "Error openning serial port : " << e.what() << std::endl;
        return false;
    }
}

void DeviceManager::addConnection(std::shared_ptr<Connection> connection,
                                  std::vector<std::string> topics, bool isStandard) {
    connections.push_back(std::pair<std::shared_ptr<Connection>, bool>(connection, isStandard));
    std::cout << "Connection added. Total connections: " << connections.size() << std::endl;

    start_timer(connection, topics);

    // Setup async read for this connection
}

void DeviceManager::removeConnection(std::shared_ptr<Connection> connection) {
    connections.erase(std::remove_if(connections.begin(), connections.end(),
                                     [&connection](const std::pair<std::shared_ptr<Connection>, bool>& pair) {
                                         return pair.first == connection;
                                     }),
                      connections.end());
    std::cout << "Connection removed. Total connections: " << connections.size() << std::endl;
}

void DeviceManager::setReadFunction() {
    for (auto connection : connections) {
        connection.first->asyncRead([this](const std::string& message_, const std::string& topic_) {
            if (data_callback)
                data_callback(message_, topic_);
            std::cout << "Received message: " << message_ << topic_ << std::endl;
        });
    }
}

void DeviceManager::setDataCallback(
    std::function<void(const std::string&, const std::string&)> callback) {
    data_callback = callback;
}
