#ifndef MSNGR__PROFILE__HANDLERS__PROFILE_HANDLER_HPP_
#define MSNGR__PROFILE__HANDLERS__PROFILE_HANDLER_HPP_

#include "profile_service/handlers/ihandler.hpp"
#include "profile_service/repository/repository.hpp"
#include <memory>

namespace msngr::profile::handlers {

class ProfileHandler : public IHandler {
public:
  ProfileHandler(
    std::shared_ptr<repository::ProfileRepository> profileRepo,
    std::shared_ptr<repository::AvatarStorage> avatarStorage
  );

  HttpResponse Handle(const HttpRequest & request, const RouteContext & context) override;

private:
  HttpResponse HandleCreateProfile(const HttpRequest & request, const RouteContext & context);
  HttpResponse HandleGetMyProfile(const HttpRequest & request, const RouteContext & context);
  HttpResponse HandleGetProfileByID(const HttpRequest & request, const RouteContext & context);
  HttpResponse HandleUpdateProfile(const HttpRequest & request, const RouteContext & context);
  HttpResponse HandleUploadAvatar(const HttpRequest & request, const RouteContext & context);
  HttpResponse HandleDeleteAvatar(const HttpRequest & request, const RouteContext & context);

private:
  std::shared_ptr<repository::ProfileRepository> m_profileRepo;
  std::shared_ptr<repository::AvatarStorage> m_avatarStorage;
};

} // namespace msngr::profile::handlers

#endif  // MSNGR__PROFILE__HANDLERS__PROFILE_HANDLER_HPP_