#ifndef MSNGR__PROFILE__SERVICES__FAVORITES_SERVICE_HPP_
#define MSNGR__PROFILE__SERVICES__FAVORITES_SERVICE_HPP_

#include "profile_service/services/iservice.hpp"
#include "profile_service/persistence/repositories/favorites_repository.hpp"

#include <memory>

namespace msngr::profile::services {

class FavoritesService : public IService {
public:
  explicit FavoritesService(std::shared_ptr<repository::FavoriteRepository> favoritesRepo);

  HttpResponse Execute(const HttpRequest & request, const ServiceContext & context) override;

private:
  HttpResponse HandleGetFavorites(const HttpRequest & request, const ServiceContext & context);
  HttpResponse HandleAddFavorite(const HttpRequest & request, const ServiceContext & context);
  HttpResponse HandleRemoveFavorite(const HttpRequest & request, const ServiceContext & context);

  std::shared_ptr<repository::FavoriteRepository> m_favoritesRepo;
};

} // namespace msngr::profile::services

#endif // MSNGR__PROFILE__SERVICES__FAVORITES_SERVICE_HPP_