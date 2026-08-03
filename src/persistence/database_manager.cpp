#include "profile_service/persistence/database_manager.hpp"
#include "profile_service/persistence/postgres_connection.hpp"
#include "profile_service/utils/logger.hpp"

#include <stdexcept>

namespace msngr::profile {

DatabaseManager::DatabaseManager(std::string connectionString)
{
  Initialize(std::move(connectionString));
}

DatabaseManager::~DatabaseManager() = default;

void DatabaseManager::Initialize(std::string connectionString)
{
  try {
    // Create connection
    auto connection = std::make_unique<repository::PostgresConnection>(connectionString);
    if (!connection->IsOpen()) {
      throw std::runtime_error("Failed to connect to database");
    }

    // Create executor
    m_executor = std::make_shared<repository::QueryExecutor>(std::move(connection));

    // Create repositories
    m_repositories.Profile = std::make_shared<repository::ProfileRepository>(m_executor);
    m_repositories.Contacts = std::make_shared<repository::ContactsRepository>(m_executor);
    m_repositories.Privacy = std::make_shared<repository::PrivacyRepository>(m_executor);
    m_repositories.Favorites = std::make_shared<repository::FavoriteRepository>(m_executor);
    m_repositories.Notifications = std::make_shared<repository::NotificationRepository>(m_executor);

    LOG(info) << "Database initialized successfully";

  } catch (const std::exception& e) {
    LOG(error) << "Failed to initialize database: " << e.what();
    throw;
  }
}

} // namespace msngr::profile