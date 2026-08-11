#include <catch2/catch_test_macros.hpp>
#include "profile_service/api_spec.hpp"
#include <boost/beast/http/verb.hpp>

using namespace msngr::profile;

TEST_CASE("ApiSpec includes expected routes", "[api_spec]") {
  auto const & spec = GetApiSpec();
  REQUIRE(spec.Version == "v1");

  auto key = std::string("/me:") + std::to_string(static_cast<int>(boost::beast::http::verb::get));
  REQUIRE(spec.FlatRoutes.contains(key));
  REQUIRE(spec.FlatRoutes.at(key).Id == HandlerId::GetMyProfile);
  REQUIRE(spec.FlatRoutes.at(key).Scope == "profile:read");
}

TEST_CASE("ApiSpec correctly maps favorite routes", "[api_spec]") {
  auto const & spec = GetApiSpec();
  auto key = std::string("/favorites/{chat_id}:") + std::to_string(static_cast<int>(boost::beast::http::verb::delete_));
  REQUIRE(spec.FlatRoutes.contains(key));
  REQUIRE(spec.FlatRoutes.at(key).Id == HandlerId::RemoveFavorite);
  REQUIRE(spec.FlatRoutes.at(key).Scope == "favorites:write");
}
