// ssl_server.hpp
#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <memory>
#include <string>
#include <functional>

namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = net::ip::tcp;

using AcceptHandler = std::function<void(const boost::system::error_code&, std::shared_ptr<ssl::stream<tcp::socket>>)>;

class ssl_server {
public:
    // ctor: io_context, addr, port, path to server cert and private key
    ssl_server(net::io_context& io_context,
               const std::string& address,
               unsigned short port,
               const std::string& cert_file,
               const std::string& key_file,
               bool require_client_cert = false);

    // start accepting (async)
    void start_accept(AcceptHandler handler);

    // stop server
    void stop();

    // accessors
    net::io_context& get_io_context();
    ssl::context& get_ssl_context();

private:
    void do_accept();

private:
    net::io_context& m_io_context;
    tcp::acceptor m_acceptor;
    ssl::context m_ssl_context;
    std::string m_address;
    unsigned short m_port;
    bool m_require_client_cert{false};

    AcceptHandler m_accept_handler;
};
