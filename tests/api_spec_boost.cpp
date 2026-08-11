#define BOOST_TEST_MODULE ProfileServiceTests
#include <boost/test/included/unit_test.hpp>
#include "profile_service/api_spec.hpp"
#include "profile_service/routing/router.hpp"
#include "profile_service/services/iservice.hpp"
#include "profile_service/auth/auth_service.hpp"
#include <boost/beast/http/verb.hpp>
#include <nlohmann/json.hpp>

using namespace msngr::profile;

// ==================== ApiSpec Tests ====================

BOOST_AUTO_TEST_SUITE(ApiSpecTests)

BOOST_AUTO_TEST_CASE(ApiSpecHasExpectedRoutes) {
  auto const& spec = GetApiSpec();
  BOOST_TEST(spec.Version == "v1");

  auto key = std::string("/me:") + std::to_string(static_cast<int>(boost::beast::http::verb::get));
  BOOST_TEST(spec.FlatRoutes.contains(key));
  BOOST_TEST(spec.FlatRoutes.at(key).Id == HandlerId::GetMyProfile);
  BOOST_TEST(spec.FlatRoutes.at(key).Scope == "profile:read");
}

BOOST_AUTO_TEST_CASE(ApiSpecRouteScopeValidation) {
  auto const& spec = GetApiSpec();
  auto key = std::string("/notifications:") + std::to_string(static_cast<int>(boost::beast::http::verb::put));
  BOOST_TEST(spec.FlatRoutes.contains(key));
  BOOST_TEST(spec.FlatRoutes.at(key).Id == HandlerId::UpdateNotifications);
  BOOST_TEST(spec.FlatRoutes.at(key).Scope == "notifications:write");
}

BOOST_AUTO_TEST_CASE(ApiSpecAllRoutesHaveScopes) {
  auto const& spec = GetApiSpec();
  for (const auto& kv : spec.FlatRoutes) {
    BOOST_TEST(!kv.second.Scope.empty());
  }
}

BOOST_AUTO_TEST_CASE(ApiSpecRouteCount) {
  auto const& spec = GetApiSpec();
  // Actually there are 18 routes in api_spec.cpp
  BOOST_TEST(spec.FlatRoutes.size() == 18);
}

BOOST_AUTO_TEST_SUITE_END()

// ==================== AuthService Tests ====================

BOOST_AUTO_TEST_SUITE(AuthServiceTests)

BOOST_AUTO_TEST_CASE(AuthCheckValidUser) {
  AuthService auth;
  BOOST_TEST(auth.Check("user123", "profile:read") == true);
  BOOST_TEST(auth.Check("valid_user", "any:scope") == true);
}

BOOST_AUTO_TEST_CASE(AuthCheckEmptyUser) {
  AuthService auth;
  BOOST_TEST(auth.Check("", "profile:read") == false);
  BOOST_TEST(auth.Check("", "") == false);
}

BOOST_AUTO_TEST_CASE(AuthCheckIgnoresScope) {
  AuthService auth;
  BOOST_TEST(auth.Check("user123", "profile:read") == true);
  BOOST_TEST(auth.Check("user123", "profile:write") == true);
}

BOOST_AUTO_TEST_SUITE_END()

// ==================== HandlerId Enum Tests ====================

BOOST_AUTO_TEST_SUITE(HandlerIdTests)

BOOST_AUTO_TEST_CASE(HandlerIdEnumValues) {
  BOOST_TEST(static_cast<int>(HandlerId::CreateProfile) >= 0);
  BOOST_TEST(static_cast<int>(HandlerId::GetMyProfile) >= 0);
  BOOST_TEST(static_cast<int>(HandlerId::UpdateProfile) >= 0);
  BOOST_TEST(static_cast<int>(HandlerId::GetProfileByID) >= 0);
  BOOST_TEST(static_cast<int>(HandlerId::UploadAvatar) >= 0);
  BOOST_TEST(static_cast<int>(HandlerId::DeleteAvatar) >= 0);
  BOOST_TEST(static_cast<int>(HandlerId::GetPrivacy) >= 0);
  BOOST_TEST(static_cast<int>(HandlerId::UpdatePrivacy) >= 0);
  BOOST_TEST(static_cast<int>(HandlerId::GetContacts) >= 0);
  BOOST_TEST(static_cast<int>(HandlerId::AddContact) >= 0);
  BOOST_TEST(static_cast<int>(HandlerId::DeleteContact) >= 0);
  BOOST_TEST(static_cast<int>(HandlerId::GetFavorites) >= 0);
  BOOST_TEST(static_cast<int>(HandlerId::AddFavorite) >= 0);
  BOOST_TEST(static_cast<int>(HandlerId::RemoveFavorite) >= 0);
  BOOST_TEST(static_cast<int>(HandlerId::GetNotifications) >= 0);
  BOOST_TEST(static_cast<int>(HandlerId::GetChatNotifications) >= 0);
  BOOST_TEST(static_cast<int>(HandlerId::UpdateNotifications) >= 0);
  BOOST_TEST(static_cast<int>(HandlerId::UpdateChatNotifications) >= 0);
}

BOOST_AUTO_TEST_SUITE_END()

// ==================== Router Tests ====================

