#ifndef MSNGR__PROFILE__API_SPEC_HPP_
#define MSNGR__PROFILE__API_SPEC_HPP_

#include <boost/beast/http.hpp>
#include <unordered_map>
#include <string>
#include <vector>

namespace beast_http = boost::beast::http;

namespace msngr::profile {

enum class HandlerId {
  // Profile
  CreateProfile,
  GetMyProfile,
  GetProfileByID,
  UpdateProfile,
  UploadAvatar,
  DeleteAvatar,

  // Contacts
  GetContacts,
  AddContact,
  DeleteContact,

  // Privacy
  GetPrivacy,
  UpdatePrivacy,

  // Favorites
  GetFavorites,
  AddFavorite,
  RemoveFavorite,

  // Notifications
  GetNotifications,
  GetChatNotifications,
  UpdateNotifications,
  UpdateChatNotifications
};

struct HandlerInfo {
  beast_http::verb method;
  HandlerId id;
  std::string scope;
};

struct RouteNode {
  std::string segment;
  std::unordered_map<beast_http::verb, HandlerInfo> handlers;
  std::vector<RouteNode> children;
  std::unordered_map<std::string, RouteNode> paramChildren;
  bool hasParam;
  std::string paramName;
};

struct ApiSpec {
  std::string version;
  std::unordered_map<std::string, HandlerInfo> flatRoutes;
  RouteNode routeTree;
};

const ApiSpec & GetApiSpec();

} // namespace msngr::profile

#endif  // MSNGR__PROFILE__API_SPEC_HPP_