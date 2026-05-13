#ifndef MSNGR__PROFILE__HANDLERS__IHANDLER_HPP_
#define MSNGR__PROFILE__HANDLERS__IHANDLER_HPP_

#include <boost/beast/http.hpp>
#include <string>
#include <unordered_map>

namespace beast_http = boost::beast::http;

namespace msngr::profile::handlers {

struct RouteContext {
  std::unordered_map<std::string, std::string> PathParams;
  std::string UserID;
};

using HttpRequest = beast_http::request<beast_http::string_body>;
using HttpResponse = beast_http::response<beast_http::string_body>;

class IHandler {
public:
  virtual ~IHandler() = default;
  virtual HttpResponse Handle(const HttpRequest & request, const RouteContext & context) = 0;
};

inline HttpResponse JsonResponse(int status, unsigned version, const std::string & body) {
  HttpResponse response{static_cast<beast_http::status>(status), version};
  response.set(beast_http::field::content_type, "application/json");
  response.body() = body;
  response.prepare_payload();
  return response;
}

inline HttpResponse ErrorResponse(int status, unsigned version, const std::string & error) {
  return JsonResponse(status, version, "{\"error\":\"" + error + "\"}");
}

} // namespace msngr::profile::handlers

#endif  // MSNGR__PROFILE__HANDLERS__IHANDLER_HPP_