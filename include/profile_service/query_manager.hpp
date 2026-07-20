#ifndef MSNGR__PROFILE__QUERY_MANAGER_HPP_
#define MSNGR__PROFILE__QUERY_MANAGER_HPP_

#include <memory>
#include <string>

namespace msngr::profile::repository {
  class ProfileRepository;
  class ContactsRepository;
  class PrivacyRepository;
  class FavoriteRepository;
  class NotificationRepository;
  class MinIOStorage;
  class QueryExecutor;
}

namespace msngr::profile {


class QueryManager {
public:
  QueryManager();
  ~QueryManager();

  std::shared_ptr<repository::ProfileRepository> GetProfileRepository() const;
  std::shared_ptr<repository::ContactsRepository> GetContactsRepository() const;
  std::shared_ptr<repository::PrivacyRepository> GetPrivacyRepository() const;
  std::shared_ptr<repository::FavoriteRepository> GetFavoriteRepository() const;
  std::shared_ptr<repository::NotificationRepository> GetNotificationRepository() const;
  std::shared_ptr<repository::MinIOStorage> GetAvatarStorage() const;

private:
  void InitializeRepositories();
  void InitializeStorage();

  std::shared_ptr<repository::QueryExecutor> m_queryExecutor;
  std::shared_ptr<repository::ProfileRepository> m_profileRepo;
  std::shared_ptr<repository::ContactsRepository> m_contactsRepo;
  std::shared_ptr<repository::PrivacyRepository> m_privacyRepo;
  std::shared_ptr<repository::FavoriteRepository> m_favoriteRepo;
  std::shared_ptr<repository::NotificationRepository> m_notificationRepo;
  std::shared_ptr<repository::MinIOStorage> m_avatarStorage;
};

} // namespace msngr::profile

#endif // MSNGR__PROFILE__QUERY_MANAGER_HPP_
