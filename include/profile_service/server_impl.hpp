#ifndef MSNGR__PROFILE__SERVER_IMPL_HPP_
#define MSNGR__PROFILE__SERVER_IMPL_HPP_

#include "profile_service/server.hpp"
#include <boost/beast/http.hpp>
#include <memory>
#include <string>

namespace msngr::profile {

class RequestManager;
class QueryManager;
class IAuthChecker;

namespace handlers {
  class HandlerFactory;
}

namespace beast_http = boost::beast::http;

class ServerImpl {
public:
  explicit ServerImpl(const Endpoint & endpoint);
  ~ServerImpl();

  beast_http::response<beast_http::string_body> Handle(
    const beast_http::request<beast_http::string_body> & request
  );

private:
  void InitializeComponents();

  Endpoint m_endpoint;
  std::shared_ptr<QueryManager> m_queryManager;
  std::shared_ptr<handlers::HandlerFactory> m_handlerFactory;
  std::shared_ptr<IAuthChecker> m_authChecker;
  std::shared_ptr<RequestManager> m_requestManager;
};

} // namespace msngr::profile

#endif // MSNGR__PROFILE__SERVER_IMPL_HPP_
