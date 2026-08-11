#include <gtest/gtest.h>
#include "profile_service/api_spec.hpp"
#include <boost/beast/http/verb.hpp>

using namespace msngr::profile;

TEST(ApiSpecTest, HasExpectedRoutes) {
  auto const& spec = GetApiSpec();
  EXPECT_EQ(spec.Version, "v1");

  auto key = std::string("/me:") + std::to_string(static_cast<int>(boost::beast::http::verb::get));
  EXPECT_TRUE(spec.FlatRoutes.contains(key));
  EXPECT_EQ(spec.FlatRoutes.at(key).Id, HandlerId::GetMyProfile);
  EXPECT_EQ(spec.FlatRoutes.at(key).Scope, "profile:read");
}
