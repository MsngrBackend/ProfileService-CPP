#ifndef MSNGR__PROFILE__REPOSITORY__PROFILE_REPOSITORY_HPP_
#define MSNGR__PROFILE__REPOSITORY__PROFILE_REPOSITORY_HPP_

#include "repository.hpp"
#include "../domain/models.hpp"
#include <pqxx/pqxx>
#include <memory>

namespace msngr::profile::repository {

class ProfileRepositoryPostgres : public ProfileRepository {
public:
    explicit ProfileRepositoryPostgres(std::shared_ptr<pqxx::connection> conn);
    
    domain::Profile Create(const std::string& user_id) override;
    domain::Profile GetByID(const std::string& user_id) override;
    void Update(const domain::Profile& profile) override;
    void UpdateAvatarURL(const std::string& user_id, const std::string& url) override;
    
private:
    std::shared_ptr<pqxx::connection> conn_;
};

} // namespace msngr::profile::repository

#endif  // MSNGR__PROFILE__REPOSITORY__PROFILE_REPOSITORY_HPP_