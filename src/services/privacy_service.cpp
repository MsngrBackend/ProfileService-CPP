#include "profile_service/services/privacy_service.hpp"
#include "profile_service/domain/models.hpp"
#include "profile_service/utils/logger.hpp"

#include <nlohmann/json.hpp>

namespace msngr::profile::services {

using json = nlohmann::json;

PrivacyService::PrivacyService(std::shared_ptr<repository::PrivacyRepository> privacyRepo)
    : m_privacyRepo(std::move(privacyRepo)) {}

HttpResponse PrivacyService::Execute(const HttpRequest& request, const ServiceContext& context) {
    auto method = request.method();
    auto target = std::string(request.target());

    if (method == boost::beast::http::verb::get && target == "/me/privacy") {
        return HandleGetPrivacy(request, context);
    }
    if (method == boost::beast::http::verb::put && target == "/me/privacy") {
        return HandleUpdatePrivacy(request, context);
    }

    return handlers::ErrorResponse(404, request.version(), "not_found");
}

HttpResponse PrivacyService::HandleGetPrivacy(const HttpRequest& request, const ServiceContext& context) {
    try {
        auto settings = m_privacyRepo->Get(context.UserID);

        json response;
        response["user_id"] = settings.UserID;
        response["profile_visibility"] = settings.ProfileVisibility;
        response["last_seen_visibility"] = settings.LastSeenVisibility;
        response["avatar_visibility"] = settings.AvatarVisibility;

        return handlers::JsonResponse(200, request.version(), response.dump());

    } catch (const std::exception& e) {
        LOG(error) << "GetPrivacy failed for user " << context.UserID << ": " << e.what();
        return handlers::ErrorResponse(500, request.version(), "failed_to_get_privacy_settings");
    }
}

HttpResponse PrivacyService::HandleUpdatePrivacy(const HttpRequest& request, const ServiceContext& context) {
    try {
        auto jsonBody = json::parse(request.body());

        domain::PrivacySettings settings;
        settings.UserID = context.UserID;
        settings.ProfileVisibility = jsonBody.value("profile_visibility", "everyone");
        settings.LastSeenVisibility = jsonBody.value("last_seen_visibility", "everyone");
        settings.AvatarVisibility = jsonBody.value("avatar_visibility", "everyone");

        m_privacyRepo->Update(settings);

        return handlers::JsonResponse(204, request.version(), "");

    } catch (const std::exception& e) {
        LOG(error) << "UpdatePrivacy failed for user " << context.UserID << ": " << e.what();
        return handlers::ErrorResponse(500, request.version(), "update_failed");
    }
}

} // namespace msngr::profile::services