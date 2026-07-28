#ifndef MSNGR__PROFILE__DOMAIN__MODELS_HPP_
#define MSNGR__PROFILE__DOMAIN__MODELS_HPP_

#include <chrono>
#include <optional>
#include <string>

namespace msngr::profile::domain {

struct Profile {
  std::string UserID;
  std::optional<std::string> FirstName;
  std::optional<std::string> LastName;
  std::optional<std::string> Username;
  std::optional<std::string> Bio;
  std::optional<std::string> AvatarUrl;
  std::optional<std::chrono::system_clock::time_point> LastSeenAt;
  std::chrono::system_clock::time_point CreatedAt;
  std::chrono::system_clock::time_point UpdatedAt;
};

struct Contact {
  std::string OwnerID;
  std::string ContactID;
  std::optional<std::string> Alias;
  std::chrono::system_clock::time_point CreatedAt;
};

struct PrivacySettings {
  std::string UserID;
  std::string ProfileVisibility;
  std::string LastSeenVisibility;
  std::string AvatarVisibility;
};

struct NotificationSettings {
  std::string ID;
  std::string UserID;
  std::optional<std::string> ChatID;
  bool Muted = false;
  std::optional<std::chrono::system_clock::time_point> MutedUntil;
};

struct Favorite {
  std::string UserID;
  std::string ChatID;
  std::chrono::system_clock::time_point CreatedAt;
};

} // namespace msngr::profile::domain

#endif // MSNGR__PROFILE__DOMAIN__MODELS_HPP_