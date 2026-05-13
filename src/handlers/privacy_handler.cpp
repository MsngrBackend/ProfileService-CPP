#include "privacy_handler.hpp"
#include "../domain/models.hpp"
#include <nlohmann/json.hpp>

namespace msngr::profile::handlers {

using json = nlohmann::json;

PrivacyHandler::PrivacyHandler(std::shared_ptr<repository::PrivacyRepository> privacy_repo)
    : privacy_repo_(std::move(privacy_repo)) {}

HttpResponse PrivacyHandler::Handle(const HttpRequest& request, const RouteContext& context) {
    auto method = request.method();
    auto target = std::string(request.target());
    
    if (method == beast_http::verb::get && target == "/me/privacy") {
        return HandleGetPrivacy(request, context);
    } else if (method == beast_http::verb::put && target == "/me/privacy") {
        return HandleUpdatePrivacy(request, context);
    }
    
    return ErrorResponse(404, request.version(), "not_found");
}

HttpResponse PrivacyHandler::HandleGetPrivacy(const HttpRequest& request, const RouteContext& context) {
    try {
        auto settings = privacy_repo_->Get(context.UserID);
        
        json response = {
            {"user_id", settings.UserID},
            {"profile_visibility", settings.ProfileVisibility},
            {"last_seen_visibility", settings.LastSeenVisibility},
            {"avatar_visibility", settings.AvatarVisibility}
        };
        
        return JsonResponse(200, request.version(), response.dump());
    } catch (const std::exception& e) {
        return ErrorResponse(500, request.version(), "failed to get privacy settings");
    }
}

HttpResponse PrivacyHandler::HandleUpdatePrivacy(const HttpRequest& request, const RouteContext& context) {
    try {
        auto json_body = json::parse(request.body());
        
        domain::PrivacySettings settings;
        settings.UserID = context.UserID;
        settings.ProfileVisibility = json_body.value("profile_visibility", "everyone");
        settings.LastSeenVisibility = json_body.value("last_seen_visibility", "everyone");
        settings.AvatarVisibility = json_body.value("avatar_visibility", "everyone");
        
        privacy_repo_->Update(settings);
        
        return JsonResponse(204, request.version(), "");
    } catch (const std::exception& e) {
        return ErrorResponse(500, request.version(), "update failed");
    }
}

} // namespace msngr::profile::handlers