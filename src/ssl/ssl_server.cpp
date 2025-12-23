#include "ssl_server.hpp"
#include <iostream>
#include <boost/system/error_code.hpp>

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
    // load cert and key
    m_ssl_context.use_certificate_chain_file(cert_file);
    m_ssl_context.use_private_key_file(key_file, ssl::context::pem);

    if (m_require_client_cert) {
        m_ssl_context.set_verify_mode(ssl::verify_fail_if_no_peer_cert | ssl::verify_peer);
    } else {
        m_ssl_context.set_verify_mode(ssl::verify_none);
    }

    tcp::endpoint endpoint(net::ip::make_address(m_address), m_port);
    boost::system::error_code ec;

    m_acceptor.open(endpoint.protocol(), ec);
    if(ec) throw std::runtime_error("acceptor open failed: " + ec.message());

    m_acceptor.set_option(net::socket_base::reuse_address(true), ec);
    if(ec) throw std::runtime_error("set_option failed: " + ec.message());

    m_acceptor.bind(endpoint, ec);
    if(ec) throw std::runtime_error("bind failed: " + ec.message());

    m_acceptor.listen(net::socket_base::max_listen_connections, ec);
    if(ec) throw std::runtime_error("listen failed: " + ec.message());
}

void ssl_server::start_accept()
{
    do_accept();
}

void ssl_server::do_accept()
{
    auto socket = std::make_shared<ssl::stream<tcp::socket>>(m_io_context, m_ssl_context);

    m_acceptor.async_accept(socket->lowest_layer(),
        [this, socket](const boost::system::error_code& ec) {
            if (ec) {
                std::cerr << "Accept error: " << ec.message() << "\n";
            } else {
                // perform TLS handshake then handle session
                socket->async_handshake(ssl::stream_base::server,
                    [this, socket](const boost::system::error_code& hec) {
                        if (hec) {
                            std::cerr << "Handshake error: " << hec.message() << "\n";
                        } else {
                            on_session(socket);
                        }
                    });
            }
            // continue accepting
            do_accept();
        });
}


void ssl_server::receive_request(std::shared_ptr<ssl::stream<tcp::socket>> stream,
    std::shared_ptr<beast::flat_buffer> buffer,
    std::shared_ptr<http::request<http::dynamic_body>> req)
{
http::async_read(*stream, *buffer, *req,
[this, stream, buffer, req](const boost::system::error_code& ec, std::size_t bytes_transferred) {
(void)bytes_transferred;
if (ec) {
std::cerr << "HTTP read error: " << ec.message() << "\n";
stream->async_shutdown([stream](const boost::system::error_code&){});
return;
}

auto res = std::make_shared<http::response<http::string_body>>();
res->version(req->version());
res->result(http::status::ok);
res->set(http::field::server, "beast-ssl-server");
res->set(http::field::content_type, "text/plain");
res->body() = std::string("Hello from server\n");
res->prepare_payload();


send_message(stream, res);
});
}


void ssl_server::send_message(std::shared_ptr<ssl::stream<tcp::socket>> stream,
 std::shared_ptr<http::response<http::string_body>> res)
{
http::async_write(*stream, *res,
[stream, res](const boost::system::error_code& write_ec, std::size_t) {
if (write_ec) {
std::cerr << "HTTP write error: " << write_ec.message() << "\n";
}

stream->async_shutdown([stream](const boost::system::error_code& shut_ec) {
// ignore shutdown errors
});
});
}


void ssl_server::on_session(std::shared_ptr<ssl::stream<tcp::socket>> stream)
{
auto buffer = std::make_shared<beast::flat_buffer>();
auto req = std::make_shared<http::request<http::dynamic_body>>();
receive_request(stream, buffer, req);
}


void ssl_server::stop()
{
    boost::system::error_code ec;
    m_acceptor.close(ec);
}

net::io_context& ssl_server::get_io_context() { return m_io_context; }
ssl::context& ssl_server::get_ssl_context() { return m_ssl_context; }
