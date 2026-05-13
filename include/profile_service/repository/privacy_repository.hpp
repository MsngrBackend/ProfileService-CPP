#ifndef MSNGR__PROFILE__REPOSITORY__PRIVACY_REPOSITORY_HPP_
#define MSNGR__PROFILE__REPOSITORY__PRIVACY_REPOSITORY_HPP_

#include "repository.hpp"
#include "../domain/models.hpp"
#include <pqxx/pqxx>
#include <memory>

namespace msngr::profile::repository {

class PrivacyRepositoryPostgres : public PrivacyRepository {
public:
    explicit PrivacyRepositoryPostgres(std::shared_ptr<pqxx::connection> conn);
    
    domain::PrivacySettings Get(const std::string& user_id) override;
    void Update(const domain::PrivacySettings& settings) override;
    
private:
    std::shared_ptr<pqxx::connection> conn_;
};

} // namespace msngr::profile::repository

#endif  // MSNGR__PROFILE__REPOSITORY__PRIVACY_REPOSITORY_HPP_