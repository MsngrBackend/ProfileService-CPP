#include "profile_service/server.hpp"
#include "profile_service/auth_validator.hpp"

#include "profile_service/utils/logger.hpp"

#include "api_spec.hpp"
#include "handlers/handler_factory.hpp"
#include "profile_service/repository/profileRepository.hpp"
#include "profile_service/repository/contactsRepository.hpp"
#include "profile_service/repository/privacyRepository.hpp"
#include "profile_service/repository/favoritesRepository.hpp"
#include "profile_service/repository/notificationsRepository.hpp"
#include "profile_service/repository/minio_storage.hpp"
#include "profile_service/repository/query_executor.hpp"

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
  Impl(const Endpoint& endpoint)
  : m_endpoint(endpoint)
  {
    const char* db_url = std::getenv("DATABASE_URL");
    if (!db_url) {
      throw std::runtime_error("DATABASE_URL environment variable not set");
    }

    // Create database connection using the new interface
    auto connection = std::make_unique<repository::PostgresConnection>(db_url);
    auto queryExecutor = std::make_shared<repository::QueryExecutor>(std::move(connection));

    // Initialize repositories
    m_profileRepo = std::make_shared<repository::ProfileRepository>(queryExecutor);
    m_contactsRepo = std::make_shared<repository::ContactsRepository>(queryExecutor);
    m_privacyRepo = std::make_shared<repository::PrivacyRepository>(queryExecutor);
    m_favoriteRepo = std::make_shared<repository::FavoriteRepository>(queryExecutor);
    m_notificationRepo = std::make_shared<repository::NotificationRepository>(queryExecutor);

    // Initialize MinIO storage (from env)
    auto minioEndpoint = std::getenv("MINIO_ENDPOINT") ? std::getenv("MINIO_ENDPOINT") : "localhost:9000";
    auto minioAccess = std::getenv("MINIO_ACCESS_KEY") ? std::getenv("MINIO_ACCESS_KEY") : "";
    auto minioSecret = std::getenv("MINIO_SECRET_KEY") ? std::getenv("MINIO_SECRET_KEY") : "";
    m_avatarStorage = std::make_shared<repository::MinIOStorage>(minioEndpoint, minioAccess, minioSecret);

    // Initialize handler factory
    m_handlerFactory = std::make_shared<handlers::HandlerFactory>(
      m_profileRepo, m_contactsRepo, m_privacyRepo,
      m_favoriteRepo, m_notificationRepo, m_avatarStorage
    );

    m_authChecker = std::make_shared<SimpleAuthChecker>();

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
    const auto& handlerInfo = *route_info;

    std::string userID = ExtractUserID(request);
    const bool is_internalCreateProfile =
      method == beast_http::verb::post && target == "/internal/profiles";

    if (!is_internalCreateProfile) {
      if (userID.empty()) {
        LOG(error) << "Missing X-User-ID header";
        return handlers::ErrorResponse(401, request.version(), "missing X-User-ID");
      }

      if (!m_authChecker->Check(userID, handlerInfo.scope)) {
        LOG(error) << "Auth failed: " << request.method_string() << " " << target;
        return handlers::ErrorResponse(403, request.version(), "auth_failed");
      }
    }

    handlers::RouteContext ctx;
    ctx.UserID = userID;

    if (target.find("/contacts/") == 0 && target != "/contacts") {
      ctx.PathParams["contact_id"] = target.substr(10);
    } else if (target.find("/favorites/") == 0) {
      ctx.PathParams["chat_id"] = target.substr(11);
    } else if (target.find("/notifications/") == 0 && target != "/notifications") {
      ctx.PathParams["chat_id"] = target.substr(15);
    } else if (target != "/me" && target.find("/") == 0 && target.length() > 1) {
      ctx.PathParams["user_id"] = target.substr(1);
    }

    auto handler = m_handlerFactory->Bind(handlerInfo.id);
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
  std::shared_ptr<repository::IDatabaseConnection> m_dbConn;

  std::shared_ptr<repository::ProfileRepository> m_profileRepo;
  std::shared_ptr<repository::ContactsRepository> m_contactsRepo;
  std::shared_ptr<repository::PrivacyRepository> m_privacyRepo;
  std::shared_ptr<repository::FavoriteRepository> m_favoriteRepo;
  std::shared_ptr<repository::NotificationRepository> m_notificationRepo;
  std::shared_ptr<repository::MinIOStorage> m_avatarStorage;

  std::shared_ptr<handlers::HandlerFactory> m_handlerFactory;
  std::shared_ptr<IAuthChecker> m_authChecker;

  std::unordered_map<std::string, std::unordered_map<beast_http::verb, HandlerInfo>> m_routeMap;
};

Server::Server(Endpoint endpoint)
  : m_endpoint(std::move(endpoint)) {
  m_impl = std::make_unique<Impl>(m_endpoint);
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
