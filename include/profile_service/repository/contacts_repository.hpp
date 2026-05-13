#ifndef MSNGR__PROFILE__REPOSITORY__CONTACTS_REPOSITORY_HPP_
#define MSNGR__PROFILE__REPOSITORY__CONTACTS_REPOSITORY_HPP_

#include "repository.hpp"
#include "../domain/models.hpp"
#include <pqxx/pqxx>
#include <memory>
#include <vector>

namespace msngr::profile::repository {

class ContactsRepositoryPostgres : public ContactsRepository {
public:
    explicit ContactsRepositoryPostgres(std::shared_ptr<pqxx::connection> conn);
    
    std::vector<domain::Contact> List(const std::string& owner_id) override;
    void Add(const domain::Contact& contact) override;
    void Remove(const std::string& owner_id, const std::string& contact_id) override;
    
private:
    std::shared_ptr<pqxx::connection> conn_;
};

} // namespace msngr::profile::repository

#endif  // MSNGR__PROFILE__REPOSITORY__CONTACTS_REPOSITORY_HPP_