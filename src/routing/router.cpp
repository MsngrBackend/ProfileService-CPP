#include "profile_service/routing/router.hpp"
#include "profile_service/utils/logger.hpp"
#include "profile_service/api_spec.hpp"

namespace msngr::profile {

RequestRouter::RequestRouter(
    const std::unordered_map<HandlerId, std::unique_ptr<IService>>& services,
    AuthService* authService
) : m_services(services), m_authService(authService) {}

void RequestRouter::BuildRoutes() {
    const auto& spec = GetApiSpec();

    for (const auto& [key, info] : spec.FlatRoutes) {
        size_t colonPos = key.find(':');
        if (colonPos != std::string::npos) {
            std::string path = key.substr(0, colonPos);
            m_routeMap[path][info.Method] = {info.Id, info.Scope};
        }
    }

    LOG(info) << "Built " << m_routeMap.size() << " route patterns";
}

HttpResponse RequestRouter::Route(const HttpRequest& request) {
    auto method = request.method();
    auto target = std::string(request.target());

    if (method == boost::beast::http::verb::options) {
        return handlers::JsonResponse(204, request.version(), "");
    }

    auto routeInfo = FindRoute(target, method);
    if (!routeInfo) {
        LOG(error) << "Route not found: " << request.method_string() << " " << target;
        return handlers::ErrorResponse(404, request.version(), "route_not_found");
    }

    bool isInternalCreate = IsInternalCreateProfile(request);

    std::optional<std::string> userId;
    if (!isInternalCreate) {
        userId = ExtractUserID(request);
        if (!userId) {
            LOG(error) << "Missing X-User-ID header";
            return handlers::ErrorResponse(401, request.version(), "missing_x_user_id");
        }

        if (!Authorize(*userId, routeInfo->RequiredScope)) {
            LOG(error) << "Auth failed: " << request.method_string() << " " << target
                       << " scope: " << routeInfo->RequiredScope;
            return handlers::ErrorResponse(403, request.version(), "auth_failed");
        }
    }

    // Use Id instead of HandlerId
    auto it = m_services.find(routeInfo->Id);
    if (it == m_services.end()) {
        LOG(error) << "Service not found for handler: " << static_cast<int>(routeInfo->Id);
        return handlers::ErrorResponse(501, request.version(), "service_not_implemented");
    }

    ServiceContext ctx;
    if (userId) {
        ctx.UserID = *userId;
    }
    ctx.PathParams = ExtractPathParams(target);

    try {
        return it->second->Execute(request, ctx);
    } catch (const std::exception& e) {
        LOG(error) << "Service execution error: " << e.what();
        return handlers::ErrorResponse(500, request.version(), "internal_server_error");
    }
}

std::optional<RequestRouter::RouteInfo> RequestRouter::FindRoute(
    const std::string& target,
    boost::beast::http::verb method) const
{
    auto findExact = [&](const std::string& path) -> std::optional<RouteInfo> {
        auto it = m_routeMap.find(path);
        if (it == m_routeMap.end()) {
            return std::nullopt;
        }

        auto handlerIt = it->second.find(method);
        if (handlerIt == it->second.end()) {
            return std::nullopt;
        }

        return handlerIt->second;
    };

    if (auto exact = findExact(target)) {
        return exact;
    }

    if (target.rfind("/contacts/", 0) == 0 && target != "/contacts") {
        return findExact("/contacts/{contact_id}");
    }

    if (target.rfind("/favorites/", 0) == 0) {
        return findExact("/favorites/{chat_id}");
    }

    if (target.rfind("/notifications/", 0) == 0 && target != "/notifications") {
        return findExact("/notifications/{chat_id}");
    }

    if (target != "/me" && target != "/me/avatar" && target != "/me/privacy" &&
        target.rfind("/", 0) == 0 && target.find('/', 1) == std::string::npos) {
        return findExact("/{user_id}");
    }

    return std::nullopt;
}

std::optional<std::string> RequestRouter::ExtractUserID(const HttpRequest& request) const {
    auto it = request.find("X-User-ID");
    if (it == request.end()) {
        return std::nullopt;
    }
    return std::string(it->value());
}

std::unordered_map<std::string, std::string> RequestRouter::ExtractPathParams(
    const std::string& target) const
{
    std::unordered_map<std::string, std::string> params;

    if (target.find("/contacts/") == 0 && target != "/contacts") {
        params["contact_id"] = target.substr(10);
    } else if (target.find("/favorites/") == 0) {
        params["chat_id"] = target.substr(11);
    } else if (target.find("/notifications/") == 0 && target != "/notifications") {
        params["chat_id"] = target.substr(15);
    } else if (target != "/me" && target.find("/") == 0 && target.length() > 1) {
        if (target != "/me/avatar" && target != "/me/privacy") {
            params["user_id"] = target.substr(1);
        }
    }

    return params;
}

bool RequestRouter::IsInternalCreateProfile(const HttpRequest& request) const {
    return request.method() == boost::beast::http::verb::post &&
           std::string(request.target()) == "/internal/profiles";
}

bool RequestRouter::Authorize(const std::string& userId, const std::string& scope) const {
    (void)scope;
    return m_authService->Check(userId, scope);
}

} // namespace msngr::profile