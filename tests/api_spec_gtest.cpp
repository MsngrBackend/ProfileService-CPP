#include <gtest/gtest.h>
#include "profile_service/api_spec.hpp"
#include "profile_service/routing/api_routes.hpp"
#include <boost/beast/http/verb.hpp>

using namespace msngr::profile;

TEST(ApiSpecTest, HasExpectedRoutes) {
  auto const& spec = GetApiSpec();
  EXPECT_EQ(spec.Version, "v1");
  EXPECT_EQ(spec.FlatRoutes.size(), api_routes::AllApiRoutes.size());

  auto key = std::string("/me:") + std::to_string(static_cast<int>(boost::beast::http::verb::get));
  EXPECT_TRUE(spec.FlatRoutes.contains(key));
  EXPECT_EQ(spec.FlatRoutes.at(key).Id, HandlerId::GetMyProfile);
  EXPECT_EQ(spec.FlatRoutes.at(key).Scope, "profile:read");
}

TEST(ApiSpecTest, AllDefinedRoutesAreRegistered) {
  auto const& spec = GetApiSpec();
  for (auto const& route : api_routes::AllApiRoutes) {
    auto key = std::string(route.Path) + ":" + std::to_string(static_cast<int>(route.Method));
    ASSERT_TRUE(spec.FlatRoutes.contains(key));
    EXPECT_EQ(spec.FlatRoutes.at(key).Id, route.Id);
    EXPECT_EQ(spec.FlatRoutes.at(key).Scope, route.Scope);
  }
}
