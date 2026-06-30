#include "notifications_repository.hpp"
#include "mappers.hpp"
#include <ctime>
#include <chrono>
#include <sstream>
#include <iomanip>

namespace msngr::profile::repository {

NotificationRepository::NotificationRepository(std::shared_ptr<QueryExecutor> executor)
  : m_executor(std::move(executor)) {}

domain::NotificationSettings NotificationRepository::Get(
  const std::string & userId,
  const std::optional<std::string> & chatId)
{
  std::string query;
  std::vector<DBValue> params = {userId};

  if (!chatId.has_value()) {
    query = "SELECT id, user_id, chat_id, muted, muted_until FROM notification_settings "
            "WHERE user_id = $1 AND chat_id IS NULL";
  } else {
    query = "SELECT id, user_id, chat_id, muted, muted_until FROM notification_settings "
            "WHERE user_id = $1 AND chat_id = $2";
    params.push_back(*chatId);
  }

  auto result = m_executor->ExecuteSelect(query, params);

  domain::NotificationSettings settings;

  if (!result || result->Empty()) {
    // Return defaults
    settings.UserID = userId;
    settings.Muted = false;
    settings.ChatID = chatId.value_or("");
  } else {
    auto row = result->GetRow(0);
    settings = NotificationMapper::MapRow(row.get());
  }

  return settings;
}

void NotificationRepository::Upsert(const domain::NotificationSettings& settings) {
  std::string query;
  std::vector<DBValue> params = {
    settings.UserID, 
    settings.ChatID.value_or(""), 
    settings.Muted
  };

  if (settings.MutedUntil.has_value()) {
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(
      settings.MutedUntil.value().time_since_epoch()).count();
    std::time_t t = static_cast<std::time_t>(seconds);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::gmtime(&t));

    query = "INSERT INTO notification_settings (user_id, chat_id, muted, muted_until) "
            "VALUES ($1, $2, $3, $4) "
            "ON CONFLICT (user_id, chat_id) DO UPDATE SET muted = $3, muted_until = $4";
    params.push_back(std::string(buf));
  } else {
    query = "INSERT INTO notification_settings (user_id, chat_id, muted, muted_until) "
            "VALUES ($1, $2, $3, NULL) "
            "ON CONFLICT (user_id, chat_id) DO UPDATE SET muted = $3, muted_until = NULL";
  }

  m_executor->ExecuteModify(query, params);
}

} // namespace msngr::profile::repository