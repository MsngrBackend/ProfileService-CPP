#ifndef MSNGR__PROFILE__ROUTING__ROUTER_HPP_
#define MSNGR__PROFILE__ROUTING__ROUTER_HPP_

#include "profile_service/http/http_server.hpp"
#include "profile_service/api_spec.hpp"
#include "profile_service/auth/auth_service.hpp"
#include "profile_service/services/iservice.hpp"

#include <boost/beast/http/verb.hpp>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

namespace msngr::profile {

class RequestRouter {
public:
  RequestRouter(
    const std::unordered_map<HandlerId, std::unique_ptr<IService>> & services,
    AuthService* authService
  );

  void BuildRoutes();
  HttpResponse Route(const HttpRequest & request);

private:
  struct RouteInfo {
    HandlerId Id;
    std::string RequiredScope;
  };

  std::optional<RouteInfo> FindRoute(const std::string & target, boost::beast::http::verb method) const;
  std::optional<std::string> ExtractUserID(const HttpRequest & request) const;
  std::unordered_map<std::string, std::string> ExtractPathParams(const std::string & target) const;
  bool IsInternalCreateProfile(const HttpRequest & request) const;
  bool Authorize(const std::string & userId, const std::string & scope) const;

  const std::unordered_map<HandlerId, std::unique_ptr<IService>> & m_services;
  AuthService* m_authService;
  std::unordered_map<std::string, std::unordered_map<boost::beast::http::verb, RouteInfo>> m_routeMap;
};

} // namespace msngr::profile

#endif // MSNGR__PROFILE__ROUTING__ROUTER_HPP_