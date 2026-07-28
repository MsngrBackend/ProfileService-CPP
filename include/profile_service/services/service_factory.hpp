#ifndef MSNGR__PROFILE__SERVICES__SERVICE_FACTORY_HPP_
#define MSNGR__PROFILE__SERVICES__SERVICE_FACTORY_HPP_

#include "profile_service/api_spec.hpp"
#include "profile_service/services/iservice.hpp"
#include "profile_service/persistence/database_manager.hpp"
#include "profile_service/storage/storage_manager.hpp"

#include <memory>
#include <unordered_map>

namespace msngr::profile {

class ServiceFactory
{
public:
  ServiceFactory(const Repositories & repos, const Storage & storage);

  const std::unordered_map<HandlerId, std::unique_ptr<IService>> & GetServices() const {
    return m_services;
  }

private:
  void CreateProfileServices(const Repositories & repos, const Storage & storage);
  void CreateContactServices(const Repositories & repos);
  void CreatePrivacyServices(const Repositories & repos);
  void CreateFavoriteServices(const Repositories & repos);
  void CreateNotificationServices(const Repositories & repos);

  std::unordered_map<HandlerId, std::unique_ptr<IService>> m_services;
};

} // namespace msngr::profile

#endif // MSNGR__PROFILE__SERVICES__SERVICE_FACTORY_HPP_