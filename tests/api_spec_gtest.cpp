#include <gtest/gtest.h>
#include "profile_service/api_spec.hpp"
#include "profile_service/routing/router.hpp"
#include "profile_service/services/iservice.hpp"
#include "profile_service/auth/auth_service.hpp"
#include <boost/beast/http/verb.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>
#include <nlohmann/json.hpp>

using namespace msngr::profile;

// ==================== ApiSpec Tests ====================

TEST(ApiSpecTest, HasExpectedRoutes) {
  auto const& spec = GetApiSpec();
  EXPECT_EQ(spec.Version, "v1");

  auto key = std::string("/me:") + std::to_string(static_cast<int>(boost::beast::http::verb::get));
  EXPECT_TRUE(spec.FlatRoutes.contains(key));
  EXPECT_EQ(spec.FlatRoutes.at(key).Id, HandlerId::GetMyProfile);
  EXPECT_EQ(spec.FlatRoutes.at(key).Scope, "profile:read");
}

TEST(ApiSpecTest, AllRoutesHaveValidScopes) {
  auto const& spec = GetApiSpec();
  for (const auto& kv : spec.FlatRoutes) {
    EXPECT_FALSE(kv.second.Scope.empty());
  }
}

TEST(ApiSpecTest, AllHandlerIdsAreMapped) {
  auto const& spec = GetApiSpec();
  std::set<HandlerId> foundIds;
  for (const auto& kv : spec.FlatRoutes) {
    foundIds.insert(kv.second.Id);
  }
  EXPECT_GE(foundIds.size(), 12);
}

TEST(ApiSpecTest, RouteCount) {
  auto const& spec = GetApiSpec();
  EXPECT_EQ(spec.FlatRoutes.size(), 18);
}

// ==================== AuthService Tests ====================

TEST(AuthServiceTest, ValidUserPasses) {
  AuthService auth;
  EXPECT_TRUE(auth.Check("user123", "profile:read"));
  EXPECT_TRUE(auth.Check("valid_user", "any:scope"));
}

TEST(AuthServiceTest, EmptyUserFails) {
  AuthService auth;
  EXPECT_FALSE(auth.Check("", "profile:read"));
  EXPECT_FALSE(auth.Check("", ""));
}

TEST(AuthServiceTest, ScopeIsIgnored) {
  AuthService auth;
  EXPECT_TRUE(auth.Check("user123", "profile:read"));
  EXPECT_TRUE(auth.Check("user123", "profile:write"));
  EXPECT_TRUE(auth.Check("user123", "invalid:scope"));
}

// ==================== Router Tests ====================

class MockService : public IService {
public:
  HttpResponse Execute(const HttpRequest& /*request*/, const ServiceContext& /*context*/) override {
    boost::beast::http::response<boost::beast::http::string_body> res;
    res.result(boost::beast::http::status::ok);
    res.body() = "{\"status\":\"ok\"}";
    res.prepare_payload();
    return res;
  }
};

class TestAuthService : public AuthService {
public:
  bool Check(const std::string& userId, const std::string& /*scope*/) const override {
    return userId == "valid_user";
  }
};

TEST(RouterTest, BuildRoutes) {
  std::unordered_map<HandlerId, std::unique_ptr<IService>> services;
  services[HandlerId::GetMyProfile] = std::make_unique<MockService>();
  services[HandlerId::GetContacts] = std::make_unique<MockService>();
  services[HandlerId::GetFavorites] = std::make_unique<MockService>();
  services[HandlerId::GetNotifications] = std::make_unique<MockService>();
  services[HandlerId::GetPrivacy] = std::make_unique<MockService>();
  services[HandlerId::CreateProfile] = std::make_unique<MockService>();

  auto authService = std::make_unique<TestAuthService>();
  RequestRouter router(services, authService.get());
  EXPECT_NO_THROW(router.BuildRoutes());
}

TEST(RouterTest, RouteExactMatch) {
  std::unordered_map<HandlerId, std::unique_ptr<IService>> services;
  services[HandlerId::GetMyProfile] = std::make_unique<MockService>();

  auto authService = std::make_unique<TestAuthService>();
  RequestRouter router(services, authService.get());
  router.BuildRoutes();

  boost::beast::http::request<boost::beast::http::string_body> req;
  req.method(boost::beast::http::verb::get);
  req.target("/me");
  req.set("X-User-ID", "valid_user");

  auto response = router.Route(req);
  EXPECT_EQ(response.result(), boost::beast::http::status::ok);
}

TEST(RouterTest, RouteParametricMatch) {
  std::unordered_map<HandlerId, std::unique_ptr<IService>> services;
  services[HandlerId::GetProfileByID] = std::make_unique<MockService>();
  services[HandlerId::DeleteContact] = std::make_unique<MockService>();

  auto authService = std::make_unique<TestAuthService>();
  RequestRouter router(services, authService.get());
  router.BuildRoutes();

  boost::beast::http::request<boost::beast::http::string_body> req;
  req.method(boost::beast::http::verb::get);
  req.target("/user123");
  req.set("X-User-ID", "valid_user");

  auto response = router.Route(req);
  EXPECT_EQ(response.result(), boost::beast::http::status::ok);
}

TEST(RouterTest, RouteNotFound) {
  std::unordered_map<HandlerId, std::unique_ptr<IService>> services;
  // Добавляем все необходимые сервисы, чтобы роутер не пытался найти
  // неподдерживаемый HandlerId
  services[HandlerId::GetMyProfile] = std::make_unique<MockService>();
  services[HandlerId::GetProfileByID] = std::make_unique<MockService>();
  services[HandlerId::DeleteContact] = std::make_unique<MockService>();

  auto authService = std::make_unique<TestAuthService>();
  RequestRouter router(services, authService.get());
  router.BuildRoutes();

  // Используем путь, который не совпадает ни с одним шаблоном
  // Например, путь с несколькими сегментами не совпадет с /{user_id}
  boost::beast::http::request<boost::beast::http::string_body> req;
  req.method(boost::beast::http::verb::get);
  req.target("/api/v1/unknown");
  req.set("X-User-ID", "valid_user");

  auto response = router.Route(req);
  EXPECT_EQ(response.result(), boost::beast::http::status::not_found);
}

