#include "profile_service/handlers/handler_factory.hpp"
#include "profile_service/handlers/contacts_handler.hpp"
#include "profile_service/handlers/profile_handler.hpp"
#include "profile_service/handlers/privacy_handler.hpp"
#include "profile_service/handlers/favorites_handler.hpp"
#include "profile_service/handlers/notifications_handler.hpp"

namespace msngr::profile::handlers {

HandlerFactory::HandlerFactory(
  std::shared_ptr<repository::ProfileRepository> profileRepo,
  std::shared_ptr<repository::ContactsRepository> contactsRepo,
  std::shared_ptr<repository::PrivacyRepository> privacyRepo,
  std::shared_ptr<repository::FavoriteRepository> FavoriteRepo,
  std::shared_ptr<repository::NotificationRepository> NotificationRepo,
  std::shared_ptr<repository::AvatarStorage> avatarStorage
) {
  m_handlers[HandlerId::CreateProfile] = std::make_shared<ProfileHandler>(profileRepo, avatarStorage);
  m_handlers[HandlerId::GetMyProfile] = std::make_shared<ProfileHandler>(profileRepo, avatarStorage);
  m_handlers[HandlerId::GetProfileByID] = std::make_shared<ProfileHandler>(profileRepo, avatarStorage);
  m_handlers[HandlerId::UpdateProfile] = std::make_shared<ProfileHandler>(profileRepo, avatarStorage);
  m_handlers[HandlerId::UploadAvatar] = std::make_shared<ProfileHandler>(profileRepo, avatarStorage);
  m_handlers[HandlerId::DeleteAvatar] = std::make_shared<ProfileHandler>(profileRepo, avatarStorage);

  m_handlers[HandlerId::GetContacts] = std::make_shared<ContactsHandler>(contactsRepo);
  m_handlers[HandlerId::AddContact] = std::make_shared<ContactsHandler>(contactsRepo);
  m_handlers[HandlerId::DeleteContact] = std::make_shared<ContactsHandler>(contactsRepo);

  m_handlers[HandlerId::GetPrivacy] = std::make_shared<PrivacyHandler>(privacyRepo);
  m_handlers[HandlerId::UpdatePrivacy] = std::make_shared<PrivacyHandler>(privacyRepo);

  m_handlers[HandlerId::GetFavorites] = std::make_shared<FavoritesHandler>(FavoriteRepo);
  m_handlers[HandlerId::AddFavorite] = std::make_shared<FavoritesHandler>(FavoriteRepo);
  m_handlers[HandlerId::RemoveFavorite] = std::make_shared<FavoritesHandler>(FavoriteRepo);

  m_handlers[HandlerId::GetNotifications] = std::make_shared<NotificationsHandler>(NotificationRepo);
  m_handlers[HandlerId::GetChatNotifications] = std::make_shared<NotificationsHandler>(NotificationRepo);
  m_handlers[HandlerId::UpdateNotifications] = std::make_shared<NotificationsHandler>(NotificationRepo);
  m_handlers[HandlerId::UpdateChatNotifications] = std::make_shared<NotificationsHandler>(NotificationRepo);
}

std::function<HttpResponse(const HttpRequest &, const RouteContext &)> HandlerFactory::Bind(HandlerId id) {
  auto it = m_handlers.find(id);
  if (it != m_handlers.end()) {
    return [handler = it->second](const HttpRequest & req, const RouteContext & ctx) {
      return handler->Handle(req, ctx);
    };
  }

  return [](const HttpRequest & req, const RouteContext & ctx) -> HttpResponse {
    return ErrorResponse(501, req.version(), "handler not implemented");
  };
}

} // namespace msngr::profile::handlers