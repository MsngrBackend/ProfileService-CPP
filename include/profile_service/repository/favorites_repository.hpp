#ifndef MSNGR__PROFILE__REPOSITORY__FAVORITES_REPOSITORY_HPP_
#define MSNGR__PROFILE__REPOSITORY__FAVORITES_REPOSITORY_HPP_

#include "repository.hpp"
#include "../domain/models.hpp"
#include <pqxx/pqxx>
#include <memory>
#include <vector>

namespace msngr::profile::repository {

class FavoriteRepositoryPostgres : public FavoriteRepository {
public:
    explicit FavoriteRepositoryPostgres(std::shared_ptr<pqxx::connection> conn);
    
    std::vector<domain::Favorite> List(const std::string& user_id) override;
    void Add(const std::string& user_id, const std::string& chat_id) override;
    void Remove(const std::string& user_id, const std::string& chat_id) override;
    
private:
    std::shared_ptr<pqxx::connection> conn_;
};

} // namespace msngr::profile::repository

#endif  // MSNGR__PROFILE__REPOSITORY__FAVORITES_REPOSITORY_HPP_