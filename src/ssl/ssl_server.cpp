// ssl_server.cpp
#include "ssl_server.hpp"
#include <iostream>
#include <fstream>

ssl_server::ssl_server(net::io_context& io_context,
                       const std::string& address,
                       unsigned short port,
                       const std::string& cert_file,
                       const std::string& key_file,
                       bool require_client_cert)
    : m_io_context(io_context),
      m_acceptor(io_context),
      m_ssl_context(ssl::context::tls_server),
      m_address(address),
      m_port(port),
      m_require_client_cert(require_client_cert)
{
    // Load server certificate and private key
    m_ssl_context.use_certificate_chain_file(cert_file);
    m_ssl_context.use_private_key_file(key_file, ssl::context::pem);

    if (m_require_client_cert) {
        // Require client certificate and set verify mode (caller should set CA file separately)
        m_ssl_context.set_verify_mode(ssl::verify_fail_if_no_peer_cert | ssl::verify_peer);
    } else {
        m_ssl_context.set_verify_mode(ssl::verify_none);
    }

    // Open and bind acceptor
    tcp::endpoint endpoint(net::ip::make_address(m_address), m_port);
    boost::system::error_code ec;

    m_acceptor.open(endpoint.protocol(), ec);
    if (ec) throw std::runtime_error("acceptor open failed: " + ec.message());

    m_acceptor.set_option(net::socket_base::reuse_address(true), ec);
    if (ec) throw std::runtime_error("set_option failed: " + ec.message());

    m_acceptor.bind(endpoint, ec);
    if (ec) throw std::runtime_error("bind failed: " + ec.message());

    m_acceptor.listen(net::socket_base::max_listen_connections, ec);
    if (ec) throw std::runtime_error("listen failed: " + ec.message());
}

void ssl_server::start_accept(AcceptHandler handler) {
    m_accept_handler = std::move(handler);
    do_accept();
}

void ssl_server::do_accept() {
    // Create a new SSL stream for the incoming connection
    auto socket = std::make_shared<ssl::stream<tcp::socket>>(m_io_context, m_ssl_context);

    m_acceptor.async_accept(socket->lowest_layer(),
        [this, socket](const boost::system::error_code& ec) {
            if (ec) {
                if (m_accept_handler) m_accept_handler(ec, nullptr);
            } else {
                // perform handshake
                socket->async_handshake(ssl::stream_base::server,
                    [this, socket](const boost::system::error_code& hec) {
                        if (m_accept_handler) {
                            if (hec) m_accept_handler(hec, nullptr);
                            else m_accept_handler(boost::system::error_code{}, socket);
                        }
                    });
            }
            // continue accepting next
            do_accept();
        });
}

void ssl_server::stop() {
    boost::system::error_code ec;
    m_acceptor.close(ec);
}

net::io_context& ssl_server::get_io_context() { return m_io_context; }
ssl::context& ssl_server::get_ssl_context() { return m_ssl_context; }
