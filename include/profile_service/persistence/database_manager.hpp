#ifndef MSNGR__PROFILE__PERSISTENCE__DATABASE_MANAGER_HPP_
#define MSNGR__PROFILE__PERSISTENCE__DATABASE_MANAGER_HPP_

#include "profile_service/persistence/query_executor.hpp"
#include "profile_service/persistence/repositories/profile_repository.hpp"
#include "profile_service/persistence/repositories/contacts_repository.hpp"
#include "profile_service/persistence/repositories/privacy_repository.hpp"
#include "profile_service/persistence/repositories/favorites_repository.hpp"
#include "profile_service/persistence/repositories/notifications_repository.hpp"

#include <memory>
#include <string>

namespace msngr::profile {

struct Repositories {
  std::shared_ptr<repository::ProfileRepository> Profile;
  std::shared_ptr<repository::ContactsRepository> Contacts;
  std::shared_ptr<repository::PrivacyRepository> Privacy;
  std::shared_ptr<repository::FavoriteRepository> Favorites;
  std::shared_ptr<repository::NotificationRepository> Notifications;
};

class DatabaseManager {
public:
  explicit DatabaseManager(std::string connectionString);
  ~DatabaseManager();

  DatabaseManager(const DatabaseManager &) = delete;
  DatabaseManager& operator=(const DatabaseManager &) = delete;

  inline Repositories GetRepositories() const
  {
    return m_repositories;
  }

private:
  void Initialize(std::string connectionString);

  std::shared_ptr<repository::QueryExecutor> m_executor;
  Repositories m_repositories;
};

} // namespace msngr::profile

#endif // MSNGR__PROFILE__PERSISTENCE__DATABASE_MANAGER_HPP_