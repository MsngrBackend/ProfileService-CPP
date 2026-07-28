#include "profile_service/persistence/repositories/privacy_repository.hpp"
#include "profile_service/persistence/repositories/mappers.hpp"

namespace msngr::profile::repository {

PrivacyRepository::PrivacyRepository(std::shared_ptr<QueryExecutor> executor)
  : m_executor(std::move(executor)) {}

domain::PrivacySettings PrivacyRepository::Get(const std::string & userId)
{
  const std::string query =
    "SELECT user_id, profile_visibility, last_seen_visibility, avatar_visibility "
    "FROM privacy_settings WHERE user_id = $1";

  auto result = m_executor->ExecuteSelect(query, {userId});

  domain::PrivacySettings settings;

  if (!result || result->Empty()) {
    // Return defaults
    settings.UserID = userId;
    settings.ProfileVisibility = "everyone";
    settings.LastSeenVisibility = "everyone";
    settings.AvatarVisibility = "everyone";
  } else {
    auto row = result->GetRow(0);
    settings = PrivacySettingsMapper::MapRow(row.get());
  }

  return settings;
}

void PrivacyRepository::Update(const domain::PrivacySettings & settings)
{
  const std::string query =
    "INSERT INTO privacy_settings (user_id, profile_visibility, last_seen_visibility, avatar_visibility) "
    "VALUES ($1, $2, $3, $4) "
    "ON CONFLICT (user_id) DO UPDATE SET "
    "profile_visibility = $2, last_seen_visibility = $3, avatar_visibility = $4";

  m_executor->ExecuteModify(query, {
    settings.UserID,
    settings.ProfileVisibility,
    settings.LastSeenVisibility,
    settings.AvatarVisibility
  });
}

} // namespace msngr::profile::repository