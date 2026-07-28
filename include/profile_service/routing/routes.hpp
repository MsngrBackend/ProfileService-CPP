#ifndef MSNGR__PROFILE__ROUTING__ROUTES_HPP_
#define MSNGR__PROFILE__ROUTING__ROUTES_HPP_

#include <boost/beast/http/verb.hpp>
#include <array>
#include <utility>

namespace msngr::profile {

namespace api_routes {

// Profile routes
inline constexpr auto ProfileRoutes = std::array<std::pair<const char*, boost::beast::http::verb>, 6>
{
  {{
    {"/internal/profiles", boost::beast::http::verb::post},
    {"/me", boost::beast::http::verb::get},
    {"/me", boost::beast::http::verb::patch},
    {"/{user_id}", boost::beast::http::verb::get},
    {"/me/avatar", boost::beast::http::verb::post},
    {"/me/avatar", boost::beast::http::verb::delete_},
  }}
};

// Contact management routes
inline constexpr auto ContactRoutes = std::array<std::pair<const char*, boost::beast::http::verb>, 3>
{
  {{
    {"/contacts", boost::beast::http::verb::get},
    {"/contacts", boost::beast::http::verb::post},
    {"/contacts/{contact_id}", boost::beast::http::verb::delete_},
  }}
};

// Privacy setting routes
inline constexpr auto PrivacyRoutes = std::array<std::pair<const char*, boost::beast::http::verb>, 2>
{
  {{
    {"/me/privacy", boost::beast::http::verb::get},
    {"/me/privacy", boost::beast::http::verb::put},
  }}
};

// Favorites management routes
inline constexpr auto FavoriteRoutes = std::array<std::pair<const char*, boost::beast::http::verb>, 3>
{
  {{
    {"/favorites", boost::beast::http::verb::get},
    {"/favorites/{chat_id}", boost::beast::http::verb::post},
    {"/favorites/{chat_id}", boost::beast::http::verb::delete_},
  }}
};

// Notification management routes
inline constexpr auto NotificationRoutes = std::array<std::pair<const char*, boost::beast::http::verb>, 4>
{
  {{
    {"/notifications", boost::beast::http::verb::get},
    {"/notifications/{chat_id}", boost::beast::http::verb::get},
    {"/notifications", boost::beast::http::verb::put},
    {"/notifications/{chat_id}", boost::beast::http::verb::put},
  }}
};

} // namespace api_routes

} // namespace msngr::profile

#endif // MSNGR__PROFILE__ROUTING__ROUTES_HPP_