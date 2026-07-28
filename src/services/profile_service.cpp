#include "profile_service/services/profile_service.hpp"
#include "profile_service/utils/json_utils.hpp"
#include "profile_service/utils/logger.hpp"
#include "profile_service/services/iservice.hpp"

#include <nlohmann/json.hpp>
#include <chrono>

namespace msngr::profile::services {

using json = nlohmann::json;

ProfileService::ProfileService(
    std::shared_ptr<repository::ProfileRepository> profileRepo,
    std::shared_ptr<repository::AvatarStorage> avatarStorage
) : m_profileRepo(std::move(profileRepo)),
    m_avatarStorage(std::move(avatarStorage)) {}

HttpResponse ProfileService::Execute(const HttpRequest& request, const ServiceContext& context) {
    auto method = request.method();
    auto target = std::string(request.target());

    if (method == boost::beast::http::verb::post && target == "/internal/profiles") {
        return HandleCreateProfile(request, context);
    }
    if (method == boost::beast::http::verb::get && target == "/me") {
        return HandleGetMyProfile(request, context);
    }
    if (method == boost::beast::http::verb::patch && target == "/me") {
        return HandleUpdateProfile(request, context);
    }
    if (method == boost::beast::http::verb::get && target.find("/") == 0 && target != "/me") {
        return HandleGetProfileByID(request, context);
    }
    if (method == boost::beast::http::verb::post && target == "/me/avatar") {
        return HandleUploadAvatar(request, context);
    }
    if (method == boost::beast::http::verb::delete_ && target == "/me/avatar") {
        return HandleDeleteAvatar(request, context);
    }

    return handlers::ErrorResponse(404, request.version(), "not_found");
}

HttpResponse ProfileService::HandleCreateProfile(const HttpRequest& request, const ServiceContext& /*context*/) {
    try {
        auto jsonBody = json::parse(request.body());
        std::string userId = jsonBody.value("user_id", "");

        if (userId.empty()) {
            return handlers::ErrorResponse(400, request.version(), "user_id_required");
        }

        auto profile = m_profileRepo->Create(userId);

        json response;
        response["user_id"] = profile.UserID;
        response["created_at"] = std::chrono::duration_cast<std::chrono::seconds>(
            profile.CreatedAt.time_since_epoch()).count();
        response["updated_at"] = std::chrono::duration_cast<std::chrono::seconds>(
            profile.UpdatedAt.time_since_epoch()).count();

        return handlers::JsonResponse(201, request.version(), response.dump());

    } catch (const std::exception& e) {
        LOG(error) << "CreateProfile failed: " << e.what();
        return handlers::ErrorResponse(500, request.version(), "create_failed");
    }
}

HttpResponse ProfileService::HandleGetMyProfile(const HttpRequest& request, const ServiceContext& context) {
    try {
        auto profile = m_profileRepo->GetByID(context.UserID);

        json response;
        response["user_id"] = profile.UserID;
        response["first_name"] = profile.FirstName.value_or("");
        response["last_name"] = profile.LastName.value_or("");
        response["username"] = profile.Username.value_or("");
        response["bio"] = profile.Bio.value_or("");
        response["avatar_url"] = profile.AvatarUrl.value_or("");
        response["created_at"] = std::chrono::duration_cast<std::chrono::seconds>(
            profile.CreatedAt.time_since_epoch()).count();
        response["updated_at"] = std::chrono::duration_cast<std::chrono::seconds>(
            profile.UpdatedAt.time_since_epoch()).count();

        return handlers::JsonResponse(200, request.version(), response.dump());

    } catch (const std::exception& e) {
        LOG(error) << "GetMyProfile failed for user " << context.UserID << ": " << e.what();
        return handlers::ErrorResponse(404, request.version(), "profile_not_found");
    }
}

HttpResponse ProfileService::HandleGetProfileByID(const HttpRequest& request, const ServiceContext& context) {
    try {
        auto it = context.PathParams.find("user_id");
        if (it == context.PathParams.end()) {
            return handlers::ErrorResponse(400, request.version(), "user_id_required");
        }

        auto profile = m_profileRepo->GetByID(it->second);

        json response;
        response["user_id"] = profile.UserID;
        response["first_name"] = profile.FirstName.value_or("");
        response["last_name"] = profile.LastName.value_or("");
        response["username"] = profile.Username.value_or("");
        response["bio"] = profile.Bio.value_or("");
        response["avatar_url"] = profile.AvatarUrl.value_or("");
        response["created_at"] = std::chrono::duration_cast<std::chrono::seconds>(
            profile.CreatedAt.time_since_epoch()).count();
        response["updated_at"] = std::chrono::duration_cast<std::chrono::seconds>(
            profile.UpdatedAt.time_since_epoch()).count();

        return handlers::JsonResponse(200, request.version(), response.dump());

    } catch (const std::exception& e) {
        LOG(error) << "GetProfileByID failed: " << e.what();
        return handlers::ErrorResponse(404, request.version(), "profile_not_found");
    }
}

HttpResponse ProfileService::HandleUpdateProfile(const HttpRequest& request, const ServiceContext& context) {
    try {
        auto jsonBody = json::parse(request.body());
        auto profile = m_profileRepo->GetByID(context.UserID);

        if (jsonBody.contains("first_name")) {
            profile.FirstName = jsonBody["first_name"].get<std::string>();
        }
        if (jsonBody.contains("last_name")) {
            profile.LastName = jsonBody["last_name"].get<std::string>();
        }
        if (jsonBody.contains("username")) {
            profile.Username = jsonBody["username"].get<std::string>();
        }
        if (jsonBody.contains("bio")) {
            profile.Bio = jsonBody["bio"].get<std::string>();
        }

        m_profileRepo->Update(profile);

        json response;
        response["user_id"] = profile.UserID;
        response["first_name"] = profile.FirstName.value_or("");
        response["last_name"] = profile.LastName.value_or("");
        response["username"] = profile.Username.value_or("");
        response["bio"] = profile.Bio.value_or("");
        response["avatar_url"] = profile.AvatarUrl.value_or("");

        return handlers::JsonResponse(200, request.version(), response.dump());

    } catch (const std::exception& e) {
        std::string errorMsg = e.what();
        if (errorMsg.find("username already taken") != std::string::npos ||
            errorMsg.find("duplicate key") != std::string::npos) {
            return handlers::ErrorResponse(409, request.version(), "username_already_taken");
        }
        LOG(error) << "UpdateProfile failed for user " << context.UserID << ": " << e.what();
        return handlers::ErrorResponse(500, request.version(), "update_failed");
    }
}

HttpResponse ProfileService::HandleUploadAvatar(const HttpRequest& request, const ServiceContext& context) {
    try {
        auto jsonBody = json::parse(request.body());

        if (!jsonBody.contains("image_data")) {
            return handlers::ErrorResponse(400, request.version(), "image_data_required");
        }

        std::string base64Data = jsonBody["image_data"].get<std::string>();
        std::string contentType = jsonBody.value("content_type", "image/jpeg");

        auto imageData = utils::DecodeBase64(base64Data);
        if (imageData.empty()) {
            return handlers::ErrorResponse(400, request.version(), "invalid_image_data");
        }

        std::string avatarUrl = m_avatarStorage->Upload(context.UserID, imageData, contentType);
        m_profileRepo->UpdateAvatarURL(context.UserID, avatarUrl);

        json response;
        response["user_id"] = context.UserID;
        response["avatar_url"] = avatarUrl;

        return handlers::JsonResponse(200, request.version(), response.dump());

    } catch (const std::exception& e) {
        LOG(error) << "UploadAvatar failed for user " << context.UserID << ": " << e.what();
        return handlers::ErrorResponse(500, request.version(), "upload_failed");
    }
}

HttpResponse ProfileService::HandleDeleteAvatar(const HttpRequest& request, const ServiceContext& context) {
    try {
        m_avatarStorage->Delete(context.UserID);
        m_profileRepo->UpdateAvatarURL(context.UserID, "");

        return handlers::JsonResponse(204, request.version(), "");

    } catch (const std::exception& e) {
        LOG(error) << "DeleteAvatar failed for user " << context.UserID << ": " << e.what();
        return handlers::ErrorResponse(500, request.version(), "delete_failed");
    }
}

} // namespace msngr::profile::services