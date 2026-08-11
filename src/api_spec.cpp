#include "profile_service/api_spec.hpp"
#include <boost/beast/http/verb.hpp>

namespace msngr::profile {

const ApiSpec & GetApiSpec() {
    static const ApiSpec spec = []() {
      ApiSpec s;
      s.Version = "v1";

      auto addRoute = [&](const std::string & path,
                          beast_http::verb method,
                          HandlerId id,
                          const std::string & scope) {
        std::string key = path + ":" + std::to_string(static_cast<int>(method));
        s.FlatRoutes[key] = {method, id, scope};  // Use id
      };

      addRoute("/internal/profiles", beast_http::verb::post, HandlerId::CreateProfile, "profile:write");
      addRoute("/me", beast_http::verb::get, HandlerId::GetMyProfile, "profile:read");
      addRoute("/me", beast_http::verb::patch, HandlerId::UpdateProfile, "profile:write");
      addRoute("/{user_id}", beast_http::verb::get, HandlerId::GetProfileByID, "profile:read");
      addRoute("/me/avatar", beast_http::verb::post, HandlerId::UploadAvatar, "profile:write");
      addRoute("/me/avatar", beast_http::verb::delete_, HandlerId::DeleteAvatar, "profile:write");
      addRoute("/me/privacy", beast_http::verb::get, HandlerId::GetPrivacy, "profile:read");
      addRoute("/me/privacy", beast_http::verb::put, HandlerId::UpdatePrivacy, "profile:write");
      addRoute("/contacts", beast_http::verb::get, HandlerId::GetContacts, "contacts:read");
      addRoute("/contacts", beast_http::verb::post, HandlerId::AddContact, "contacts:write");
      addRoute("/contacts/{contact_id}", beast_http::verb::delete_, HandlerId::DeleteContact, "contacts:write");
      addRoute("/favorites", beast_http::verb::get, HandlerId::GetFavorites, "favorites:read");
      addRoute("/favorites/{chat_id}", beast_http::verb::post, HandlerId::AddFavorite, "favorites:write");
      addRoute("/favorites/{chat_id}", beast_http::verb::delete_, HandlerId::RemoveFavorite, "favorites:write");
      addRoute("/notifications", beast_http::verb::get, HandlerId::GetNotifications, "notifications:read");
      addRoute("/notifications/{chat_id}", beast_http::verb::get, HandlerId::GetChatNotifications, "notifications:read");
      addRoute("/notifications", beast_http::verb::put, HandlerId::UpdateNotifications, "notifications:write");
      addRoute("/notifications/{chat_id}", beast_http::verb::put, HandlerId::UpdateChatNotifications, "notifications:write");

    return s;
  }();

  return spec;
}

} // namespace msngr::profile