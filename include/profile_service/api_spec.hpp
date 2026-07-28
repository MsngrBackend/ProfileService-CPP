#ifndef MSNGR__PROFILE__API_SPEC_HPP_
#define MSNGR__PROFILE__API_SPEC_HPP_

#include <boost/beast/http.hpp>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

namespace beast_http = boost::beast::http;

namespace msngr::profile {

enum class HandlerId {
  // Profile operations
  CreateProfile,
  GetMyProfile,
  GetProfileByID,
  UpdateProfile,
  UploadAvatar,
  DeleteAvatar,

  // Contact operations
  GetContacts,
  AddContact,
  DeleteContact,

  // Privacy operations
  GetPrivacy,
  UpdatePrivacy,

  // Favorite operations
  GetFavorites,
  AddFavorite,
  RemoveFavorite,

  // Notification operations
  GetNotifications,
  GetChatNotifications,
  UpdateNotifications,
  UpdateChatNotifications
};

struct HandlerInfo {
  beast_http::verb Method;
  HandlerId Id;              // This is the handler ID
  std::string Scope;
};

struct ApiSpec {
  std::string Version;
  std::unordered_map<std::string, HandlerInfo> FlatRoutes;
};

const ApiSpec& GetApiSpec();

} // namespace msngr::profile

#endif // MSNGR__PROFILE__API_SPEC_HPP_