class BoostMockService : public IService {
public:
  HttpResponse Execute(const HttpRequest& /*request*/, const ServiceContext& /*context*/) override {
    boost::beast::http::response<boost::beast::http::string_body> res;
    res.result(boost::beast::http::status::ok);
    res.body() = "{\"status\":\"ok\"}";
    res.prepare_payload();
    return res;
  }
};

class BoostMockAuthService : public AuthService {
public:
  bool Check(const std::string& userId, const std::string& /*scope*/) const override {
    return userId == "valid_user";
  }
};

BOOST_AUTO_TEST_SUITE(RouterTests)

BOOST_AUTO_TEST_CASE(RouterBuildsRoutes) {
  std::unordered_map<HandlerId, std::unique_ptr<IService>> services;
  services[HandlerId::GetMyProfile] = std::make_unique<BoostMockService>();
  services[HandlerId::GetContacts] = std::make_unique<BoostMockService>();
  services[HandlerId::GetFavorites] = std::make_unique<BoostMockService>();
  services[HandlerId::GetNotifications] = std::make_unique<BoostMockService>();
  services[HandlerId::GetPrivacy] = std::make_unique<BoostMockService>();
  services[HandlerId::CreateProfile] = std::make_unique<BoostMockService>();

  auto authService = std::make_unique<BoostMockAuthService>();
  RequestRouter router(services, authService.get());
  router.BuildRoutes();

  BOOST_TEST(true);
}

BOOST_AUTO_TEST_CASE(RouterHandlesExactRoute) {
  std::unordered_map<HandlerId, std::unique_ptr<IService>> services;
  services[HandlerId::GetMyProfile] = std::make_unique<BoostMockService>();

  auto authService = std::make_unique<BoostMockAuthService>();
  RequestRouter router(services, authService.get());
  router.BuildRoutes();

  boost::beast::http::request<boost::beast::http::string_body> req;
  req.method(boost::beast::http::verb::get);
  req.target("/me");
  req.set("X-User-ID", "valid_user");

  auto response = router.Route(req);
  BOOST_TEST(response.result() == boost::beast::http::status::ok);
}

BOOST_AUTO_TEST_CASE(RouterHandlesParametricRoute) {
  std::unordered_map<HandlerId, std::unique_ptr<IService>> services;
  services[HandlerId::DeleteContact] = std::make_unique<BoostMockService>();
  services[HandlerId::GetProfileByID] = std::make_unique<BoostMockService>();

  auto authService = std::make_unique<BoostMockAuthService>();
  RequestRouter router(services, authService.get());
  router.BuildRoutes();

  boost::beast::http::request<boost::beast::http::string_body> req;
  req.method(boost::beast::http::verb::delete_);
  req.target("/contacts/contact123");
  req.set("X-User-ID", "valid_user");

  auto response = router.Route(req);
  BOOST_TEST(response.result() == boost::beast::http::status::ok);
}

BOOST_AUTO_TEST_CASE(RouterReturns404ForUnknownRoute) {
  std::unordered_map<HandlerId, std::unique_ptr<IService>> services;
  auto authService = std::make_unique<BoostMockAuthService>();
  RequestRouter router(services, authService.get());
  router.BuildRoutes();

  boost::beast::http::request<boost::beast::http::string_body> req;
  req.method(boost::beast::http::verb::get);
  req.target("/unknown/path");
  req.set("X-User-ID", "valid_user");

  auto response = router.Route(req);
  BOOST_TEST(response.result() == boost::beast::http::status::not_found);
}

BOOST_AUTO_TEST_CASE(RouterReturns401WithoutUserId) {
  std::unordered_map<HandlerId, std::unique_ptr<IService>> services;
  services[HandlerId::GetMyProfile] = std::make_unique<BoostMockService>();

  auto authService = std::make_unique<BoostMockAuthService>();
  RequestRouter router(services, authService.get());
  router.BuildRoutes();

  boost::beast::http::request<boost::beast::http::string_body> req;
  req.method(boost::beast::http::verb::get);
  req.target("/me");

  auto response = router.Route(req);
  BOOST_TEST(response.result() == boost::beast::http::status::unauthorized);
}

BOOST_AUTO_TEST_CASE(RouterHandlesInternalCreateProfile) {
  std::unordered_map<HandlerId, std::unique_ptr<IService>> services;
  services[HandlerId::CreateProfile] = std::make_unique<BoostMockService>();

  auto authService = std::make_unique<BoostMockAuthService>();
  RequestRouter router(services, authService.get());
  router.BuildRoutes();

  boost::beast::http::request<boost::beast::http::string_body> req;
  req.method(boost::beast::http::verb::post);
  req.target("/internal/profiles");
  req.body() = "{\"user_id\":\"new_user\"}";
  req.prepare_payload();

  auto response = router.Route(req);
  BOOST_TEST(response.result() == boost::beast::http::status::ok);
}

BOOST_AUTO_TEST_CASE(RouterHandlesOptions) {
  std::unordered_map<HandlerId, std::unique_ptr<IService>> services;
  auto authService = std::make_unique<BoostMockAuthService>();
  RequestRouter router(services, authService.get());
  router.BuildRoutes();

  boost::beast::http::request<boost::beast::http::string_body> req;
  req.method(boost::beast::http::verb::options);
  req.target("/me");

  auto response = router.Route(req);
  BOOST_TEST(response.result() == boost::beast::http::status::no_content);
}

BOOST_AUTO_TEST_SUITE_END()