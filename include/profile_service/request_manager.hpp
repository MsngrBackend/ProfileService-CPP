#ifndef MSNGR__PROFILE__REQUEST_MANAGER_HPP_
#define MSNGR__PROFILE__REQUEST_MANAGER_HPP_

#include <boost/beast/http.hpp>
#include <memory>
#include <string>
#include <optional>
#include <unordered_map>

namespace msngr::profile {

struct HandlerInfo;
class QueryManager;
class IAuthChecker;

namespace handlers {
  class HandlerFactory;
  struct RouteContext;
}

namespace beast_http = boost::beast::http;

class RequestManager {
public:
  explicit RequestManager(
    std::shared_ptr<QueryManager> queryManager,
    std::shared_ptr<handlers::HandlerFactory> handlerFactory,
    std::shared_ptr<IAuthChecker> authChecker
  );

  beast_http::response<beast_http::string_body> Handle(
    const beast_http::request<beast_http::string_body> & request
  );

private:
  std::optional<HandlerInfo> ResolveRoute(
    const std::string& target,
    beast_http::verb method
  ) const;

  void BuildRouteMap();
  std::string ExtractUserID(const beast_http::request<beast_http::string_body> & request) const;
  void ExtractPathParams(const std::string & target, handlers::RouteContext & ctx) const;
  bool AuthorizeRequest(
    const std::string& target,
    beast_http::verb method,
    const std::string & userID,
    const HandlerInfo & handlerInfo
  ) const;

  std::shared_ptr<QueryManager> m_queryManager;
  std::shared_ptr<handlers::HandlerFactory> m_handlerFactory;
  std::shared_ptr<IAuthChecker> m_authChecker;

  std::unordered_map<std::string, std::unordered_map<beast_http::verb, HandlerInfo>> m_routeMap;
};

} // namespace msngr::profile

#endif // MSNGR__PROFILE__REQUEST_MANAGER_HPP_
