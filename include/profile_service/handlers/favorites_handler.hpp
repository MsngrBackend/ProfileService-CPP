#ifndef MSNGR__PROFILE__HANDLERS__FAVORITES_HANDLER_HPP_
#define MSNGR__PROFILE__HANDLERS__FAVORITES_HANDLER_HPP_

#include "ihandler.hpp"
#include "../repository/repository.hpp"
#include <memory>

namespace msngr::profile::handlers {

class FavoritesHandler : public IHandler {
public:
    explicit FavoritesHandler(std::shared_ptr<repository::FavoriteRepository> favorite_repo);
    
    HttpResponse Handle(const HttpRequest& request, const RouteContext& context) override;
    
private:
    HttpResponse HandleGetFavorites(const HttpRequest& request, const RouteContext& context);
    HttpResponse HandleAddFavorite(const HttpRequest& request, const RouteContext& context);
    HttpResponse HandleRemoveFavorite(const HttpRequest& request, const RouteContext& context);
    
    std::shared_ptr<repository::FavoriteRepository> favorite_repo_;
};

} // namespace msngr::profile::handlers

#endif  // MSNGR__PROFILE__HANDLERS__FAVORITES_HANDLER_HPP_