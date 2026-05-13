#ifndef MSNGR__PROFILE__HANDLERS__NOTIFICATIONS_HANDLER_HPP_
#define MSNGR__PROFILE__HANDLERS__NOTIFICATIONS_HANDLER_HPP_

#include "ihandler.hpp"
#include "../repository/repository.hpp"
#include <memory>

namespace msngr::profile::handlers {

class NotificationsHandler : public IHandler {
public:
    explicit NotificationsHandler(std::shared_ptr<repository::NotificationRepository> notification_repo);
    
    HttpResponse Handle(const HttpRequest& request, const RouteContext& context) override;
    
private:
    HttpResponse HandleGetNotifications(const HttpRequest& request, const RouteContext& context);
    HttpResponse HandleGetChatNotifications(const HttpRequest& request, const RouteContext& context);
    HttpResponse HandleUpdateNotifications(const HttpRequest& request, const RouteContext& context);
    HttpResponse HandleUpdateChatNotifications(const HttpRequest& request, const RouteContext& context);
    
    std::shared_ptr<repository::NotificationRepository> notification_repo_;
};

} // namespace msngr::profile::handlers

#endif  // MSNGR__PROFILE__HANDLERS__NOTIFICATIONS_HANDLER_HPP_