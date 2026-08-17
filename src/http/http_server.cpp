#include "profile_service/http/http_server.hpp"
#include "profile_service/utils/logger.hpp"

namespace beast = boost::beast;
namespace net = boost::asio;
namespace ssl = net::ssl;
using tcp = net::ip::tcp;

namespace msngr::profile {

HttpServer::HttpServer(std::string address, uint16_t port, RequestHandler handler, TlsConfig tlsConfig)
  : m_address(std::move(address)), m_port(port), m_requestHandler(std::move(handler)), m_tlsConfig(std::move(tlsConfig)) {}

HttpServer::~HttpServer() {
  Stop();
}

void HttpServer::Stop() {
  m_stopRequested.store(true);
  m_running.store(false);
}

void HttpServer::Run(std::stop_token stopToken)
{
  try {
    net::io_context ioc{1};
    tcp::acceptor acceptor{ioc};
    auto endpoint = tcp::endpoint{
      net::ip::make_address(m_address),
      m_port
    };

    acceptor.open(endpoint.protocol());
    acceptor.set_option(net::socket_base::reuse_address(true));
    acceptor.bind(endpoint);
    acceptor.listen(net::socket_base::max_listen_connections);

    std::unique_ptr<ssl::context> sslCtx;
    if (m_tlsConfig.Enabled) {
      sslCtx = std::make_unique<ssl::context>(ssl::context::tlsv12_server);
      sslCtx->use_certificate_chain_file(m_tlsConfig.CertificateFile);
      sslCtx->use_private_key_file(m_tlsConfig.PrivateKeyFile, ssl::context::pem);
      if (!m_tlsConfig.DhParamFile.empty()) {
        sslCtx->use_tmp_dh_file(m_tlsConfig.DhParamFile);
      }
      LOG(info) << "HTTPS server enabled with TLS";
    }

    LOG(info) << "HTTP server listening on " << m_address << ":" << m_port;

    while (m_running.load() && !stopToken.stop_requested() && !m_stopRequested.load()) {
      tcp::socket socket{ioc};
      beast::error_code ec;

      acceptor.accept(socket, ec);
      if (ec) {
        if (m_running.load() && ec != beast::errc::operation_canceled) {
          LOG(error) << "Accept error: " << ec.message();
        }
        continue;
      }

      if (m_tlsConfig.Enabled && sslCtx) {
        ssl::stream<tcp::socket> sslStream{std::move(socket), *sslCtx};
        sslStream.handshake(ssl::stream_base::server, ec);
        if (ec) {
          LOG(error) << "SSL handshake failed: " << ec.message();
          continue;
        }
        HandleRequest(sslStream);
      } else {
        HandleRequest(socket);
      }
    }

    LOG(info) << "HTTP server stopped";
  } catch (const std::exception & e) {
    LOG(error) << "HTTP server fatal error: " << e.what();
    m_running.store(false);
  }
}

void HttpServer::HandleRequest(tcp::socket & socket)
{
  beast::flat_buffer buffer;
  HttpRequest request;
  beast::error_code ec;

  boost::beast::http::read(socket, buffer, request, ec);
  if (ec) {
    if (m_running.load() && ec != boost::beast::http::error::end_of_stream) {
      LOG(error) << "Read error: " << ec.message();
    }
    socket.shutdown(tcp::socket::shutdown_both, ec);
    return;
  }

  HttpResponse response = m_requestHandler(request);

  response.set(boost::beast::http::field::server, "profile-service");
  response.set(boost::beast::http::field::access_control_allow_origin, "*");
  response.set(boost::beast::http::field::access_control_allow_methods,
                "GET, POST, PUT, PATCH, DELETE, OPTIONS");
  response.set(boost::beast::http::field::access_control_allow_headers,
                "Content-Type, Authorization, X-User-ID");

  SendResponse(socket, response);
}

void HttpServer::HandleRequest(ssl::stream<tcp::socket>& stream)
{
  beast::flat_buffer buffer;
  HttpRequest request;
  beast::error_code ec;

  boost::beast::http::read(stream, buffer, request, ec);
  if (ec) {
    if (m_running.load() && ec != boost::beast::http::error::end_of_stream) {
      LOG(error) << "Read error: " << ec.message();
    }
    stream.shutdown(ec);
    return;
  }

  HttpResponse response = m_requestHandler(request);

  response.set(boost::beast::http::field::server, "profile-service");
  response.set(boost::beast::http::field::access_control_allow_origin, "*");
  response.set(boost::beast::http::field::access_control_allow_methods,
                "GET, POST, PUT, PATCH, DELETE, OPTIONS");
  response.set(boost::beast::http::field::access_control_allow_headers,
                "Content-Type, Authorization, X-User-ID");

  SendResponse(stream, response);
}

void HttpServer::SendResponse(tcp::socket & socket, const HttpResponse & response)
{
  beast::error_code ec;
  boost::beast::http::write(socket, response, ec);

  if (ec) {
    LOG(error) << "Write error: " << ec.message();
  }

  socket.shutdown(tcp::socket::shutdown_both, ec);
}

void HttpServer::SendResponse(ssl::stream<tcp::socket>& stream, const HttpResponse & response)
{
  beast::error_code ec;
  boost::beast::http::write(stream, response, ec);

  if (ec) {
    LOG(error) << "Write error: " << ec.message();
  }

  stream.shutdown(ec);
}

} // namespace msngr::profile