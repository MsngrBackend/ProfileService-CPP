#include "profile_service/services/favorites_service.hpp"
#include "profile_service/domain/models.hpp"
#include "profile_service/utils/logger.hpp"

#include <nlohmann/json.hpp>
#include <chrono>

namespace msngr::profile::services {

using json = nlohmann::json;

FavoritesService::FavoritesService(std::shared_ptr<repository::FavoriteRepository> favoritesRepo)
    : m_favoritesRepo(std::move(favoritesRepo)) {}

HttpResponse FavoritesService::Execute(const HttpRequest& request, const ServiceContext& context) {
    auto method = request.method();
    auto target = std::string(request.target());

    if (method == boost::beast::http::verb::get && target == "/favorites") {
        return HandleGetFavorites(request, context);
    }
    if (method == boost::beast::http::verb::post && target.find("/favorites/") == 0) {
        return HandleAddFavorite(request, context);
    }
    if (method == boost::beast::http::verb::delete_ && target.find("/favorites/") == 0) {
        return HandleRemoveFavorite(request, context);
    }

    return handlers::ErrorResponse(404, request.version(), "not_found");
}

HttpResponse FavoritesService::HandleGetFavorites(const HttpRequest& request, const ServiceContext& context) {
    try {
        auto favorites = m_favoritesRepo->List(context.UserID);

        json response = json::array();
        for (const auto& fav : favorites) {
            json item;
            item["user_id"] = fav.UserID;
            item["chat_id"] = fav.ChatID;
            item["created_at"] = std::chrono::duration_cast<std::chrono::seconds>(
                fav.CreatedAt.time_since_epoch()).count();
            response.push_back(item);
        }

        return handlers::JsonResponse(200, request.version(), response.dump());

    } catch (const std::exception& e) {
        LOG(error) << "GetFavorites failed for user " << context.UserID << ": " << e.what();
        return handlers::ErrorResponse(500, request.version(), "failed_to_get_favorites");
    }
}

HttpResponse FavoritesService::HandleAddFavorite(const HttpRequest& request, const ServiceContext& context) {
    try {
        auto it = context.PathParams.find("chat_id");
        if (it == context.PathParams.end()) {
            return handlers::ErrorResponse(400, request.version(), "chat_id_required");
        }

        m_favoritesRepo->Add(context.UserID, it->second);
        return handlers::JsonResponse(201, request.version(), "");

    } catch (const std::exception& e) {
        LOG(error) << "AddFavorite failed for user " << context.UserID << ": " << e.what();
        return handlers::ErrorResponse(500, request.version(), "failed_to_add_favorite");
    }
}

HttpResponse FavoritesService::HandleRemoveFavorite(const HttpRequest& request, const ServiceContext& context) {
    try {
        auto it = context.PathParams.find("chat_id");
        if (it == context.PathParams.end()) {
            return handlers::ErrorResponse(400, request.version(), "chat_id_required");
        }

        m_favoritesRepo->Remove(context.UserID, it->second);
        return handlers::JsonResponse(204, request.version(), "");

    } catch (const std::exception& e) {
        LOG(error) << "RemoveFavorite failed for user " << context.UserID << ": " << e.what();
        return handlers::ErrorResponse(500, request.version(), "failed_to_remove_favorite");
    }
}

} // namespace msngr::profile::services