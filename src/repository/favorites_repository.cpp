#include "profile_service/repository/favorites_repository.hpp"

namespace msngr::profile::repository {

FavoriteRepositoryPostgres::FavoriteRepositoryPostgres(std::shared_ptr<IDatabaseConnection> conn)
    : m_connection(std::move(conn)) {}

std::vector<domain::Favorite> FavoriteRepositoryPostgres::List(const std::string & userId) {
  pqxx::work txn(m_connection->Connection());

  auto result = txn.exec_params(
    "SELECT user_id, chat_id, created_at FROM favorites "
    "WHERE user_id = $1 ORDER BY created_at DESC",
    userId
  );
 
  std::vector<domain::Favorite> favorites;
  for (const auto & row : result) {
    domain::Favorite fav;
    fav.UserID = row["user_id"].as<std::string>();
    fav.ChatID = row["chat_id"].as<std::string>();
    favorites.push_back(fav);
  }

  return favorites;
}

void FavoriteRepositoryPostgres::Add(const std::string & userId, const std::string & chatId) {
  pqxx::work txn(m_connection->Connection());

  txn.exec_params(
    "INSERT INTO favorites (user_id, chat_id) VALUES ($1, $2) "
    "ON CONFLICT (user_id, chat_id) DO NOTHING",
    userId, chatId
  );
  txn.commit();
}

void FavoriteRepositoryPostgres::Remove(const std::string & userId, const std::string & chatId) {
  pqxx::work txn(m_connection->Connection());

  txn.exec_params(
    "DELETE FROM favorites WHERE user_id = $1 AND chat_id = $2",
    userId, chatId
  );
  txn.commit();
}

} // namespace msngr::profile::repository