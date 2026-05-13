#include "privacy_repository.hpp"

namespace msngr::profile::repository {

PrivacyRepositoryPostgres::PrivacyRepositoryPostgres(std::shared_ptr<pqxx::connection> conn)
    : conn_(std::move(conn)) {}

domain::PrivacySettings PrivacyRepositoryPostgres::Get(const std::string& user_id) {
    pqxx::work txn(*conn_);
    
    auto result = txn.exec_params(
        "SELECT user_id, profile_visibility, last_seen_visibility, avatar_visibility "
        "FROM privacy_settings WHERE user_id = $1",
        user_id
    );
    
    domain::PrivacySettings settings;
    
    if (result.empty()) {
        // Return defaults
        settings.UserID = user_id;
        settings.ProfileVisibility = "everyone";
        settings.LastSeenVisibility = "everyone";
        settings.AvatarVisibility = "everyone";
    } else {
        settings.UserID = result[0]["user_id"].as<std::string>();
        settings.ProfileVisibility = result[0]["profile_visibility"].as<std::string>();
        settings.LastSeenVisibility = result[0]["last_seen_visibility"].as<std::string>();
        settings.AvatarVisibility = result[0]["avatar_visibility"].as<std::string>();
    }
    
    return settings;
}

void PrivacyRepositoryPostgres::Update(const domain::PrivacySettings& settings) {
    pqxx::work txn(*conn_);
    
    txn.exec_params(
        "INSERT INTO privacy_settings (user_id, profile_visibility, last_seen_visibility, avatar_visibility) "
        "VALUES ($1, $2, $3, $4) "
        "ON CONFLICT (user_id) DO UPDATE SET "
        "profile_visibility = $2, last_seen_visibility = $3, avatar_visibility = $4",
        settings.UserID, settings.ProfileVisibility, 
        settings.LastSeenVisibility, settings.AvatarVisibility
    );
    txn.commit();
}

} // namespace msngr::profile::repository