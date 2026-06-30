#ifndef MSNGR__PROFILE__REPOSITORY__CONTACTS_REPOSITORY_HPP_
#define MSNGR__PROFILE__REPOSITORY__CONTACTS_REPOSITORY_HPP_

#include "profile_service/domain/models.hpp"
#include "profile_service/repository/query_executor.hpp"
#include <memory>
#include <vector>

namespace msngr::profile::repository {

class ContactsRepository {
public:
  explicit ContactsRepository(std::shared_ptr<QueryExecutor> executor);

  std::vector<domain::Contact> List(const std::string & ownerId);
  void Add(const domain::Contact & contact);
  void Remove(const std::string & ownerId, const std::string & contactId);

private:
  std::shared_ptr<QueryExecutor> m_executor;
};

} // namespace msngr::profile::repository

#endif  // MSNGR__PROFILE__REPOSITORY__CONTACTS_REPOSITORY_HPP_