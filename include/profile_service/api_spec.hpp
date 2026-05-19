#ifndef MSNGR__PROFILE__API_SPEC_HPP_
#define MSNGR__PROFILE__API_SPEC_HPP_

#include <boost/beast/http.hpp>
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>

namespace beast_http = boost::beast::http;

namespace msngr::profile {

	enum class HandlerId {
		CreateProfile, GetMyProfile, GetProfileByID, UpdateProfile, UploadAvatar, DeleteAvatar,
		GetContacts, AddContact, DeleteContact,
		GetPrivacy, UpdatePrivacy,
		GetFavorites, AddFavorite, RemoveFavorite,
		GetNotifications, GetChatNotifications, UpdateNotifications, UpdateChatNotifications
	};

	struct HandlerInfo {
		beast_http::verb method;
		HandlerId id;
		std::string scope;
	};

	struct RouteNode {
		std::string segment;
		std::unordered_map<beast_http::verb, HandlerInfo> handlers;
		std::vector<std::unique_ptr<RouteNode>> children;
		std::unordered_map<std::string, std::unique_ptr<RouteNode>> paramChildren;
		bool hasParam = false;
		std::string paramName;
	};

	struct ApiSpec {
		std::string version;
		std::unordered_map<std::string, HandlerInfo> flatRoutes;
		RouteNode routeTree;
	};

	const ApiSpec& GetApiSpec();

} // namespace msngr::profile

#endif