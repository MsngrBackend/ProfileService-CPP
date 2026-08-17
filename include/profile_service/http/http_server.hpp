#ifndef MSNGR__PROFILE__HTTP__HTTP_SERVER_HPP_
#define MSNGR__PROFILE__HTTP__HTTP_SERVER_HPP_

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <stop_token>

namespace msngr::profile {

using HttpRequest = boost::beast::http::request<boost::beast::http::string_body>;
using HttpResponse = boost::beast::http::response<boost::beast::http::string_body>;
using RequestHandler = std::function<HttpResponse(const HttpRequest&)>;

struct TlsConfig {
  std::string CertificateFile;
  std::string PrivateKeyFile;
  std::string DhParamFile;
  bool Enabled = false;
};

class HttpServer {
public:
  HttpServer(std::string address, uint16_t port, RequestHandler handler, TlsConfig tlsConfig = {});
  ~HttpServer();

  HttpServer(const HttpServer &) = delete;
  HttpServer& operator=(const HttpServer &) = delete;

  void Run(std::stop_token stopToken);
  void Stop();

  inline bool IsRunning() const
  {
    return m_running.load();
  }

private:
  void HandleRequest(boost::asio::ip::tcp::socket & socket);
  void HandleRequest(boost::asio::ssl::stream<boost::asio::ip::tcp::socket>& stream);
  void SendResponse(boost::asio::ip::tcp::socket & socket, const HttpResponse & response);
  void SendResponse(boost::asio::ssl::stream<boost::asio::ip::tcp::socket>& stream, const HttpResponse & response);

  std::string m_address;
  uint16_t m_port;
  RequestHandler m_requestHandler;
  TlsConfig m_tlsConfig;
  std::atomic<bool> m_running{true};
  std::atomic<bool> m_stopRequested{false};
};

} // namespace msngr::profile

#endif // MSNGR__PROFILE__HTTP__HTTP_SERVER_HPP_