#ifndef MSNGR__PROFILE__HANDLERS__NOTIFICATIONS_HANDLER_HPP_
#define MSNGR__PROFILE__HANDLERS__NOTIFICATIONS_HANDLER_HPP_

#include "profile_service/handlers/ihandler.hpp"
#include "profile_service/repository/repository.hpp"
#include <memory>

namespace msngr::profile::handlers {

class NotificationsHandler : public IHandler {
public:
    explicit NotificationsHandler(std::shared_ptr<repository::NotificationRepository> notificationRepo);

    HttpResponse Handle(const HttpRequest & request, const RouteContext & context) override;

private:
    HttpResponse HandleGetNotifications(const HttpRequest & request, const RouteContext & context);
    HttpResponse HandleGetChatNotifications(const HttpRequest & request, const RouteContext & context);
    HttpResponse HandleUpdateNotifications(const HttpRequest & request, const RouteContext & context);
    HttpResponse HandleUpdateChatNotifications(const HttpRequest & request, const RouteContext & context);

    std::shared_ptr<repository::NotificationRepository> m_notificationRepo;
};

} // namespace msngr::profile::handlers

#endif  // MSNGR__PROFILE__HANDLERS__NOTIFICATIONS_HANDLER_HPP_