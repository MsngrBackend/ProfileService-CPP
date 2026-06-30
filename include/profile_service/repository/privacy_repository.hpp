#ifndef MSNGR__PROFILE__REPOSITORY__PRIVACY_REPOSITORY_HPP_
#define MSNGR__PROFILE__REPOSITORY__PRIVACY_REPOSITORY_HPP_

#include "profile_service/domain/models.hpp"
#include "profile_service/repository/query_executor.hpp"
#include <memory>

namespace msngr::profile::repository {

class PrivacyRepository {
public:
  explicit PrivacyRepository(std::shared_ptr<QueryExecutor> executor);

  domain::PrivacySettings Get(const std::string & userId);
  void Update(const domain::PrivacySettings & settings);

private:
  std::shared_ptr<QueryExecutor> m_executor;
};

} // namespace msngr::profile::repository

#endif  // MSNGR__PROFILE__REPOSITORY__PRIVACY_REPOSITORY_HPP_