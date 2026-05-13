#ifndef MSNGR__PROFILE__REPOSITORY__NOTIFICATIONS_REPOSITORY_HPP_
#define MSNGR__PROFILE__REPOSITORY__NOTIFICATIONS_REPOSITORY_HPP_

#include "repository.hpp"
#include "../domain/models.hpp"
#include <pqxx/pqxx>
#include <memory>
#include <optional>

namespace msngr::profile::repository {

class NotificationRepositoryPostgres : public NotificationRepository {
public:
    explicit NotificationRepositoryPostgres(std::shared_ptr<pqxx::connection> conn);
    
    domain::NotificationSettings Get(const std::string& user_id, const std::optional<std::string>& chat_id) override;
    void Upsert(const domain::NotificationSettings& settings) override;
    
private:
    std::shared_ptr<pqxx::connection> conn_;
};

} // namespace msngr::profile::repository

#endif  // MSNGR__PROFILE__REPOSITORY__NOTIFICATIONS_REPOSITORY_HPP_