#include "ssl_client.hpp"
#include <boost/beast/http.hpp>
#include <boost/beast/core.hpp>
#include <openssl/err.h>
#include <iostream>

namespace http = boost::beast::http;

ssl_client::ssl_client(std::string host,
                       std::string port,
                       std::string target,
                       int version)
    : m_host(std::move(host))
    , m_port(std::move(port))
    , m_target(std::move(target))
    , m_version(version)
    , m_ioc()
    , m_ctx(ssl::context::tls_client)
    , m_resolver(m_ioc)
    , m_stream(m_ioc, m_ctx)
{
    boost::system::error_code ec;
    m_ctx.set_default_verify_paths(ec);
    if(ec) {
        std::cerr << "Warning: set_default_verify_paths failed: " << ec.message() << "\n";
    }
    m_ctx.set_verify_mode(ssl::verify_peer);
}

std::string ssl_client::fetch()
{
    boost::system::error_code ec;

    auto const results = m_resolver.resolve(m_host, m_port, ec);
    if(ec) throw boost::system::system_error{ec};

    if(! SSL_set_tlsext_host_name(m_stream.native_handle(), m_host.c_str()))
    {
        beast::error_code err{static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()};
        throw beast::system_error{err};
    }

    beast::get_lowest_layer(m_stream).connect(results, ec);
    if(ec) throw boost::system::system_error{ec};

    m_stream.handshake(ssl::stream_base::client, ec);
    if(ec) throw boost::system::system_error{ec};

    http::request<http::string_body> req{http::verb::get, m_target, m_version};
    req.set(http::field::host, m_host);
    req.set(http::field::user_agent, "boost-beast/1.0");
    req.set(http::field::connection, "close");

    http::write(m_stream, req, ec);
    if(ec) throw boost::system::system_error{ec};

    beast::flat_buffer buffer;
    http::response<http::dynamic_body> res;
    http::read(m_stream, buffer, res, ec);
    if(ec) throw boost::system::system_error{ec};

    std::string body = beast::buffers_to_string(res.body().data());

    beast::error_code shutdown_ec;
    m_stream.shutdown(shutdown_ec);
    if(shutdown_ec && shutdown_ec != net::error::eof) {
        std::cerr << "Shutdown error: " << shutdown_ec.message() << "\n";
    }

    return body;
}
