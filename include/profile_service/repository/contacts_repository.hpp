#ifndef MSNGR__PROFILE__REPOSITORY__CONTACTS_REPOSITORY_HPP_
#define MSNGR__PROFILE__REPOSITORY__CONTACTS_REPOSITORY_HPP_

#include "profile_service/repository/repository.hpp"
#include "profile_service/domain/models.hpp"
#include "profile_service/repository/db_connection.hpp"
#include <memory>
#include <vector>

namespace msngr::profile::repository {

class ContactsRepositoryPostgres : public ContactsRepository {
public:
  explicit ContactsRepositoryPostgres(std::shared_ptr<IDatabaseConnection> conn);

  std::vector<domain::Contact> List(const std::string & ownerId) override;
  void Add(const domain::Contact & contact) override;
  void Remove(const std::string & ownerId, const std::string & contactId) override;

private:
  std::shared_ptr<IDatabaseConnection> m_connection;
};

} // namespace msngr::profile::repository

#endif  // MSNGR__PROFILE__REPOSITORY__CONTACTS_REPOSITORY_HPP_