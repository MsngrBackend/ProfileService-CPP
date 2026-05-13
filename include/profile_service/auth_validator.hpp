#ifndef MSNGR__PROFILE__AUTH_VALIDATOR_HPP_
#define MSNGR__PROFILE__AUTH_VALIDATOR_HPP_

#include <boost/beast/http.hpp>
#include <string>
#include <functional>

namespace beast_http = boost::beast::http;

namespace msngr::profile{

class IAuthChecker {
public:
  virtual ~IAuthChecker() = default;
  virtual bool Check(const std::string& user_id, const std::string& scope) = 0;
};

class SimpleAuthChecker : public IAuthChecker {
public:
  bool Check(const std::string & user_id, const std::string & scope) override {
    return !user_id.empty();
  }
};

inline std::string ExtractUserID(const beast_http::request<beast_http::string_body>& request) {
  auto it = request.find("X-User-ID");
  if (it != request.end()) {
    return std::string(it->value());
  }
  return "";
}

} // namespace msngr::profile

#endif  // MSNGR__PROFILE__AUTH_VALIDATOR_HPP_
