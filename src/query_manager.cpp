#include "profile_service/query_manager.hpp"
#include "profile_service/repository/postgres_connection.hpp"
#include "profile_service/repository/query_executor.hpp"
#include "profile_service/repository/profile_repository.hpp"
#include "profile_service/repository/contacts_repository.hpp"
#include "profile_service/repository/privacy_repository.hpp"
#include "profile_service/repository/favorites_repository.hpp"
#include "profile_service/repository/notifications_repository.hpp"
#include "profile_service/repository/minio_storage.hpp"
#include "profile_service/utils/logger.hpp"

#include <cstdlib>
#include <stdexcept>

namespace msngr::profile {

QueryManager::QueryManager() {
  InitializeRepositories();
  InitializeStorage();
}

QueryManager::~QueryManager() = default;

void QueryManager::InitializeRepositories() {
  const char* db_url = std::getenv("DATABASE_URL");
  if (!db_url) {
    throw std::runtime_error("DATABASE_URL environment variable not set");
  }

  auto connection = std::make_unique<repository::PostgresConnection>(db_url);
  m_queryExecutor = std::make_shared<repository::QueryExecutor>(std::move(connection));

  m_profileRepo = std::make_shared<repository::ProfileRepository>(m_queryExecutor);
  m_contactsRepo = std::make_shared<repository::ContactsRepository>(m_queryExecutor);
  m_privacyRepo = std::make_shared<repository::PrivacyRepository>(m_queryExecutor);
  m_favoriteRepo = std::make_shared<repository::FavoriteRepository>(m_queryExecutor);
  m_notificationRepo = std::make_shared<repository::NotificationRepository>(m_queryExecutor);

  LOG(info) << "Database repositories initialized";
}

void QueryManager::InitializeStorage() {
  auto minioEndpoint = std::getenv("MINIO_ENDPOINT") ? std::getenv("MINIO_ENDPOINT") : "localhost:9000";
  auto minioAccess = std::getenv("MINIO_ACCESS_KEY") ? std::getenv("MINIO_ACCESS_KEY") : "";
  auto minioSecret = std::getenv("MINIO_SECRET_KEY") ? std::getenv("MINIO_SECRET_KEY") : "";

  m_avatarStorage = std::make_shared<repository::MinIOStorage>(minioEndpoint, minioAccess, minioSecret);

  LOG(info) << "MinIO storage initialized";
}

std::shared_ptr<repository::ProfileRepository> QueryManager::GetProfileRepository() const {
  return m_profileRepo;
}

std::shared_ptr<repository::ContactsRepository> QueryManager::GetContactsRepository() const {
  return m_contactsRepo;
}

std::shared_ptr<repository::PrivacyRepository> QueryManager::GetPrivacyRepository() const {
  return m_privacyRepo;
}

std::shared_ptr<repository::FavoriteRepository> QueryManager::GetFavoriteRepository() const {
  return m_favoriteRepo;
}

std::shared_ptr<repository::NotificationRepository> QueryManager::GetNotificationRepository() const {
  return m_notificationRepo;
}

std::shared_ptr<repository::MinIOStorage> QueryManager::GetAvatarStorage() const {
  return m_avatarStorage;
}

} // namespace msngr::profile
