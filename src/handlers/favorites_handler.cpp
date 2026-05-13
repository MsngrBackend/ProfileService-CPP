#include "favorites_handler.hpp"
#include "../domain/models.hpp"
#include <nlohmann/json.hpp>

namespace msngr::profile::handlers {

using json = nlohmann::json;

FavoritesHandler::FavoritesHandler(std::shared_ptr<repository::FavoriteRepository> favorite_repo)
    : favorite_repo_(std::move(favorite_repo)) {}

HttpResponse FavoritesHandler::Handle(const HttpRequest& request, const RouteContext& context) {
    auto method = request.method();
    auto target = std::string(request.target());
    
    if (method == beast_http::verb::get && target == "/favorites") {
        return HandleGetFavorites(request, context);
    } else if (method == beast_http::verb::post && target.find("/favorites/") == 0) {
        return HandleAddFavorite(request, context);
    } else if (method == beast_http::verb::delete_ && target.find("/favorites/") == 0) {
        return HandleRemoveFavorite(request, context);
    }
    
    return ErrorResponse(404, request.version(), "not_found");
}

HttpResponse FavoritesHandler::HandleGetFavorites(const HttpRequest& request, const RouteContext& context) {
    try {
        auto favorites = favorite_repo_->List(context.UserID);
        json response = json::array();
        
        for (const auto& fav : favorites) {
            response.push_back({
                {"user_id", fav.UserID},
                {"chat_id", fav.ChatID},
                {"created_at", std::chrono::duration_cast<std::chrono::seconds>(fav.CreatedAt.time_since_epoch()).count()}
            });
        }
        
        return JsonResponse(200, request.version(), response.dump());
    } catch (const std::exception& e) {
        return ErrorResponse(500, request.version(), "failed to get favorites");
    }
}

HttpResponse FavoritesHandler::HandleAddFavorite(const HttpRequest& request, const RouteContext& context) {
    try {
        auto it = context.PathParams.find("chat_id");
        if (it == context.PathParams.end()) {
            return ErrorResponse(400, request.version(), "chat_id is required");
        }
        
        favorite_repo_->Add(context.UserID, it->second);
        return JsonResponse(201, request.version(), "");
    } catch (const std::exception& e) {
        return ErrorResponse(500, request.version(), "failed to add favorite");
    }
}

HttpResponse FavoritesHandler::HandleRemoveFavorite(const HttpRequest& request, const RouteContext& context) {
    try {
        auto it = context.PathParams.find("chat_id");
        if (it == context.PathParams.end()) {
            return ErrorResponse(400, request.version(), "chat_id is required");
        }
        
        favorite_repo_->Remove(context.UserID, it->second);
        return JsonResponse(204, request.version(), "");
    } catch (const std::exception& e) {
        return ErrorResponse(500, request.version(), "failed to remove favorite");
    }
}

} // namespace msngr::profile::handlers