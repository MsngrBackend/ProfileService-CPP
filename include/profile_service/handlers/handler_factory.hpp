#ifndef MSNGR__PROFILE__HANDLERS__HANDLER_FACTORY_HPP_
#define MSNGR__PROFILE__HANDLERS__HANDLER_FACTORY_HPP_

#include "profile_service/handlers/ihandler.hpp"
#include "profile_service/api_spec.hpp"
#include "profile_service/repository/repository.hpp"

#include <functional>
#include <memory>
#include <unordered_map>

namespace msngr::profile::handlers {

class HandlerFactory {
public:
  HandlerFactory(
    std::shared_ptr<repository::ProfileRepository> profile_repo,
    std::shared_ptr<repository::ContactsRepository> contacts_repo,
    std::shared_ptr<repository::PrivacyRepository> privacy_repo,
    std::shared_ptr<repository::FavoriteRepository> favorite_repo,
    std::shared_ptr<repository::NotificationRepository> notification_repo,
    std::shared_ptr<repository::AvatarStorage> avatar_storage
  );

  std::function<HttpResponse(const HttpRequest&, const RouteContext &)> Bind(HandlerId id);

private:
  std::unordered_map<HandlerId, std::shared_ptr<IHandler>> m_handlers;
};

} // namespace msngr::profile::handlers

#endif // MSNGR__PROFILE__HANDLERS__HANDLER_FACTORY_HPP_