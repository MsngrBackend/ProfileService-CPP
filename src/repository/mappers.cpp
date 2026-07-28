#include "mappers.hpp"
#include <chrono>

namespace msngr::profile::repository {

domain::Profile ProfileMapper::MapRow(const IRow* row)
{
  domain::Profile profile;

  auto userId = QueryExecutor::GetStringValue(row, "user_id");
  if (userId) {
    profile.UserID = *userId;
  }

  profile.FirstName = QueryExecutor::GetStringValue(row, "first_name");
  profile.LastName = QueryExecutor::GetStringValue(row, "last_name");
  profile.Username = QueryExecutor::GetStringValue(row, "username");
  profile.Bio = QueryExecutor::GetStringValue(row, "bio");
  profile.AvatarUrl = QueryExecutor::GetStringValue(row, "avatar_url");

  // TODO: Parse timestamps properly
  profile.CreatedAt = std::chrono::system_clock::now();
  profile.UpdatedAt = std::chrono::system_clock::now();

  return profile;
}

domain::Contact ContactMapper::MapRow(const IRow* row)
{
  domain::Contact contact;

  auto ownerId = QueryExecutor::GetStringValue(row, "owner_id");
  if (ownerId) contact.OwnerID = *ownerId;

  auto contactId = QueryExecutor::GetStringValue(row, "contact_id");
  if (contactId) {
    contact.ContactID = *contactId;
  }

  contact.Alias = QueryExecutor::GetStringValue(row, "alias");
  contact.CreatedAt = std::chrono::system_clock::now();

  return contact;
}

domain::PrivacySettings PrivacySettingsMapper::MapRow(const IRow* row)
{
  domain::PrivacySettings settings;

  auto userId = QueryExecutor::GetStringValue(row, "user_id");
  if (userId) {
    settings.UserID = *userId;
  }

  auto profileVis = QueryExecutor::GetStringValue(row, "profile_visibility");
  if (profileVis) {
    settings.ProfileVisibility = *profileVis;
  }

  auto lastSeenVis = QueryExecutor::GetStringValue(row, "last_seen_visibility");
  if (lastSeenVis) {
    settings.LastSeenVisibility = *lastSeenVis;
  }

  auto avatarVis = QueryExecutor::GetStringValue(row, "avatar_visibility");
  if (avatarVis) {
    settings.AvatarVisibility = *avatarVis;
  }

  return settings;
}

domain::NotificationSettings NotificationMapper::MapRow(const IRow* row)
{
  domain::NotificationSettings settings;

  auto id = QueryExecutor::GetStringValue(row, "id");
  if (id) {
    settings.ID = *id;
  }

  auto userId = QueryExecutor::GetStringValue(row, "user_id");
  if (userId) {
    settings.UserID = *userId;
  }

  settings.ChatID = QueryExecutor::GetStringValue(row, "chat_id");

  auto muted = QueryExecutor::GetBoolValue(row, "muted");
  if (muted) {
    settings.Muted = *muted;
  }

  // TODO: Parse muted_until timestamp

  return settings;
}

domain::Favorite FavoriteMapper::MapRow(const IRow* row)
{
  domain::Favorite favorite;

  auto userId = QueryExecutor::GetStringValue(row, "user_id");
  if (userId) {
    favorite.UserID = *userId;
  }

  auto chatId = QueryExecutor::GetStringValue(row, "chat_id");
  if (chatId) {
    favorite.ChatID = *chatId;
  }

  favorite.CreatedAt = std::chrono::system_clock::now();

  return favorite;
}

} // namespace msngr::profile::repository