#define BOOST_TEST_MODULE ApiSpecBoostTests
#include <boost/test/included/unit_test.hpp>
#include "profile_service/api_spec.hpp"
#include "profile_service/routing/api_routes.hpp"
#include <boost/beast/http/verb.hpp>

using namespace msngr::profile;

BOOST_AUTO_TEST_CASE(ApiSpecHasExpectedRoutes)
{
  auto const& spec = GetApiSpec();
  BOOST_TEST(spec.Version == "v1");
  BOOST_TEST(spec.FlatRoutes.size() == api_routes::AllApiRoutes.size());

  auto key = std::string("/me:") + std::to_string(static_cast<int>(boost::beast::http::verb::get));
  BOOST_TEST(spec.FlatRoutes.contains(key));
  BOOST_TEST(spec.FlatRoutes.at(key).Id == HandlerId::GetMyProfile);
  BOOST_TEST(spec.FlatRoutes.at(key).Scope == "profile:read");
}

BOOST_AUTO_TEST_CASE(ApiSpecRouteScopeValidation)
{
  auto const& spec = GetApiSpec();
  auto key = std::string("/notifications:") + std::to_string(static_cast<int>(boost::beast::http::verb::put));
  BOOST_TEST(spec.FlatRoutes.contains(key));
  BOOST_TEST(spec.FlatRoutes.at(key).Id == HandlerId::UpdateNotifications);
  BOOST_TEST(spec.FlatRoutes.at(key).Scope == "notifications:write");
}
