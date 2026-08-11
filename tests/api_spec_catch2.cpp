#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
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

TEST_CASE("ApiSpec includes all expected routes", "[api_spec]") {
  auto const& spec = GetApiSpec();
  REQUIRE(spec.Version == "v1");

  SECTION("Profile routes") {
    auto key = std::string("/me:") + std::to_string(static_cast<int>(boost::beast::http::verb::get));
    REQUIRE(spec.FlatRoutes.contains(key));
    REQUIRE(spec.FlatRoutes.at(key).Id == HandlerId::GetMyProfile);
    REQUIRE(spec.FlatRoutes.at(key).Scope == "profile:read");

    key = std::string("/me:") + std::to_string(static_cast<int>(boost::beast::http::verb::patch));
    REQUIRE(spec.FlatRoutes.contains(key));
    REQUIRE(spec.FlatRoutes.at(key).Id == HandlerId::UpdateProfile);
    REQUIRE(spec.FlatRoutes.at(key).Scope == "profile:write");
  }

  SECTION("Contact routes") {
    auto key = std::string("/contacts:") + std::to_string(static_cast<int>(boost::beast::http::verb::get));
    REQUIRE(spec.FlatRoutes.contains(key));
    REQUIRE(spec.FlatRoutes.at(key).Id == HandlerId::GetContacts);
    REQUIRE(spec.FlatRoutes.at(key).Scope == "contacts:read");

    key = std::string("/contacts:") + std::to_string(static_cast<int>(boost::beast::http::verb::post));
    REQUIRE(spec.FlatRoutes.contains(key));
    REQUIRE(spec.FlatRoutes.at(key).Id == HandlerId::AddContact);
    REQUIRE(spec.FlatRoutes.at(key).Scope == "contacts:write");
  }

  SECTION("Favorite routes") {
    auto key = std::string("/favorites/{chat_id}:") + std::to_string(static_cast<int>(boost::beast::http::verb::delete_));
    REQUIRE(spec.FlatRoutes.contains(key));
    REQUIRE(spec.FlatRoutes.at(key).Id == HandlerId::RemoveFavorite);
    REQUIRE(spec.FlatRoutes.at(key).Scope == "favorites:write");
  }

  SECTION("Notification routes") {
    auto key = std::string("/notifications:") + std::to_string(static_cast<int>(boost::beast::http::verb::put));
    REQUIRE(spec.FlatRoutes.contains(key));
    REQUIRE(spec.FlatRoutes.at(key).Id == HandlerId::UpdateNotifications);
    REQUIRE(spec.FlatRoutes.at(key).Scope == "notifications:write");
  }

  SECTION("Privacy routes") {
    auto key = std::string("/me/privacy:") + std::to_string(static_cast<int>(boost::beast::http::verb::get));
    REQUIRE(spec.FlatRoutes.contains(key));
    REQUIRE(spec.FlatRoutes.at(key).Id == HandlerId::GetPrivacy);
    REQUIRE(spec.FlatRoutes.at(key).Scope == "profile:read");
  }
}

TEST_CASE("ApiSpec has correct route count", "[api_spec]") {
  auto const& spec = GetApiSpec();
  REQUIRE(spec.FlatRoutes.size() == 18);
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

class MockAuthService : public AuthService {
public:
  bool Check(const std::string& userId, const std::string& /*scope*/) const override {
    return userId == "valid_user";
  }
};

TEST_CASE("RequestRouter route matching", "[router]") {
  std::unordered_map<HandlerId, std::unique_ptr<IService>> services;
  services[HandlerId::GetMyProfile] = std::make_unique<MockService>();
  services[HandlerId::GetContacts] = std::make_unique<MockService>();
  services[HandlerId::GetFavorites] = std::make_unique<MockService>();
  services[HandlerId::GetNotifications] = std::make_unique<MockService>();
  services[HandlerId::GetPrivacy] = std::make_unique<MockService>();
  services[HandlerId::CreateProfile] = std::make_unique<MockService>();
  services[HandlerId::GetProfileByID] = std::make_unique<MockService>();
  services[HandlerId::DeleteContact] = std::make_unique<MockService>();

  auto authService = std::make_unique<MockAuthService>();
  RequestRouter router(services, authService.get());
  router.BuildRoutes();

  SECTION("Exact route matches /me") {
    boost::beast::http::request<boost::beast::http::string_body> req;
    req.method(boost::beast::http::verb::get);
    req.target("/me");
    req.set("X-User-ID", "valid_user");

    auto response = router.Route(req);
    REQUIRE(response.result() == boost::beast::http::status::ok);
  }

  SECTION("Parametric route matches /contacts/{contact_id}") {
    boost::beast::http::request<boost::beast::http::string_body> req;
    req.method(boost::beast::http::verb::delete_);
    req.target("/contacts/12345");
    req.set("X-User-ID", "valid_user");

    auto response = router.Route(req);
    REQUIRE(response.result() == boost::beast::http::status::ok);
  }

  SECTION("Returns 404 for truly unknown route") {
    // Use a path that doesn't match any pattern
    boost::beast::http::request<boost::beast::http::string_body> req;
    req.method(boost::beast::http::verb::get);
    req.target("/api/v1/unknown");
    req.set("X-User-ID", "valid_user");

    auto response = router.Route(req);
    REQUIRE(response.result() == boost::beast::http::status::not_found);
  }

  SECTION("Returns 401 without X-User-ID") {
    boost::beast::http::request<boost::beast::http::string_body> req;
    req.method(boost::beast::http::verb::get);
    req.target("/me");

    auto response = router.Route(req);
    REQUIRE(response.result() == boost::beast::http::status::unauthorized);
  }

  SECTION("Allows internal create profile without auth") {
    boost::beast::http::request<boost::beast::http::string_body> req;
    req.method(boost::beast::http::verb::post);
    req.target("/internal/profiles");
    req.body() = "{\"user_id\":\"new_user\"}";
    req.prepare_payload();

    auto response = router.Route(req);
    REQUIRE(response.result() == boost::beast::http::status::ok);
  }

  SECTION("Handles OPTIONS method") {
    boost::beast::http::request<boost::beast::http::string_body> req;
    req.method(boost::beast::http::verb::options);
    req.target("/me");

    auto response = router.Route(req);
    REQUIRE(response.result() == boost::beast::http::status::no_content);
  }
}

// ==================== AuthService Tests ====================

TEST_CASE("AuthService basic checks", "[auth]") {
  AuthService auth;

  SECTION("Validates non-empty user ID") {
    REQUIRE(auth.Check("user123", "profile:read") == true);
    REQUIRE(auth.Check("valid_user", "any:scope") == true);
  }

  SECTION("Rejects empty user ID") {
    REQUIRE(auth.Check("", "profile:read") == false);
    REQUIRE(auth.Check("", "") == false);
  }

  SECTION("Ignores scope in simple implementation") {
    REQUIRE(auth.Check("user123", "profile:read") == true);
    REQUIRE(auth.Check("user123", "profile:write") == true);
    REQUIRE(auth.Check("user123", "invalid:scope") == true);
  }
}