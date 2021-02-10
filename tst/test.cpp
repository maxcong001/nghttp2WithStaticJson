/*
#include "clientWrapper.hpp"

int main()
{
  Http2Client client;
  // int connect(const string & addr, const string & port) {
  auto sessionID = client.connect("127.0.0.1", "1234");
  std::cout << "session id is " << sessionID << std::endl;
  //int send(int sess_id, string method, string uri, string data)
  int a = 0;
  while (cin >> a)
  {
    client.send(sessionID, "GET", "", "{\"test\":\"testsss\"}");
  }
}
*/
#include <chrono>
#include <thread>
#include <iostream>

#include <nghttp2/asio_http2_client.h>

using boost::asio::ip::tcp;

using namespace nghttp2::asio_http2;
using namespace nghttp2::asio_http2::client;
int i = 0;
int main(int argc, char *argv[])
{
  // vector<session> sessions;
  try
  {
    /*
    if (argc < 2)
    {
      std::cerr << "Usage:  URI" << std::endl;
      return 1;
    }*/
    boost::system::error_code ec;
    boost::asio::io_service io_service;
    boost::asio::io_service::work work(io_service);

    std::string uri = "http://192.168.30.171:1234"; //argv[1];
    std::string scheme, host, service;

    if (host_service_from_uri(ec, scheme, host, service, uri))
    {
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
    int i = 0;
    bool flag = false;
    std::thread th([&flag, &sess, &i, &io_service, &uri, &scheme, &tls_ctx, &host, &service]() {
      while (1)
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        if (flag)
        {
          i++;
          for (int j = 0; j < 1; j++)
          {

            io_service.post([&i, &sess, &io_service, &uri]() {
              boost::system::error_code ec;
              std::cout << "now try to send " << i << "st message" << std::endl;
              auto req = sess.submit(ec, "GET", uri, R"({"test":"teststr"})");
              if (req)
              {
                req->on_response([&i, &sess, &uri, &io_service](const response &res) {
                  i++;

                  std::cout << "message id " << i << std::endl;
                  std::cerr << "HTTP/2 " << res.status_code() << std::endl;
                  for (auto &kv : res.header())
                  {
                    std::cerr << kv.first << ": " << kv.second.value << "\n";
                  }
                  std::cerr << std::endl;

                  res.on_data([](const uint8_t *data, std::size_t len) {
                    std::cerr.write(reinterpret_cast<const char *>(data), len);
                    std::cerr << std::endl;
                  });
                });
                req->on_close([&sess](uint32_t error_code) { //sess.shutdown();
                  std::cout << " req->on_close is called " << std::endl;
                });
              }
            });
          }
        }
      }
    });
    sess.on_connect([&flag, &i, &io_service, &sess, &uri](tcp::resolver::iterator endpoint_it) {
      std::cout << "connected" << std::endl;
      flag = true;
    });

    sess.on_error([](const boost::system::error_code &ec) {
      std::cerr << "error: " << ec.message() << std::endl;
    });

    io_service.run();
    th.join();
  }
  catch (std::exception &e)
  {
    std::cerr << "exception: " << e.what() << "\n";
  }

  return 0;
}
