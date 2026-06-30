#ifndef MSNGR__PROFILE__REPOSITORY__PROFILE_REPOSITORY_HPP_
#define MSNGR__PROFILE__REPOSITORY__PROFILE_REPOSITORY_HPP_

#include "profile_service/repository/repository.hpp"
#include "profile_service/domain/models.hpp"
#include "profile_service/repository/db_connection.hpp"
#include <memory>

namespace msngr::profile::repository {

class ProfileRepositoryPostgres : public ProfileRepository {
public:
  explicit ProfileRepositoryPostgres(std::shared_ptr<IDatabaseConnection> conn);

  domain::Profile Create(const std::string & userId) override;
  domain::Profile GetByID(const std::string & userId) override;
  void Update(const domain::Profile & profile) override;
  void UpdateAvatarURL(const std::string & userId, const std::string & url) override;

private:
  std::shared_ptr<IDatabaseConnection> m_connection;
};

} // namespace msngr::profile::repository

#endif  // MSNGR__PROFILE__REPOSITORY__PROFILE_REPOSITORY_HPP_