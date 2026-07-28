#ifndef MSNGR__PROFILE__PERSISTENCE__REPOSITORIES__CONTACTS_REPOSITORY_HPP_
#define MSNGR__PROFILE__PERSISTENCE__REPOSITORIES__CONTACTS_REPOSITORY_HPP_

#include "profile_service/domain/models.hpp"
#include "profile_service/persistence/query_executor.hpp"
#include "profile_service/persistence/repositories/mappers.hpp"

#include <memory>
#include <vector>
#include <string>

namespace msngr::profile::repository {

class ContactsRepository {
public:
  explicit ContactsRepository(std::shared_ptr<QueryExecutor> executor);

  std::vector<domain::Contact> List(const std::string &  ownerId);
  void Add(const domain::Contact & contact);
  void Remove(const std::string & ownerId, const std::string & contactId);

private:
  std::shared_ptr<QueryExecutor> m_executor;
};

} // namespace msngr::profile::repository

#endif // MSNGR__PROFILE__PERSISTENCE__REPOSITORIES__CONTACTS_REPOSITORY_HPP_