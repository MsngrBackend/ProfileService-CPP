#include "profile_service/storage/storage_manager.hpp"
#include "profile_service/utils/logger.hpp"

namespace msngr::profile {

StorageManager::StorageManager(std::string endpoint, std::string accessKey, std::string secretKey) {
    m_storage.Avatar = std::make_shared<repository::MinIOStorage>(
      std::move(endpoint), std::move(accessKey), std::move(secretKey));
    LOG(info) << "Storage manager initialized with endpoint: "
              << (m_storage.Avatar ? "configured" : "unconfigured");
}

} // namespace msngr::profile