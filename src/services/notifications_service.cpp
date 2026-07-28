#include "profile_service/services/notifications_service.hpp"
#include "profile_service/domain/models.hpp"
#include "profile_service/utils/logger.hpp"

#include <nlohmann/json.hpp>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <ctime>

namespace msngr::profile::services {

using json = nlohmann::json;

NotificationsService::NotificationsService(
    std::shared_ptr<repository::NotificationRepository> notificationRepo
) : m_notificationRepo(std::move(notificationRepo)) {}

HttpResponse NotificationsService::Execute(const HttpRequest& request, const ServiceContext& context) {
    auto method = request.method();
    auto target = std::string(request.target());

    if (method == boost::beast::http::verb::get && target == "/notifications") {
        return HandleGetNotifications(request, context);
    }
    if (method == boost::beast::http::verb::get && target.find("/notifications/") == 0) {
        return HandleGetChatNotifications(request, context);
    }
    if (method == boost::beast::http::verb::put && target == "/notifications") {
        return HandleUpdateNotifications(request, context);
    }
    if (method == boost::beast::http::verb::put && target.find("/notifications/") == 0) {
        return HandleUpdateChatNotifications(request, context);
    }

    return handlers::ErrorResponse(404, request.version(), "not_found");
}

HttpResponse NotificationsService::HandleGetNotifications(const HttpRequest& request, const ServiceContext& context) {
    try {
        auto settings = m_notificationRepo->Get(context.UserID, std::nullopt);

        json response;
        response["id"] = settings.ID;
        response["user_id"] = settings.UserID;
        response["muted"] = settings.Muted;

        if (settings.MutedUntil.has_value()) {
            response["muted_until"] = std::chrono::duration_cast<std::chrono::seconds>(
                settings.MutedUntil->time_since_epoch()).count();
        }

        return handlers::JsonResponse(200, request.version(), response.dump());

    } catch (const std::exception& e) {
        LOG(error) << "GetNotifications failed for user " << context.UserID << ": " << e.what();
        return handlers::ErrorResponse(500, request.version(), "failed_to_get_settings");
    }
}

HttpResponse NotificationsService::HandleGetChatNotifications(const HttpRequest& request, const ServiceContext& context) {
    try {
        auto it = context.PathParams.find("chat_id");
        if (it == context.PathParams.end()) {
            return handlers::ErrorResponse(400, request.version(), "chat_id_required");
        }

        domain::NotificationSettings settings;
        try {
            settings = m_notificationRepo->Get(context.UserID, it->second);
        } catch (const std::exception&) {
            // Fallback to global settings
            settings = m_notificationRepo->Get(context.UserID, std::nullopt);
        }

        json response;
        response["id"] = settings.ID;
        response["user_id"] = settings.UserID;
        response["chat_id"] = it->second;
        response["muted"] = settings.Muted;

        if (settings.MutedUntil.has_value()) {
            response["muted_until"] = std::chrono::duration_cast<std::chrono::seconds>(
                settings.MutedUntil->time_since_epoch()).count();
        }

        return handlers::JsonResponse(200, request.version(), response.dump());

    } catch (const std::exception& e) {
        LOG(error) << "GetChatNotifications failed: " << e.what();
        return handlers::ErrorResponse(500, request.version(), "failed_to_get_settings");
    }
}

HttpResponse NotificationsService::HandleUpdateNotifications(const HttpRequest& request, const ServiceContext& context) {
    try {
        auto jsonBody = json::parse(request.body());

        domain::NotificationSettings settings;
        settings.UserID = context.UserID;
        settings.Muted = jsonBody.value("muted", false);

        if (jsonBody.contains("muted_until") && !jsonBody["muted_until"].is_null()) {
            int64_t timestamp = jsonBody["muted_until"];
            settings.MutedUntil = std::chrono::system_clock::time_point(
                std::chrono::seconds(timestamp));
        }

        m_notificationRepo->Upsert(settings);
        return handlers::JsonResponse(204, request.version(), "");

    } catch (const std::exception& e) {
        LOG(error) << "UpdateNotifications failed for user " << context.UserID << ": " << e.what();
        return handlers::ErrorResponse(400, request.version(), "update_failed");
    }
}

HttpResponse NotificationsService::HandleUpdateChatNotifications(const HttpRequest& request, const ServiceContext& context) {
    try {
        auto it = context.PathParams.find("chat_id");
        if (it == context.PathParams.end()) {
            return handlers::ErrorResponse(400, request.version(), "chat_id_required");
        }

        auto jsonBody = json::parse(request.body());

        domain::NotificationSettings settings;
        settings.UserID = context.UserID;
        settings.ChatID = it->second;
        settings.Muted = jsonBody.value("muted", false);

        if (jsonBody.contains("muted_until") && !jsonBody["muted_until"].is_null()) {
            int64_t timestamp = jsonBody["muted_until"];
            settings.MutedUntil = std::chrono::system_clock::time_point(
                std::chrono::seconds(timestamp));
        }

        m_notificationRepo->Upsert(settings);
        return handlers::JsonResponse(204, request.version(), "");

    } catch (const std::exception& e) {
        LOG(error) << "UpdateChatNotifications failed for user " << context.UserID << ": " << e.what();
        return handlers::ErrorResponse(400, request.version(), "update_failed");
    }
}

} // namespace msngr::profile::services