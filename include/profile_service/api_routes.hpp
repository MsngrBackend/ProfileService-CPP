#ifndef MSNGR__PROFILE__API_ROUTES_HPP_
#define MSNGR__PROFILE__API_ROUTES_HPP_

#include "profile_service/api_spec.hpp"
#include <boost/beast/http/verb.hpp>

namespace msngr::profile {

namespace beast_http = boost::beast::http;

namespace api_routes {

// Profile routes
constexpr auto ProfileRoutes = std::array<std::pair<const char*, beast_http::verb>, 5>{
  {
    {"/internal/profiles", beast_http::verb::post},
    {"/me", beast_http::verb::get},
    {"/me", beast_http::verb::patch},
    {"/{user_id}", beast_http::verb::get},
    {"/me/avatar", beast_http::verb::post},
  }
};

// Contact management routes
constexpr auto ContactRoutes = std::array<std::pair<const char*, beast_http::verb>, 3>{
  {
    {"/contacts", beast_http::verb::get},
    {"/contacts", beast_http::verb::post},
    {"/contacts/{contact_id}", beast_http::verb::delete_},
  }
};

// Privacy setting routes
constexpr auto PrivacyRoutes = std::array<std::pair<const char*, beast_http::verb>, 2>{
  {
    {"/me/privacy", beast_http::verb::get},
    {"/me/privacy", beast_http::verb::put},
  }
};

// Favorites management routes
constexpr auto FavoriteRoutes = std::array<std::pair<const char*, beast_http::verb>, 3>{
  {
    {"/favorites", beast_http::verb::get},
    {"/favorites/{chat_id}", beast_http::verb::post},
    {"/favorites/{chat_id}", beast_http::verb::delete_},
  }
};

// Notification management routes
constexpr auto NotificationRoutes = std::array<std::pair<const char*, beast_http::verb>, 4>{
  {
    {"/notifications", beast_http::verb::get},
    {"/notifications/{chat_id}", beast_http::verb::get},
    {"/notifications", beast_http::verb::put},
    {"/notifications/{chat_id}", beast_http::verb::put},
  }
};

} // namespace api_routes

} // namespace msngr::profile

#endif // MSNGR__PROFILE__API_ROUTES_HPP_
