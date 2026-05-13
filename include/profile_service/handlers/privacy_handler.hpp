#ifndef MSNGR__PROFILE__HANDLERS__PRIVACY_HANDLER_HPP_
#define MSNGR__PROFILE__HANDLERS__PRIVACY_HANDLER_HPP_

#include "ihandler.hpp"
#include "../repository/repository.hpp"
#include <memory>

namespace msngr::profile::handlers {

class PrivacyHandler : public IHandler {
public:
    explicit PrivacyHandler(std::shared_ptr<repository::PrivacyRepository> privacy_repo);
    
    HttpResponse Handle(const HttpRequest& request, const RouteContext& context) override;
    
private:
    HttpResponse HandleGetPrivacy(const HttpRequest& request, const RouteContext& context);
    HttpResponse HandleUpdatePrivacy(const HttpRequest& request, const RouteContext& context);
    
    std::shared_ptr<repository::PrivacyRepository> privacy_repo_;
};

} // namespace msngr::profile::handlers

#endif  // MSNGR__PROFILE__HANDLERS__PRIVACY_HANDLER_HPP_