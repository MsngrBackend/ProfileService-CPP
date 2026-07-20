#include "profile_service/request_manager.hpp"
#include "profile_service/query_manager.hpp"
#include "profile_service/api_spec.hpp"
#include "profile_service/auth_validator.hpp"
#include "profile_service/handlers/handler_factory.hpp"
#include "profile_service/utils/logger.hpp"

#include <boost/beast/http.hpp>
#include <optional>
#include <string>

namespace msngr::profile {

RequestManager::RequestManager(
  std::shared_ptr<QueryManager> queryManager,
  std::shared_ptr<handlers::HandlerFactory> handlerFactory,
  std::shared_ptr<IAuthChecker> authChecker
)
  : m_queryManager(std::move(queryManager)),
    m_handlerFactory(std::move(handlerFactory)),
    m_authChecker(std::move(authChecker))
{
  BuildRouteMap();
}

beast_http::response<beast_http::string_body> RequestManager::Handle(
  const beast_http::request<beast_http::string_body> & request)
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

    if (!AuthorizeRequest(target, method, userID, handlerInfo)) {
      LOG(error) << "Auth failed: " << request.method_string() << " " << target;
      return handlers::ErrorResponse(403, request.version(), "auth_failed");
    }
  }

  handlers::RouteContext ctx;
  ctx.UserID = userID;
  ExtractPathParams(target, ctx);

  auto handler = m_handlerFactory->Bind(handlerInfo.Id);
  return handler(request, ctx);
}

std::optional<HandlerInfo> RequestManager::ResolveRoute(
  const std::string & target,
  beast_http::verb method) const
{
  auto findExact = [&](const std::string & path) -> std::optional<HandlerInfo> {
    auto it = m_routeMap.find(path);
    if (it == m_routeMap.end()) {
      return std::nullopt;
    }

    // TODO: use std::get
    auto handlerIt = it->second.find(method);
    if (handlerIt == it->second.end()) {
      return std::nullopt;
    }

    return handlerIt->second;
  };

  if (auto exact = findExact(target); exact.has_value()) {
    return exact;
  }

  if (target.rfind("/contacts/", 0) == 0) {
    return findExact("/contacts/{contact_id}");
  }

  if (target.rfind("/favorites/", 0) == 0) {
    return findExact("/favorites/{chat_id}");
  }

  if (target.rfind("/notifications/", 0) == 0) {
    return findExact("/notifications/{chat_id}");
  }

  if (target != "/me" && target != "/me/avatar" && target != "/me/privacy" &&
      target.rfind("/", 0) == 0 && target.find('/', 1) == std::string::npos) {
    return findExact("/{user_id}");
  }

  return std::nullopt;
}

void RequestManager::BuildRouteMap() {
  const auto& spec = GetApiSpec();

  for (const auto& [key, info] : spec.FlatRoutes) {
    size_t colonPos = key.find(':');
    if (colonPos != std::string::npos) {
      std::string path = key.substr(0, colonPos);
      m_routeMap[path][info.Method] = info;
    }
  }
}

std::string RequestManager::ExtractUserID(
  const beast_http::request<beast_http::string_body> & request) const {
  auto userIdHeader = request.find("X-User-ID");
  if (userIdHeader == request.end()) {
    return "";
  }
  return std::string(userIdHeader->value());
}

void RequestManager::ExtractPathParams(
  const std::string& target,
  handlers::RouteContext& ctx) const {
  if (target.find("/contacts/") == 0 && target != "/contacts") {
    ctx.PathParams["contact_id"] = target.substr(10);
  } else if (target.find("/favorites/") == 0) {
    ctx.PathParams["chat_id"] = target.substr(11);
  } else if (target.find("/notifications/") == 0 && target != "/notifications") {
    ctx.PathParams["chat_id"] = target.substr(15);
  } else if (target != "/me" && target.find("/") == 0 && target.length() > 1) {
    ctx.PathParams["user_id"] = target.substr(1);
  }
}

bool RequestManager::AuthorizeRequest(
  const std::string & target,
  beast_http::verb method,
  const std::string & userID,
  const HandlerInfo & handlerInfo) const {
  return m_authChecker->Check(userID, handlerInfo.Scope);
}

} // namespace msngr::profile
