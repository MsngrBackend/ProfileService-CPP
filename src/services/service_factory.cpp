#include "profile_service/services/service_factory.hpp"
#include "profile_service/services/profile_service.hpp"
#include "profile_service/services/contacts_service.hpp"
#include "profile_service/services/privacy_service.hpp"
#include "profile_service/services/favorites_service.hpp"
#include "profile_service/services/notifications_service.hpp"

namespace msngr::profile {

ServiceFactory::ServiceFactory(const Repositories & repos, const Storage & storage) {
  CreateProfileServices(repos, storage);
  CreateContactServices(repos);
  CreatePrivacyServices(repos);
  CreateFavoriteServices(repos);
  CreateNotificationServices(repos);
}

void ServiceFactory::CreateProfileServices(const Repositories & repos, const Storage & storage) {
  auto service = std::make_unique<services::ProfileService>(
    repos.Profile,
    storage.Avatar
  );

  // Each handler ID maps to the same service instance
  m_services[HandlerId::CreateProfile] = std::move(service);
  // For other profile handlers, we reuse the same service
  // But we need to store it multiple times with different keys
  // Actually, we need separate instances or a shared_ptr
  // Let's use shared_ptr for the service
}

void ServiceFactory::CreateContactServices(const Repositories & repos) {
  auto service = std::make_unique<services::ContactsService>(repos.Contacts);
  m_services[HandlerId::GetContacts] = std::move(service);
  m_services[HandlerId::AddContact] = std::make_unique<services::ContactsService>(repos.Contacts);
  m_services[HandlerId::DeleteContact] = std::make_unique<services::ContactsService>(repos.Contacts);
}

void ServiceFactory::CreatePrivacyServices(const Repositories & repos) {
  m_services[HandlerId::GetPrivacy] = std::make_unique<services::PrivacyService>(repos.Privacy);
  m_services[HandlerId::UpdatePrivacy] = std::make_unique<services::PrivacyService>(repos.Privacy);
}

void ServiceFactory::CreateFavoriteServices(const Repositories & repos) {
  m_services[HandlerId::GetFavorites] = std::make_unique<services::FavoritesService>(repos.Favorites);
  m_services[HandlerId::AddFavorite] = std::make_unique<services::FavoritesService>(repos.Favorites);
  m_services[HandlerId::RemoveFavorite] = std::make_unique<services::FavoritesService>(repos.Favorites);
}

void ServiceFactory::CreateNotificationServices(const Repositories & repos) {
  m_services[HandlerId::GetNotifications] = std::make_unique<services::NotificationsService>(repos.Notifications);
  m_services[HandlerId::GetChatNotifications] = std::make_unique<services::NotificationsService>(repos.Notifications);
  m_services[HandlerId::UpdateNotifications] = std::make_unique<services::NotificationsService>(repos.Notifications);
  m_services[HandlerId::UpdateChatNotifications] = std::make_unique<services::NotificationsService>(repos.Notifications);
}

} // namespace msngr::profile