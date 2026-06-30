#include "privacy_handler.hpp"
#include "../domain/models.hpp"
#include <nlohmann/json.hpp>

namespace msngr::profile::handlers {

using json = nlohmann::json;

PrivacyHandler::PrivacyHandler(std::shared_ptr<repository::PrivacyRepository> privacyRepo)
    : m_privacyRepo(std::move(privacyRepo)) {}

HttpResponse PrivacyHandler::Handle(const HttpRequest & request, const RouteContext & context) {
  auto method = request.method();
  auto target = std::string(request.target());

  if (method == beast_http::verb::get && target == "/me/privacy") {
    return HandleGetPrivacy(request, context);
  } else if (method == beast_http::verb::put && target == "/me/privacy") {
    return HandleUpdatePrivacy(request, context);
  }

  return ErrorResponse(404, request.version(), "not_found");
}

HttpResponse PrivacyHandler::HandleGetPrivacy(const HttpRequest & request, const RouteContext & context) {
  try {
    auto settings = m_privacyRepo->Get(context.UserID);

    json response = {
      {"user_id", settings.UserID},
      {"profile_visibility", settings.ProfileVisibility},
      {"last_seen_visibility", settings.LastSeenVisibility},
      {"avatar_visibility", settings.AvatarVisibility}
    };

    return JsonResponse(200, request.version(), response.dump());
  } catch (const std::exception & e) {
    return ErrorResponse(500, request.version(), "failed to get privacy settings");
  }
}

HttpResponse PrivacyHandler::HandleUpdatePrivacy(const HttpRequest & request, const RouteContext & context) {
  try {
    auto jsonBody = json::parse(request.body());

    domain::PrivacySettings settings;
    settings.UserID = context.UserID;
    settings.ProfileVisibility = jsonBody.value("profile_visibility", "everyone");
    settings.LastSeenVisibility = jsonBody.value("last_seen_visibility", "everyone");
    settings.AvatarVisibility = jsonBody.value("avatar_visibility", "everyone");

    m_privacyRepo->Update(settings);

    return JsonResponse(204, request.version(), "");
  } catch (const std::exception& e) {
    return ErrorResponse(500, request.version(), "update failed");
  }
}

} // namespace msngr::profile::handlers