#ifndef MSNGR__PROFILE__REPOSITORY__PROFILE_REPOSITORY_HPP_
#define MSNGR__PROFILE__REPOSITORY__PROFILE_REPOSITORY_HPP_

#include "profile_service/domain/models.hpp"
#include "profile_service/repository/query_executor.hpp"
#include <memory>

namespace msngr::profile::repository {

class ProfileRepository {
public:
    explicit ProfileRepository(std::shared_ptr<QueryExecutor> executor);

    domain::Profile Create(const std::string& userId);
    domain::Profile GetByID(const std::string& userId);
    void Update(const domain::Profile& profile);
    void UpdateAvatarURL(const std::string& userId, const std::string& url);

private:
    std::shared_ptr<QueryExecutor> m_executor;
    
    domain::Profile MapRowToProfile(const IRow* row);
};

} // namespace msngr::profile::repository

#endif // MSNGR__PROFILE__REPOSITORY__PROFILE_REPOSITORY_HPP_