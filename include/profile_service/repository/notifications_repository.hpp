#ifndef MSNGR__PROFILE__REPOSITORY__NOTIFICATIONS_REPOSITORY_HPP_
#define MSNGR__PROFILE__REPOSITORY__NOTIFICATIONS_REPOSITORY_HPP_

#include "profile_service/repository/repository.hpp"
#include "profile_service/domain/models.hpp"
#include "profile_service/repository/db_connection.hpp"
#include <memory>
#include <optional>

namespace msngr::profile::repository {

class NotificationRepositoryPostgres : public NotificationRepository {
public:
  explicit NotificationRepositoryPostgres(std::shared_ptr<IDatabaseConnection> conn);

  domain::NotificationSettings Get(const std::string & userId, const std::optional<std::string> & chatId) override;
  void Upsert(const domain::NotificationSettings & settings) override;

private:
  std::shared_ptr<IDatabaseConnection> m_connection;
};

} // namespace msngr::profile::repository

#endif  // MSNGR__PROFILE__REPOSITORY__NOTIFICATIONS_REPOSITORY_HPP_