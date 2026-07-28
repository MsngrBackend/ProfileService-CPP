#ifndef MSNGR__PROFILE__SERVICES__NOTIFICATIONS_SERVICE_HPP_
#define MSNGR__PROFILE__SERVICES__NOTIFICATIONS_SERVICE_HPP_

#include "profile_service/services/iservice.hpp"
#include "profile_service/persistence/repositories/notifications_repository.hpp"

#include <memory>

namespace msngr::profile::services {

class NotificationsService : public IService {
public:
  explicit NotificationsService(std::shared_ptr<repository::NotificationRepository> notificationRepo);

  HttpResponse Execute(const HttpRequest & request, const ServiceContext & context) override;

private:
  HttpResponse HandleGetNotifications(const HttpRequest & request, const ServiceContext & context);
  HttpResponse HandleGetChatNotifications(const HttpRequest & request, const ServiceContext & context);
  HttpResponse HandleUpdateNotifications(const HttpRequest & request, const ServiceContext & context);
  HttpResponse HandleUpdateChatNotifications(const HttpRequest & request, const ServiceContext & context);

  std::shared_ptr<repository::NotificationRepository> m_notificationRepo;
};

} // namespace msngr::profile::services

#endif // MSNGR__PROFILE__SERVICES__NOTIFICATIONS_SERVICE_HPP_