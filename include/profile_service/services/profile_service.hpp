#ifndef MSNGR__PROFILE__SERVICES__PROFILE_SERVICE_HPP_
#define MSNGR__PROFILE__SERVICES__PROFILE_SERVICE_HPP_

#include "profile_service/services/iservice.hpp"
#include "profile_service/persistence/repositories/profile_repository.hpp"
#include "profile_service/storage/avatar_storage.hpp"

#include <memory>

namespace msngr::profile::services {

class ProfileService : public IService {
public:
  ProfileService(
    std::shared_ptr<repository::ProfileRepository> profileRepo,
    std::shared_ptr<repository::AvatarStorage> avatarStorage
  );

  HttpResponse Execute(const HttpRequest & request, const ServiceContext & context) override;

private:
  HttpResponse HandleCreateProfile(const HttpRequest & request, const ServiceContext & context);
  HttpResponse HandleGetMyProfile(const HttpRequest & request, const ServiceContext & context);
  HttpResponse HandleGetProfileByID(const HttpRequest & request, const ServiceContext & context);
  HttpResponse HandleUpdateProfile(const HttpRequest & request, const ServiceContext & context);
  HttpResponse HandleUploadAvatar(const HttpRequest & request, const ServiceContext & context);
  HttpResponse HandleDeleteAvatar(const HttpRequest & request, const ServiceContext & context);

  std::shared_ptr<repository::ProfileRepository> m_profileRepo;
  std::shared_ptr<repository::AvatarStorage> m_avatarStorage;
};

} // namespace msngr::profile::services

#endif // MSNGR__PROFILE__SERVICES__PROFILE_SERVICE_HPP_