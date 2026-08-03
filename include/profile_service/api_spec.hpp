#ifndef MSNGR__PROFILE__API_SPEC_HPP_
#define MSNGR__PROFILE__API_SPEC_HPP_

#include <boost/beast/http.hpp>
#include <ostream>
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

inline std::ostream & operator<<(std::ostream & os, HandlerId id) {
  switch (id) {
    case HandlerId::CreateProfile: return os << "CreateProfile";
    case HandlerId::GetMyProfile: return os << "GetMyProfile";
    case HandlerId::GetProfileByID: return os << "GetProfileByID";
    case HandlerId::UpdateProfile: return os << "UpdateProfile";
    case HandlerId::UploadAvatar: return os << "UploadAvatar";
    case HandlerId::DeleteAvatar: return os << "DeleteAvatar";
    case HandlerId::GetContacts: return os << "GetContacts";
    case HandlerId::AddContact: return os << "AddContact";
    case HandlerId::DeleteContact: return os << "DeleteContact";
    case HandlerId::GetPrivacy: return os << "GetPrivacy";
    case HandlerId::UpdatePrivacy: return os << "UpdatePrivacy";
    case HandlerId::GetFavorites: return os << "GetFavorites";
    case HandlerId::AddFavorite: return os << "AddFavorite";
    case HandlerId::RemoveFavorite: return os << "RemoveFavorite";
    case HandlerId::GetNotifications: return os << "GetNotifications";
    case HandlerId::GetChatNotifications: return os << "GetChatNotifications";
    case HandlerId::UpdateNotifications: return os << "UpdateNotifications";
    case HandlerId::UpdateChatNotifications: return os << "UpdateChatNotifications";
    default: return os << "UnknownHandlerId";
  }
}

struct ApiSpec {
  std::string Version;
  std::unordered_map<std::string, HandlerInfo> FlatRoutes;
};

const ApiSpec& GetApiSpec();

} // namespace msngr::profile

#endif // MSNGR__PROFILE__API_SPEC_HPP_