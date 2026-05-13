#include "favorites_repository.hpp"

namespace msngr::profile::repository {

FavoriteRepositoryPostgres::FavoriteRepositoryPostgres(std::shared_ptr<pqxx::connection> conn)
    : conn_(std::move(conn)) {}

std::vector<domain::Favorite> FavoriteRepositoryPostgres::List(const std::string& user_id) {
    pqxx::work txn(*conn_);
    
    auto result = txn.exec_params(
        "SELECT user_id, chat_id, created_at FROM favorites "
        "WHERE user_id = $1 ORDER BY created_at DESC",
        user_id
    );
    
    std::vector<domain::Favorite> favorites;
    for (const auto& row : result) {
        domain::Favorite fav;
        fav.UserID = row["user_id"].as<std::string>();
        fav.ChatID = row["chat_id"].as<std::string>();
        favorites.push_back(fav);
    }
    
    return favorites;
}

void FavoriteRepositoryPostgres::Add(const std::string& user_id, const std::string& chat_id) {
    pqxx::work txn(*conn_);
    
    txn.exec_params(
        "INSERT INTO favorites (user_id, chat_id) VALUES ($1, $2) "
        "ON CONFLICT (user_id, chat_id) DO NOTHING",
        user_id, chat_id
    );
    txn.commit();
}

void FavoriteRepositoryPostgres::Remove(const std::string& user_id, const std::string& chat_id) {
    pqxx::work txn(*conn_);
    
    txn.exec_params(
        "DELETE FROM favorites WHERE user_id = $1 AND chat_id = $2",
        user_id, chat_id
    );
    txn.commit();
}

} // namespace msngr::profile::repository