#ifndef MSNGR__PROFILE__PERSISTENCE__REPOSITORIES__PROFILE_REPOSITORY_HPP_
#define MSNGR__PROFILE__PERSISTENCE__REPOSITORIES__PROFILE_REPOSITORY_HPP_

#include "profile_service/domain/models.hpp"
#include "profile_service/persistence/query_executor.hpp"
#include "profile_service/persistence/repositories/mappers.hpp"

#include <memory>
#include <string>

namespace msngr::profile::repository {

class ProfileRepository {
public:
  explicit ProfileRepository(std::shared_ptr<QueryExecutor> executor);

  domain::Profile Create(const std::string & userId);
  domain::Profile GetByID(const std::string & userId);
  void Update(const domain::Profile & profile);
  void UpdateAvatarURL(const std::string & userId, const std::string & url);

private:
  std::shared_ptr<QueryExecutor> m_executor;
};

} // namespace msngr::profile::repository

#endif // MSNGR__PROFILE__PERSISTENCE__REPOSITORIES__PROFILE_REPOSITORY_HPP_