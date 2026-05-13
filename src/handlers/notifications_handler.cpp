#include "notifications_handler.hpp"
#include "../domain/models.hpp"
#include <nlohmann/json.hpp>
#include <chrono>

namespace msngr::profile::handlers {

using json = nlohmann::json;

NotificationsHandler::NotificationsHandler(std::shared_ptr<repository::NotificationRepository> notification_repo)
    : notification_repo_(std::move(notification_repo)) {}

HttpResponse NotificationsHandler::Handle(const HttpRequest& request, const RouteContext& context) {
    auto method = request.method();
    auto target = std::string(request.target());
    
    if (method == beast_http::verb::get && target == "/notifications") {
        return HandleGetNotifications(request, context);
    } else if (method == beast_http::verb::get && target.find("/notifications/") == 0) {
        return HandleGetChatNotifications(request, context);
    } else if (method == beast_http::verb::put && target == "/notifications") {
        return HandleUpdateNotifications(request, context);
    } else if (method == beast_http::verb::put && target.find("/notifications/") == 0) {
        return HandleUpdateChatNotifications(request, context);
    }
    
    return ErrorResponse(404, request.version(), "not_found");
}

HttpResponse NotificationsHandler::HandleGetNotifications(const HttpRequest& request, const RouteContext& context) {
    try {
        auto settings = notification_repo_->Get(context.UserID, std::nullopt);
        
        json response = {
            {"id", settings.ID},
            {"user_id", settings.UserID},
            {"muted", settings.Muted}
        };
        
        if (settings.MutedUntil.has_value()) {
            response["muted_until"] = std::chrono::duration_cast<std::chrono::seconds>(
                settings.MutedUntil->time_since_epoch()).count();
        }
        
        if (settings.ChatID.has_value()) {
            response["chat_id"] = *settings.ChatID;
        }
        
        return JsonResponse(200, request.version(), response.dump());
    } catch (const std::exception& e) {
        return ErrorResponse(500, request.version(), "failed to get settings");
    }
}

HttpResponse NotificationsHandler::HandleGetChatNotifications(const HttpRequest& request, const RouteContext& context) {
    try {
        auto it = context.PathParams.find("chat_id");
        if (it == context.PathParams.end()) {
            return ErrorResponse(400, request.version(), "chat_id is required");
        }
        
        domain::NotificationSettings settings;
        try {
            settings = notification_repo_->Get(context.UserID, it->second);
        } catch (const std::exception&) {
            // Fallback to global settings
            settings = notification_repo_->Get(context.UserID, std::nullopt);
        }
        
        json response = {
            {"id", settings.ID},
            {"user_id", settings.UserID},
            {"chat_id", it->second},
            {"muted", settings.Muted}
        };
        
        if (settings.MutedUntil.has_value()) {
            response["muted_until"] = std::chrono::duration_cast<std::chrono::seconds>(
                settings.MutedUntil->time_since_epoch()).count();
        }
        
        return JsonResponse(200, request.version(), response.dump());
    } catch (const std::exception& e) {
        return ErrorResponse(500, request.version(), "failed to get settings");
    }
}

HttpResponse NotificationsHandler::HandleUpdateNotifications(const HttpRequest& request, const RouteContext& context) {
    try {
        auto json_body = json::parse(request.body());
        
        bool muted = json_body.value("muted", false);
        std::optional<std::chrono::system_clock::time_point> muted_until;
        
        if (json_body.contains("muted_until") && !json_body["muted_until"].is_null()) {
            std::string muted_until_str = json_body["muted_until"];
            if (!muted_until_str.empty()) {
                // Parse RFC3339 time
                std::tm tm = {};
                std::istringstream ss(muted_until_str);
                ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
                if (ss.fail()) {
                    return ErrorResponse(400, request.version(), "invalid muted_until format, expected RFC3339");
                }
                auto time_point = std::chrono::system_clock::from_time_t(std::mktime(&tm));
                muted_until = time_point;
            }
        }
        
        domain::NotificationSettings settings;
        settings.UserID = context.UserID;
        settings.Muted = muted;
        settings.MutedUntil = muted_until;
        
        notification_repo_->Upsert(settings);
        
        return JsonResponse(204, request.version(), "");
    } catch (const std::exception& e) {
        return ErrorResponse(400, request.version(), e.what());
    }
}

HttpResponse NotificationsHandler::HandleUpdateChatNotifications(const HttpRequest& request, const RouteContext& context) {
    try {
        auto it = context.PathParams.find("chat_id");
        if (it == context.PathParams.end()) {
            return ErrorResponse(400, request.version(), "chat_id is required");
        }
        
        auto json_body = json::parse(request.body());
        
        bool muted = json_body.value("muted", false);
        std::optional<std::chrono::system_clock::time_point> muted_until;
        
        if (json_body.contains("muted_until") && !json_body["muted_until"].is_null()) {
            std::string muted_until_str = json_body["muted_until"];
            if (!muted_until_str.empty()) {
                std::tm tm = {};
                std::istringstream ss(muted_until_str);
                ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
                if (ss.fail()) {
                    return ErrorResponse(400, request.version(), "invalid muted_until format, expected RFC3339");
                }
                auto time_point = std::chrono::system_clock::from_time_t(std::mktime(&tm));
                muted_until = time_point;
            }
        }
        
        domain::NotificationSettings settings;
        settings.UserID = context.UserID;
        settings.ChatID = it->second;
        settings.Muted = muted;
        settings.MutedUntil = muted_until;
        
        notification_repo_->Upsert(settings);
        
        return JsonResponse(204, request.version(), "");
    } catch (const std::exception& e) {
        return ErrorResponse(400, request.version(), e.what());
    }
}

} // namespace msngr::profile::handlers