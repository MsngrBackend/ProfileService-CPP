#include "notifications_repository.hpp"
#include <chrono>

namespace msngr::profile::repository {

NotificationRepositoryPostgres::NotificationRepositoryPostgres(std::shared_ptr<pqxx::connection> conn)
    : conn_(std::move(conn)) {}

domain::NotificationSettings NotificationRepositoryPostgres::Get(
    const std::string& user_id, 
    const std::optional<std::string>& chat_id) {
    
    pqxx::work txn(*conn_);
    pqxx::result result;
    
    if (!chat_id.has_value()) {
        result = txn.exec_params(
            "SELECT id, user_id, chat_id, muted, muted_until FROM notification_settings "
            "WHERE user_id = $1 AND chat_id IS NULL",
            user_id
        );
    } else {
        result = txn.exec_params(
            "SELECT id, user_id, chat_id, muted, muted_until FROM notification_settings "
            "WHERE user_id = $1 AND chat_id = $2",
            user_id, *chat_id
        );
    }
    
    domain::NotificationSettings settings;
    
    if (result.empty()) {
        // Return defaults
        settings.UserID = user_id;
        settings.Muted = false;
        settings.ChatID = chat_id;
    } else {
        settings.ID = result[0]["id"].as<std::string>();
        settings.UserID = result[0]["user_id"].as<std::string>();
        settings.Muted = result[0]["muted"].as<bool>();
        
        if (!result[0]["chat_id"].is_null()) {
            settings.ChatID = result[0]["chat_id"].as<std::string>();
        }
        
        if (!result[0]["muted_until"].is_null()) {
            auto time_str = result[0]["muted_until"].as<std::string>();
            // Parse timestamp
        }
    }
    
    return settings;
}

void NotificationRepositoryPostgres::Upsert(const domain::NotificationSettings& settings) {
    pqxx::work txn(*conn_);
    
    txn.exec_params(
        "INSERT INTO notification_settings (user_id, chat_id, muted, muted_until) "
        "VALUES ($1, $2, $3, $4) "
        "ON CONFLICT (user_id, chat_id) DO UPDATE SET "
        "muted = $3, muted_until = $4",
        settings.UserID, settings.ChatID, settings.Muted, settings.MutedUntil
    );
    txn.commit();
}

} // namespace msngr::profile::repository