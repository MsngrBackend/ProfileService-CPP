#include "favorites_repository.hpp"
#include "mappers.hpp"
#include <vector>

namespace msngr::profile::repository {

FavoriteRepository::FavoriteRepository(std::shared_ptr<QueryExecutor> executor)
    : m_executor(std::move(executor)) {}

std::vector<domain::Favorite> FavoriteRepository::List(const std::string & userId) {
  const std::string query =
    "SELECT user_id, chat_id, created_at FROM favorites "
    "WHERE user_id = $1 ORDER BY created_at DESC";

  auto result = m_executor->ExecuteSelect(query, {userId});

  std::vector<domain::Favorite> favorites;
  if (result && !result->Empty()) {
    for (size_t i = 0; i < result->Size(); ++i) {
      auto row = result->GetRow(i);
      favorites.push_back(FavoriteMapper::MapRow(row.get()));
    }
  }

  return favorites;
}

void FavoriteRepository::Add(const std::string & userId, const std::string & chatId) {
  const std::string query =
    "INSERT INTO favorites (user_id, chat_id) VALUES ($1, $2) "
    "ON CONFLICT (user_id, chat_id) DO NOTHING";

  m_executor->ExecuteModify(query, {userId, chatId});
}

void FavoriteRepository::Remove(const std::string & userId, const std::string & chatId) {
  const std::string query =
    "DELETE FROM favorites WHERE user_id = $1 AND chat_id = $2";

  m_executor->ExecuteModify(query, {userId, chatId});
}

} // namespace msngr::profile::repository