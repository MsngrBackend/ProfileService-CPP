#include "profile_handler.hpp"
#include "profile_service/utils/json_utils.hpp"
#include <nlohmann/json.hpp>
#include <signal.h>

namespace msngr::profile::handlers {

    using json = nlohmann::json;

    ProfileHandler::ProfileHandler(
        std::shared_ptr<repository::ProfileRepository> profileRepo,
        std::shared_ptr<repository::AvatarStorage> avatarStorage)
        : m_profileRepo(std::move(profileRepo)),
        m_avatarStorage(std::move(avatarStorage))
    {
    }

    HttpResponse ProfileHandler::Handle(const HttpRequest& request, const RouteContext& context)
    {
        auto method = request.method();
        auto target = std::string(request.target());
        if (method == beast_http::verb::post && target == "/internal/profiles")
            return HandleCreateProfile(request, context);
        else if (method == beast_http::verb::get && target == "/me")
            return HandleGetMyProfile(request, context);
        else if (method == beast_http::verb::patch && target == "/me")
            return HandleUpdateProfile(request, context);
        else if (method == beast_http::verb::get && target.find("/") == 0 && target != "/me")
            return HandleGetProfileByID(request, context);
        else if (method == beast_http::verb::post && target == "/me/avatar")
            return HandleUploadAvatar(request, context);
        else if (method == beast_http::verb::delete_ && target == "/me/avatar")
            return HandleDeleteAvatar(request, context);
        return ErrorResponse(404, request.version(), "not_found");
    }

    HttpResponse ProfileHandler::HandleCreateProfile(const HttpRequest& request, const RouteContext& context)
    {
        try {
            auto json_body = json::parse(request.body());
            std::string user_id = json_body.value("user_id", "");
            if (user_id.empty())
                return ErrorResponse(400, request.version(), "user_id required");
            auto profile = m_profileRepo->Create(user_id);
            json response;
            response["user_id"] = profile.UserID;
            response["created_at"] = std::chrono::duration_cast<std::chrono::seconds>(profile.CreatedAt.time_since_epoch()).count();
            response["updated_at"] = std::chrono::duration_cast<std::chrono::seconds>(profile.UpdatedAt.time_since_epoch()).count();
            return JsonResponse(201, request.version(), response.dump());
        }
        catch (const std::exception& e) {
            return ErrorResponse(500, request.version(), "create failed");
        }
    }

    HttpResponse ProfileHandler::HandleGetMyProfile(const HttpRequest& request, const RouteContext& context)
    {
        try {
            auto profile = m_profileRepo->GetByID(context.UserID);
            json response;
            response["user_id"] = profile.UserID;
            response["first_name"] = profile.FirstName.value_or("");
            response["last_name"] = profile.LastName.value_or("");
            response["username"] = profile.Username.value_or("");
            response["bio"] = profile.Bio.value_or("");
            response["avatar_url"] = profile.AvatarUrl.value_or("");
            response["created_at"] = std::chrono::duration_cast<std::chrono::seconds>(profile.CreatedAt.time_since_epoch()).count();
            response["updated_at"] = std::chrono::duration_cast<std::chrono::seconds>(profile.UpdatedAt.time_since_epoch()).count();
            return JsonResponse(200, request.version(), response.dump());
        }
        catch (const std::exception& e) {
            return ErrorResponse(404, request.version(), "profile not found");
        }
    }

    HttpResponse ProfileHandler::HandleGetProfileByID(const HttpRequest& request, const RouteContext& context)
    {
        try {
            auto it = context.PathParams.find("user_id");
            if (it == context.PathParams.end())
                return ErrorResponse(400, request.version(), "user_id required");
            auto profile = m_profileRepo->GetByID(it->second);
            json response;
            response["user_id"] = profile.UserID;
            response["first_name"] = profile.FirstName.value_or("");
            response["last_name"] = profile.LastName.value_or("");
            response["username"] = profile.Username.value_or("");
            response["bio"] = profile.Bio.value_or("");
            response["avatar_url"] = profile.AvatarUrl.value_or("");
            response["created_at"] = std::chrono::duration_cast<std::chrono::seconds>(profile.CreatedAt.time_since_epoch()).count();
            response["updated_at"] = std::chrono::duration_cast<std::chrono::seconds>(profile.UpdatedAt.time_since_epoch()).count();
            return JsonResponse(200, request.version(), response.dump());
        }
        catch (const std::exception& e) {
            return ErrorResponse(404, request.version(), "profile not found");
        }
    }

    HttpResponse ProfileHandler::HandleUpdateProfile(const HttpRequest& request, const RouteContext& context)
    {
        try {
            auto json_body = json::parse(request.body());
            auto profile = m_profileRepo->GetByID(context.UserID);
            if (json_body.contains("first_name")) profile.FirstName = json_body["first_name"].get<std::string>();
            if (json_body.contains("last_name")) profile.LastName = json_body["last_name"].get<std::string>();
            if (json_body.contains("username")) profile.Username = json_body["username"].get<std::string>();
            if (json_body.contains("bio")) profile.Bio = json_body["bio"].get<std::string>();
            m_profileRepo->Update(profile);
            json response;
            response["user_id"] = profile.UserID;
            response["first_name"] = profile.FirstName.value_or("");
            response["last_name"] = profile.LastName.value_or("");
            response["username"] = profile.Username.value_or("");
            response["bio"] = profile.Bio.value_or("");
            response["avatar_url"] = profile.AvatarUrl.value_or("");
            return JsonResponse(200, request.version(), response.dump());
        }
        catch (const std::exception& e) {
            std::string error_msg = e.what();
            if (error_msg.find("username already taken") != std::string::npos)
                return ErrorResponse(409, request.version(), "username already taken");
            return ErrorResponse(500, request.version(), "update failed");
        }
    }

    HttpResponse ProfileHandler::HandleUploadAvatar(const HttpRequest& request, const RouteContext& context)
    {
        try {
            auto json_body = json::parse(request.body());
            if (!json_body.contains("image_data"))
                return ErrorResponse(400, request.version(), "image_data (base64) is required");
            std::string base64_data = json_body["image_data"].get<std::string>();
            std::string content_type = json_body.value("content_type", "image/jpeg");
            auto image_data = msngr::profile::utils::DecodeBase64(base64_data);
            if (image_data.empty())
                return ErrorResponse(400, request.version(), "invalid image data");
            std::string avatar_url = m_avatarStorage->Upload(context.UserID, image_data, content_type);
            m_profileRepo->UpdateAvatarURL(context.UserID, avatar_url);
            json response;
            response["user_id"] = context.UserID;
            response["avatar_url"] = avatar_url;
            return JsonResponse(200, request.version(), response.dump());
        }
        catch (const std::exception& e) {
            return ErrorResponse(500, request.version(), std::string("upload failed: ") + e.what());
        }
    }

    HttpResponse ProfileHandler::HandleDeleteAvatar(const HttpRequest& request, const RouteContext& context)
    {
        try {
            m_avatarStorage->Delete(context.UserID);
            m_profileRepo->UpdateAvatarURL(context.UserID, "");
            return JsonResponse(204, request.version(), "");
        }
        catch (const std::exception& e) {
            return ErrorResponse(500, request.version(), "delete failed");
        }
    }

} // namespace msngr::profile::handlers