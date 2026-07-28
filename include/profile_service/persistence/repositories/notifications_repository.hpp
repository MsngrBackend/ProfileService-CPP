#ifndef MSNGR__PROFILE__PERSISTENCE__REPOSITORIES__NOTIFICATIONS_REPOSITORY_HPP_
#define MSNGR__PROFILE__PERSISTENCE__REPOSITORIES__NOTIFICATIONS_REPOSITORY_HPP_

#include "profile_service/domain/models.hpp"
#include "profile_service/persistence/query_executor.hpp"
#include "profile_service/persistence/repositories/mappers.hpp"

#include <memory>
#include <optional>
#include <string>

namespace msngr::profile::repository {

class NotificationRepository {
public:
  explicit NotificationRepository(std::shared_ptr<QueryExecutor> executor);

  domain::NotificationSettings Get(
    const std::string & userId,
    const std::optional<std::string> & chatId
  );

  void Upsert(const domain::NotificationSettings & settings);

private:
  std::shared_ptr<QueryExecutor> m_executor;
};

} // namespace msngr::profile::repository

#endif // MSNGR__PROFILE__PERSISTENCE__REPOSITORIES__NOTIFICATIONS_REPOSITORY_HPP_