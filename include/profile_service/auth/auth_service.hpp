#ifndef MSNGR__PROFILE__AUTH__AUTH_SERVICE_HPP_
#define MSNGR__PROFILE__AUTH__AUTH_SERVICE_HPP_

#include <string>

namespace msngr::profile {

class AuthService {
public:
  virtual ~AuthService() = default;

  virtual bool Check(const std::string & userId, const std::string & scope) const;
};

} // namespace msngr::profile

#endif // MSNGR__PROFILE__AUTH__AUTH_SERVICE_HPP_