#ifndef SSL_CLIENT_HPP
#define SSL_CLIENT_HPP

#include <string>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>

namespace net  = boost::asio;
namespace ssl  = boost::asio::ssl;
namespace beast = boost::beast;
using tcp = boost::asio::ip::tcp;

class ssl_client
{
public:
    ssl_client(std::string host = "www.example.com",
               std::string port = "443",
               std::string target = "/",
               int version = 11);

    std::string fetch();

private:
    std::string m_host;
    std::string m_port;
    std::string m_target;
    int m_version;

    net::io_context m_ioc;
    ssl::context m_ctx;
    tcp::resolver m_resolver;
    beast::ssl_stream<beast::tcp_stream> m_stream;
};

#endif // SSL_CLIENT_HPP
