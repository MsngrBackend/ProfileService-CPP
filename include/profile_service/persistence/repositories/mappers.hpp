#ifndef MSNGR__PROFILE__PERSISTENCE__REPOSITORIES__MAPPERS_HPP_
#define MSNGR__PROFILE__PERSISTENCE__REPOSITORIES__MAPPERS_HPP_

#include "profile_service/domain/models.hpp"
#include "profile_service/persistence/db_interface.hpp"
#include "profile_service/persistence/query_executor.hpp"

#include <memory>

namespace msngr::profile::repository {

class ProfileMapper {
public:
  static domain::Profile MapRow(const IRow* row);
};

class ContactMapper {
public:
  static domain::Contact MapRow(const IRow* row);
};

class PrivacySettingsMapper {
public:
  static domain::PrivacySettings MapRow(const IRow* row);
};

class NotificationMapper {
public:
  static domain::NotificationSettings MapRow(const IRow* row);
};

class FavoriteMapper {
public:
  static domain::Favorite MapRow(const IRow* row);
};

} // namespace msngr::profile::repository

#endif // MSNGR__PROFILE__PERSISTENCE__REPOSITORIES__MAPPERS_HPP_