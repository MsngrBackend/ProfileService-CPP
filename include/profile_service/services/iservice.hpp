#ifndef MSNGR__PROFILE__SERVICES__ISERVICE_HPP_
#define MSNGR__PROFILE__SERVICES__ISERVICE_HPP_

#include "profile_service/http/http_server.hpp"
#include <string>
#include <unordered_map>

namespace msngr::profile {

struct ServiceContext {
  std::unordered_map<std::string, std::string> PathParams;
  std::string UserID;
};

class IService {
public:
  virtual ~IService() = default;

  virtual HttpResponse Execute(const HttpRequest & request, const ServiceContext & context) = 0;
};

// Helper response functions
namespace handlers {

inline HttpResponse JsonResponse(int status, unsigned version, const std::string & body)
{
  HttpResponse response{static_cast<boost::beast::http::status>(status), version};
  response.set(boost::beast::http::field::content_type, "application/json");
  response.body() = body;
  response.prepare_payload();
  response.keep_alive(false);
  return response;
}

inline HttpResponse ErrorResponse(int status, unsigned version, const std::string & error)
{
  return JsonResponse(status, version, "{\"error\":\"" + error + "\"}");
}

} // namespace handlers

} // namespace msngr::profile

#endif // MSNGR__PROFILE__SERVICES__ISERVICE_HPP_