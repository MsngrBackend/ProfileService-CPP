#ifndef MSNGR__PROFILE__REPOSITORY__FAVORITES_REPOSITORY_HPP_
#define MSNGR__PROFILE__REPOSITORY__FAVORITES_REPOSITORY_HPP_

#include "profile_service/repository/repository.hpp"
#include "profile_service/domain/models.hpp"
#include "profile_service/repository/db_connection.hpp"
#include <memory>
#include <vector>

namespace msngr::profile::repository {

class FavoriteRepositoryPostgres : public FavoriteRepository {
public:
  explicit FavoriteRepositoryPostgres(std::shared_ptr<IDatabaseConnection> conn);

  std::vector<domain::Favorite> List(const std::string & userId) override;
  void Add(const std::string & userId, const std::string & chatId) override;
  void Remove(const std::string & userId, const std::string & chatId) override;

private:
  std::shared_ptr<IDatabaseConnection> m_connection;
};

} // namespace msngr::profile::repository

#endif  // MSNGR__PROFILE__REPOSITORY__FAVORITES_REPOSITORY_HPP_