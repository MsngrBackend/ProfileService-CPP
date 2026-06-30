#ifndef MSNGR__PROFILE__REPOSITORY__PRIVACY_REPOSITORY_HPP_
#define MSNGR__PROFILE__REPOSITORY__PRIVACY_REPOSITORY_HPP_

#include "profile_service/repository/repository.hpp"
#include "profile_service/domain/models.hpp"
#include "profile_service/repository/db_connection.hpp"
#include <memory>

namespace msngr::profile::repository {

class PrivacyRepositoryPostgres : public PrivacyRepository {
public:
  explicit PrivacyRepositoryPostgres(std::shared_ptr<IDatabaseConnection> conn);

  domain::PrivacySettings Get(const std::string & userId) override;
  void Update(const domain::PrivacySettings & settings) override;

private:
  std::shared_ptr<IDatabaseConnection> m_connection;
};

} // namespace msngr::profile::repository

#endif  // MSNGR__PROFILE__REPOSITORY__PRIVACY_REPOSITORY_HPP_