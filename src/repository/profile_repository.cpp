#include "profile_service/repository/profile_repository.hpp"
#include <pqxx/pqxx>
#include <chrono>
#include <stdexcept>

namespace msngr::profile::repository {

ProfileRepositoryPostgres::ProfileRepositoryPostgres(std::shared_ptr<IDatabaseConnection> conn)
  : m_connection(std::move(conn)) {}

domain::Profile ProfileRepositoryPostgres::Create(const std::string & userId) {
  pqxx::work txn(m_connection->Connection());

  auto result = txn.exec_params(
    "INSERT INTO profiles (user_id) VALUES ($1) "
    "ON CONFLICT (user_id) DO UPDATE SET updated_at = now() "
    "RETURNING user_id, first_name, last_name, username, bio, avatar_url, "
    "last_seen_at, created_at, updated_at",
    userId
  );

  txn.commit();

  if (result.empty()) {
    throw std::runtime_error("Failed to create profile");
  }

  domain::Profile profile;
  profile.UserID = result[0]["user_id"].as<std::string>();

  if (!result[0]["first_name"].is_null()) {
    profile.FirstName = result[0]["first_name"].as<std::string>();
  }
  if (!result[0]["last_name"].is_null()) {
    profile.LastName = result[0]["last_name"].as<std::string>();
  }
  if (!result[0]["username"].is_null()) {
    profile.Username = result[0]["username"].as<std::string>();
  }
  if (!result[0]["bio"].is_null()) {
    profile.Bio = result[0]["bio"].as<std::string>();
  }
  if (!result[0]["avatar_url"].is_null()) {
    profile.AvatarUrl = result[0]["avatar_url"].as<std::string>();
  }
  if (!result[0]["last_seen_at"].is_null()) {
    auto time_str = result[0]["last_seen_at"].as<std::string>();
    // Parse timestamp
  }

  profile.CreatedAt = std::chrono::system_clock::now();
  profile.UpdatedAt = std::chrono::system_clock::now();

  return profile;
}

domain::Profile ProfileRepositoryPostgres::GetByID(const std::string & userId) {
  pqxx::work txn(m_connection->Connection());

  auto result = txn.exec_params(
    "SELECT user_id, first_name, last_name, username, bio, avatar_url, "
    "last_seen_at, created_at, updated_at FROM profiles WHERE user_id = $1",
    userId
  );

  if (result.empty()) {
    throw std::runtime_error("Profile not found");
  }

  domain::Profile profile;
  profile.UserID = result[0]["user_id"].as<std::string>();

  if (!result[0]["first_name"].is_null()) {
    profile.FirstName = result[0]["first_name"].as<std::string>();
  }
  if (!result[0]["last_name"].is_null()) {
    profile.LastName = result[0]["last_name"].as<std::string>();
  }
  if (!result[0]["username"].is_null()) {
    profile.Username = result[0]["username"].as<std::string>();
  }
  if (!result[0]["bio"].is_null()) {
    profile.Bio = result[0]["bio"].as<std::string>();
  }
  if (!result[0]["avatar_url"].is_null()) {
    profile.AvatarUrl = result[0]["avatar_url"].as<std::string>();
  }

  return profile;
}

void ProfileRepositoryPostgres::Update(const domain::Profile & profile) {
  pqxx::work txn(m_connection->Connection());

  try {
    txn.exec_params(
      "UPDATE profiles SET first_name = $1, last_name = $2, username = $3, "
      "bio = $4, updated_at = now() WHERE user_id = $5",
      profile.FirstName, profile.LastName, profile.Username,
      profile.Bio, profile.UserID
    );
    txn.commit();
  } catch (const pqxx::unique_violation & e) {
    throw std::runtime_error("username already taken");
  }
}

void ProfileRepositoryPostgres::UpdateAvatarURL(const std::string & userId, const std::string & url) {
  pqxx::work txn(m_connection->Connection());

  txn.exec_params(
    "UPDATE profiles SET avatar_url = $1, updated_at = now() WHERE user_id = $2",
    url, userId
  );
  txn.commit();
}

} // namespace msngr::profile::repository