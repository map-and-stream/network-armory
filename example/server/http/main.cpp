#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <iostream>

namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;
using tcp = asio::ip::tcp;

int main() {
    try {
        asio::io_context ioc{1};

        /*
        ip white list []string
        port
        tls{}
        timeout
        */

        tcp::acceptor acceptor{ioc, {tcp::v4(), 8080}};
        std::cout << "Listening on http://0.0.0.0:8080\n";

        for (;;) {
            tcp::socket socket{ioc};
            acceptor.accept(socket);

            beast::flat_buffer buffer;
            http::request<http::string_body> req;

            // Read request
            http::read(socket, buffer, req);

            http::response<http::string_body> res;

            /*
                tcp::endpoint remote = socket.remote_endpoint();
                std::string ip = remote.address().to_string();
                std::string body = req.body();
                response = callback(ip, method, req.body())
                http::write(socket, response);
             */

            if (req.method() == http::verb::get) {
                res.result(http::status::ok);
                res.version(req.version());
                res.set(http::field::server, "Boost.Beast");
                res.set(http::field::content_type, "text/plain");
                res.body() = "GET Hello World\n";
            } else if (req.method() == http::verb::post) {
                res.result(http::status::ok);
                res.version(req.version());
                res.set(http::field::server, "Boost.Beast");
                res.set(http::field::content_type, "text/plain");
                res.body() = "POST Hello World\n";
            } else {
                res.result(http::status::method_not_allowed);
                res.version(req.version());
                res.set(http::field::content_type, "text/plain");
                res.body() = "Only GET is supported\n";
            }

            res.prepare_payload();

            // Send response
            http::write(socket, res);

            beast::error_code ec;
            socket.shutdown(tcp::socket::shutdown_send, ec);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}
