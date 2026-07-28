#ifndef MSNGR__PROFILE__SERVICES__PRIVACY_SERVICE_HPP_
#define MSNGR__PROFILE__SERVICES__PRIVACY_SERVICE_HPP_

#include "profile_service/services/iservice.hpp"
#include "profile_service/persistence/repositories/privacy_repository.hpp"

#include <memory>

namespace msngr::profile::services {

class PrivacyService : public IService {
public:
  explicit PrivacyService(std::shared_ptr<repository::PrivacyRepository> privacyRepo);

  HttpResponse Execute(const HttpRequest & request, const ServiceContext & context) override;

private:
  HttpResponse HandleGetPrivacy(const HttpRequest & request, const ServiceContext & context);
  HttpResponse HandleUpdatePrivacy(const HttpRequest & request, const ServiceContext & context);

  std::shared_ptr<repository::PrivacyRepository> m_privacyRepo;
};

} // namespace msngr::profile::services

#endif // MSNGR__PROFILE__SERVICES__PRIVACY_SERVICE_HPP_