TEST(RouterTest, MissingUserIdReturnsUnauthorized) {
  std::unordered_map<HandlerId, std::unique_ptr<IService>> services;
  services[HandlerId::GetMyProfile] = std::make_unique<MockService>();

  auto authService = std::make_unique<TestAuthService>();
  RequestRouter router(services, authService.get());
  router.BuildRoutes();

  boost::beast::http::request<boost::beast::http::string_body> req;
  req.method(boost::beast::http::verb::get);
  req.target("/me");

  auto response = router.Route(req);
  EXPECT_EQ(response.result(), boost::beast::http::status::unauthorized);
}

TEST(RouterTest, InternalCreateSkipsAuth) {
  std::unordered_map<HandlerId, std::unique_ptr<IService>> services;
  services[HandlerId::CreateProfile] = std::make_unique<MockService>();

  auto authService = std::make_unique<TestAuthService>();
  RequestRouter router(services, authService.get());
  router.BuildRoutes();

  boost::beast::http::request<boost::beast::http::string_body> req;
  req.method(boost::beast::http::verb::post);
  req.target("/internal/profiles");
  req.body() = "{\"user_id\":\"new_user\"}";
  req.prepare_payload();

  auto response = router.Route(req);
  EXPECT_EQ(response.result(), boost::beast::http::status::ok);
}

TEST(RouterTest, OptionsRoute) {
  std::unordered_map<HandlerId, std::unique_ptr<IService>> services;
  auto authService = std::make_unique<TestAuthService>();
  RequestRouter router(services, authService.get());
  router.BuildRoutes();

  boost::beast::http::request<boost::beast::http::string_body> req;
  req.method(boost::beast::http::verb::options);
  req.target("/me");

  auto response = router.Route(req);
  EXPECT_EQ(response.result(), boost::beast::http::status::no_content);
}

// ==================== Response Helpers Tests ====================

TEST(ResponseHelpersTest, JsonResponse) {
  auto response = handlers::JsonResponse(200, 11, "{\"test\":true}");
  EXPECT_EQ(response.result(), boost::beast::http::status::ok);
  EXPECT_EQ(response.version(), 11);
  EXPECT_EQ(response.body(), "{\"test\":true}");
  EXPECT_EQ(response.at(boost::beast::http::field::content_type), "application/json");
}

TEST(ResponseHelpersTest, ErrorResponse) {
  auto response = handlers::ErrorResponse(404, 11, "not_found");
  EXPECT_EQ(response.result(), boost::beast::http::status::not_found);
  EXPECT_EQ(response.version(), 11);

  auto json = nlohmann::json::parse(response.body());
  EXPECT_EQ(json["error"], "not_found");
}

// ==================== HandlerId Enum Tests ====================

TEST(HandlerIdTest, AllValuesAreValid) {
  EXPECT_GE(static_cast<int>(HandlerId::CreateProfile), 0);
  EXPECT_GE(static_cast<int>(HandlerId::GetMyProfile), 0);
  EXPECT_GE(static_cast<int>(HandlerId::UpdateProfile), 0);
  EXPECT_GE(static_cast<int>(HandlerId::GetProfileByID), 0);
  EXPECT_GE(static_cast<int>(HandlerId::UploadAvatar), 0);
  EXPECT_GE(static_cast<int>(HandlerId::DeleteAvatar), 0);
  EXPECT_GE(static_cast<int>(HandlerId::GetPrivacy), 0);
  EXPECT_GE(static_cast<int>(HandlerId::UpdatePrivacy), 0);
  EXPECT_GE(static_cast<int>(HandlerId::GetContacts), 0);
  EXPECT_GE(static_cast<int>(HandlerId::AddContact), 0);
  EXPECT_GE(static_cast<int>(HandlerId::DeleteContact), 0);
  EXPECT_GE(static_cast<int>(HandlerId::GetFavorites), 0);
  EXPECT_GE(static_cast<int>(HandlerId::AddFavorite), 0);
  EXPECT_GE(static_cast<int>(HandlerId::RemoveFavorite), 0);
  EXPECT_GE(static_cast<int>(HandlerId::GetNotifications), 0);
  EXPECT_GE(static_cast<int>(HandlerId::GetChatNotifications), 0);
  EXPECT_GE(static_cast<int>(HandlerId::UpdateNotifications), 0);
  EXPECT_GE(static_cast<int>(HandlerId::UpdateChatNotifications), 0);
}

// ==================== ServiceContext Tests ====================

TEST(ServiceContextTest, DefaultConstruction) {
  ServiceContext ctx;
  EXPECT_TRUE(ctx.UserID.empty());
  EXPECT_TRUE(ctx.PathParams.empty());
}

TEST(ServiceContextTest, CanSetUserId) {
  ServiceContext ctx;
  ctx.UserID = "test_user";
  EXPECT_EQ(ctx.UserID, "test_user");
}

TEST(ServiceContextTest, CanSetPathParams) {
  ServiceContext ctx;
  ctx.PathParams["user_id"] = "123";
  ctx.PathParams["chat_id"] = "456";
  EXPECT_EQ(ctx.PathParams.size(), 2);
  EXPECT_EQ(ctx.PathParams["user_id"], "123");
  EXPECT_EQ(ctx.PathParams["chat_id"], "456");
}