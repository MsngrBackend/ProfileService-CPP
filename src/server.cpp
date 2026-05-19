#include "profile_service/server.hpp"
#include "profile_service/auth_validator.hpp"

#include "profile_service/utils/logger.hpp"

#include "api_spec.hpp"
#include "handlers/handler_factory.hpp"
#include "repository/profile_repository.hpp"
#include "repository/contacts_repository.hpp"
#include "repository/privacy_repository.hpp"
#include "repository/favorites_repository.hpp"
#include "repository/notifications_repository.hpp"
#include "repository/minio_storage.hpp"


#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <nlohmann/json.hpp>

#include <cstdlib>
#include <optional>
#include <pqxx/pqxx>

namespace beast = boost::beast;
namespace beast_http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

namespace msngr::profile {

class Server::Impl {
public:
  Impl(std::shared_ptr<pqxx::connection> db_conn, const Endpoint& endpoint)
  : m_endpoint(endpoint),
    m_db_conn(std::move(db_conn))
  {
    // Initialize repositories
    m_profile_repo = std::make_shared<repository::ProfileRepositoryPostgres>(m_db_conn);
    m_contacts_repo = std::make_shared<repository::ContactsRepositoryPostgres>(m_db_conn);
    m_privacy_repo = std::make_shared<repository::PrivacyRepositoryPostgres>(m_db_conn);
    m_favorite_repo = std::make_shared<repository::FavoriteRepositoryPostgres>(m_db_conn);
    m_notification_repo = std::make_shared<repository::NotificationRepositoryPostgres>(m_db_conn);

    // Initialize MinIO storage (from env)
    auto minio_endpoint = std::getenv("MINIO_ENDPOINT") ? std::getenv("MINIO_ENDPOINT") : "localhost:9000";
    auto minio_access = std::getenv("MINIO_ACCESS_KEY") ? std::getenv("MINIO_ACCESS_KEY") : "";
    auto minio_secret = std::getenv("MINIO_SECRET_KEY") ? std::getenv("MINIO_SECRET_KEY") : "";
    m_avatar_storage = std::make_shared<repository::MinIOStorage>(minio_endpoint, minio_access, minio_secret);

    // Initialize handler factory
    m_handler_factory = std::make_shared<handlers::HandlerFactory>(
      m_profile_repo, m_contacts_repo, m_privacy_repo,
      m_favorite_repo, m_notification_repo, m_avatar_storage
    );

    m_auth_checker = std::make_shared<SimpleAuthChecker>();

    BuildRouteMap();
  }

  beast_http::response<beast_http::string_body> Handle(
    const beast_http::request<beast_http::string_body>& request) 
  {
    auto method = request.method();
    auto target = std::string(request.target());

    if (method == beast_http::verb::options) {
      return handlers::JsonResponse(204, request.version(), "");
    }

    auto route_info = ResolveRoute(target, method);
    if (!route_info.has_value()) {
      LOG(error) << "Route not found: " << request.method_string() << " " << target;
      return handlers::ErrorResponse(404, request.version(), "route_not_found");
    }
    const auto& handler_info = *route_info;

    std::string user_id = ExtractUserID(request);
    const bool is_internal_create_profile =
      method == beast_http::verb::post && target == "/internal/profiles";

    if (!is_internal_create_profile) {
      if (user_id.empty()) {
        LOG(error) << "Missing X-User-ID header";
        return handlers::ErrorResponse(401, request.version(), "missing X-User-ID");
      }

      if (!m_auth_checker->Check(user_id, handler_info.scope)) {
        LOG(error) << "Auth failed: " << request.method_string() << " " << target;
        return handlers::ErrorResponse(403, request.version(), "auth_failed");
      }
    }

    handlers::RouteContext ctx;
    ctx.UserID = user_id;

    if (target.find("/contacts/") == 0 && target != "/contacts") {
      ctx.PathParams["contact_id"] = target.substr(10);
    } else if (target.find("/favorites/") == 0) {
      ctx.PathParams["chat_id"] = target.substr(11);
    } else if (target.find("/notifications/") == 0 && target != "/notifications") {
      ctx.PathParams["chat_id"] = target.substr(15);
    } else if (target != "/me" && target.find("/") == 0 && target.length() > 1) {
      ctx.PathParams["user_id"] = target.substr(1);
    }

    auto handler = m_handler_factory->Bind(handler_info.id);
    return handler(request, ctx);
  }

private:
  std::optional<HandlerInfo> ResolveRoute(const std::string& target, beast_http::verb method) const {
    auto find_exact = [&](const std::string& path) -> std::optional<HandlerInfo> {
      auto it = m_route_map.find(path);
      if (it == m_route_map.end()) {
        return std::nullopt;
      }

      auto handler_it = it->second.find(method);
      if (handler_it == it->second.end()) {
        return std::nullopt;
      }

      return handler_it->second;
    };

    if (auto exact = find_exact(target); exact.has_value()) {
      return exact;
    }

    if (target.rfind("/contacts/", 0) == 0) {
      return find_exact("/contacts/{contact_id}");
    }

    if (target.rfind("/favorites/", 0) == 0) {
      return find_exact("/favorites/{chat_id}");
    }

    if (target.rfind("/notifications/", 0) == 0) {
      return find_exact("/notifications/{chat_id}");
    }

    if (target != "/me" && target != "/me/avatar" && target != "/me/privacy" &&
        target.rfind("/", 0) == 0 && target.find('/', 1) == std::string::npos) {
      return find_exact("/{user_id}");
    }

    return std::nullopt;
  }

  void BuildRouteMap() {
    const auto & spec = GetApiSpec();

    for (const auto & [key, info] : spec.flatRoutes) {
      // Extract path from key (simplified)
      size_t colon_pos = key.find(':');
      if (colon_pos != std::string::npos) {
        std::string path = key.substr(0, colon_pos);
        m_route_map[path][info.method] = info;
      }
    }
  }

  Endpoint m_endpoint;
  std::shared_ptr<pqxx::connection> m_db_conn;

  std::shared_ptr<repository::ProfileRepositoryPostgres> m_profile_repo;
  std::shared_ptr<repository::ContactsRepositoryPostgres> m_contacts_repo;
  std::shared_ptr<repository::PrivacyRepositoryPostgres> m_privacy_repo;
  std::shared_ptr<repository::FavoriteRepositoryPostgres> m_favorite_repo;
  std::shared_ptr<repository::NotificationRepositoryPostgres> m_notification_repo;
  std::shared_ptr<repository::MinIOStorage> m_avatar_storage;

  std::shared_ptr<handlers::HandlerFactory> m_handler_factory;
  std::shared_ptr<IAuthChecker> m_auth_checker;

  std::unordered_map<std::string, std::unordered_map<beast_http::verb, HandlerInfo>> m_route_map;
};

Server::Server(std::shared_ptr<void> repository, Endpoint endpoint)
  : m_repository(std::move(repository)), m_endpoint(std::move(endpoint)) {
  auto db_conn = std::static_pointer_cast<pqxx::connection>(m_repository);
  m_impl = std::make_unique<Impl>(db_conn, m_endpoint);
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
