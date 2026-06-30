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
		beast_http::verb Method;
		HandlerId Id;
		std::string Scope;
	};

	struct RouteNode {
		std::string Segment;
		std::unordered_map<beast_http::verb, HandlerInfo> Handlers;
		std::vector<std::unique_ptr<RouteNode>> Children;
		std::unordered_map<std::string, std::unique_ptr<RouteNode>> ParamChildren;
		bool HasParam = false;
		std::string ParamName;
	};

	struct ApiSpec {
		std::string Version;
		std::unordered_map<std::string, HandlerInfo> FlatRoutes;
		RouteNode RouteTree;
	};

	const ApiSpec & GetApiSpec();

} // namespace msngr::profile

#endif