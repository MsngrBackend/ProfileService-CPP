#ifndef MSNGR__PROFILE__REPOSITORY__FAVORITES_REPOSITORY_HPP_
#define MSNGR__PROFILE__REPOSITORY__FAVORITES_REPOSITORY_HPP_

#include "profile_service/domain/models.hpp"
#include "profile_service/repository/query_executor.hpp"
#include <memory>
#include <vector>

namespace msngr::profile::repository {

class FavoriteRepository {
public:
  explicit FavoriteRepository(std::shared_ptr<QueryExecutor> executor);

  std::vector<domain::Favorite> List(const std::string & userId);
  void Add(const std::string & userId, const std::string & chatId);
  void Remove(const std::string & userId, const std::string & chatId);

private:
  std::shared_ptr<QueryExecutor> m_executor;
};

} // namespace msngr::profile::repository

#endif  // MSNGR__PROFILE__REPOSITORY__FAVORITES_REPOSITORY_HPP_