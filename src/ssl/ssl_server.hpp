#ifndef SSL_SERVER_HPP
#define SSL_SERVER_HPP

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <functional>
#include <memory>
#include <string>

namespace net  = boost::asio;
namespace ssl  = boost::asio::ssl;
namespace beast = boost::beast;
namespace http  = boost::beast::http;
using tcp = boost::asio::ip::tcp;

class ssl_server {
public:
    ssl_server(net::io_context& io_context,
               const std::string& address,
               unsigned short port,
               const std::string& cert_file,
               const std::string& key_file,
               bool require_client_cert = false);

    void start_accept();
    void stop();
    net::io_context& get_io_context();
    ssl::context& get_ssl_context();
    
void receive_request(std::shared_ptr<ssl::stream<tcp::socket>> stream,
                     std::shared_ptr<beast::flat_buffer> buffer,
                     std::shared_ptr<http::request<http::dynamic_body>> req);
void send_message(std::shared_ptr<ssl::stream<tcp::socket>> stream,
                  std::shared_ptr<http::response<http::string_body>> res);


private:
    void do_accept();
    void on_session(std::shared_ptr<ssl::stream<tcp::socket>> stream);

    net::io_context& m_io_context;
    tcp::acceptor m_acceptor;
    ssl::context m_ssl_context;
    std::string m_address;
    unsigned short m_port;
    bool m_require_client_cert;
};

#endif // SSL_SERVER_HPP
