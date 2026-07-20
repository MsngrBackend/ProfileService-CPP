#include "profile_service/server.hpp"
#include "profile_service/server_impl.hpp"
#include "profile_service/utils/logger.hpp"

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

namespace beast = boost::beast;
namespace beast_http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

namespace msngr::profile {

Server::Server(Endpoint endpoint)
  : m_endpoint(std::move(endpoint)) {
  m_impl = std::make_unique<ServerImpl>(m_endpoint);
}

Server::~Server() {
  Stop();
}

bool Server::Start() {
  if (m_running.exchange(true)) {
    LOG(error) << "Server::Start called while already running";
    return false;
  }

  m_thread = std::jthread([this](std::stop_token stopToken) {
    try {
      net::io_context ioc{1};
      tcp::acceptor acceptor{ioc};
      auto endpoint = tcp::endpoint{
        net::ip::make_address(m_endpoint.Address),
        m_endpoint.Port
      };

      acceptor.open(endpoint.protocol());
      acceptor.set_option(net::socket_base::reuse_address(true));
      acceptor.bind(endpoint);
      acceptor.listen();

      LOG(info) << "Profile service running on " << m_endpoint.Address << ":" << m_endpoint.Port;

      while (m_running.load() && !stopToken.stop_requested()) {
        tcp::socket socket{ioc};
        beast::error_code ec;
        acceptor.accept(socket, ec);
        if (ec) {
          if (m_running.load()) {
            LOG(error) << "Accept error: " << ec.message();
          }
          continue;
        }

        beast::flat_buffer buffer;
        beast_http::request<beast_http::string_body> request;
        beast_http::read(socket, buffer, request, ec);
        if (ec) {
          if (m_running.load() && !(beast_http::error::end_of_stream == ec)) {
            LOG(error) << "Read error: " << ec.message();
          }
          socket.shutdown(tcp::socket::shutdown_both, ec);
          continue;
        }

        auto response = m_impl->Handle(request);
        response.set(beast_http::field::server, "profile-service");
        response.set(beast_http::field::access_control_allow_origin, "*");
        response.set(beast_http::field::access_control_allow_methods, "GET, POST, PUT, PATCH, DELETE, OPTIONS");
        response.set(beast_http::field::access_control_allow_headers, "Content-Type, Authorization, X-User-ID");
        response.keep_alive(false);

        beast_http::write(socket, response, ec);
        socket.shutdown(tcp::socket::shutdown_both, ec);
      }
    } catch (const std::exception& e) {
      LOG(error) << "Server failed: " << e.what();
      m_running.store(false);
    }
  });

  return true;
}

void Server::Stop() {
  if (!m_running.exchange(false)) {
    return;
  }

  m_thread.request_stop();

  // Send a connection to unblock accept
  try {
    net::io_context ioc{1};
    tcp::resolver resolver{ioc};
    auto endpoints = resolver.resolve(m_endpoint.Address, std::to_string(m_endpoint.Port));
    tcp::socket socket{ioc};
    net::connect(socket, endpoints);
    beast::error_code ec;
    socket.shutdown(tcp::socket::shutdown_both, ec);
  } catch (...) {
  }

  if (m_thread.joinable()) {
    m_thread.join();
  }

  LOG(info) << "Profile service stopped";
}

} // namespace msngr::profile
