#include "profile_repository.hpp"
#include "mappers.hpp"
#include <stdexcept>

namespace msngr::profile::repository {

ProfileRepository::ProfileRepository(std::shared_ptr<QueryExecutor> executor)
: m_executor(std::move(executor)) {}

domain::Profile ProfileRepository::Create(const std::string& userId)
{
  const std::string query =
      "INSERT INTO profiles (user_id) VALUES ($1) "
      "ON CONFLICT (user_id) DO UPDATE SET updated_at = now() "
      "RETURNING user_id, first_name, last_name, username, bio, avatar_url, "
      "last_seen_at, created_at, updated_at";

  std::vector<DBValue> params = {userId};
  auto result = m_executor->ExecuteSelect(query, params);

  if (!result || result->Empty()) {
    throw std::runtime_error("Failed to create profile");
  }

  auto row = result->GetRow(0);
  return ProfileMapper::MapRow(row.get());
}

domain::Profile ProfileRepository::GetByID(const std::string & userId)
{
  const std::string query =
      "SELECT user_id, first_name, last_name, username, bio, avatar_url, "
      "last_seen_at, created_at, updated_at FROM profiles WHERE user_id = $1";

  auto result = m_executor->ExecuteSelect(query, {userId});

  if (!result || result->Empty()) {
    throw std::runtime_error("Profile not found");
  }

  auto row = result->GetRow(0);
  return ProfileMapper::MapRow(row.get());
}

void ProfileRepository::Update(const domain::Profile & profile)
{
  const std::string query =
      "UPDATE profiles SET first_name = $1, last_name = $2, username = $3, "
      "bio = $4, updated_at = now() WHERE user_id = $5";

  try {
    m_executor->ExecuteModify(
      query,
      {
        profile.FirstName.value_or(""),
        profile.LastName.value_or(""),
        profile.Username.value_or(""),
        profile.Bio.value_or(""),
        profile.UserID
      });
  } catch (const std::exception & e) {
    throw std::runtime_error("username already taken");
  }
}

void ProfileRepository::UpdateAvatarURL(const std::string & userId, const std::string & url)
{
  const std::string query =
      "UPDATE profiles SET avatar_url = $1, updated_at = now() WHERE user_id = $2";

  m_executor->ExecuteModify(query, {url, userId});
}

} // namespace msngr::profile::repository