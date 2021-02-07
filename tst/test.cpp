#include <iostream>

#include <nghttp2/asio_http2_client.h>

using boost::asio::ip::tcp;

using namespace nghttp2::asio_http2;
using namespace nghttp2::asio_http2::client;

int main(int argc, char *argv[]) {
  try {
    if (argc < 2) {
      std::cerr << "Usage:  URI" << std::endl;
      return 1;
    }
    boost::system::error_code ec;
    boost::asio::io_service io_service;

    std::string uri = argv[1];
    std::string scheme, host, service;

    if (host_service_from_uri(ec, scheme, host, service, uri)) {
      std::cerr << "error: bad URI: " << ec.message() << std::endl;
      return 1;
    }

    boost::asio::ssl::context tls_ctx(boost::asio::ssl::context::sslv23);
    tls_ctx.set_default_verify_paths();
    // disabled to make development easier...
    // tls_ctx.set_verify_mode(boost::asio::ssl::verify_peer);
    configure_tls_context(ec, tls_ctx);

    auto sess = scheme == "https" ? session(io_service, tls_ctx, host, service)
                                  : session(io_service, host, service);

    sess.on_connect([&sess, &uri](tcp::resolver::iterator endpoint_it) {
      boost::system::error_code ec;
      auto req = sess.submit(ec, "GET", uri);

      if (ec) {
        std::cerr << "error: " << ec.message() << std::endl;
        return;
      }

      req->on_response([](const response &res) {
        std::cerr << "HTTP/2 " << res.status_code() << std::endl;
        for (auto &kv : res.header()) {
          std::cerr << kv.first << ": " << kv.second.value << "\n";
        }
        std::cerr << std::endl;

        res.on_data([](const uint8_t *data, std::size_t len) {
          std::cerr.write(reinterpret_cast<const char *>(data), len);
          std::cerr << std::endl;
        });
      });

      req->on_close([&sess](uint32_t error_code) { sess.shutdown(); });
    });

    sess.on_error([](const boost::system::error_code &ec) {
      std::cerr << "error: " << ec.message() << std::endl;
    });

    io_service.run();
  } catch (std::exception &e) {
    std::cerr << "exception: " << e.what() << "\n";
  }

  return 0;
}