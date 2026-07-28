#include "profile_service/storage/storage_manager.hpp"
#include "profile_service/utils/logger.hpp"

namespace msngr::profile {

StorageManager::StorageManager(const std::string& endpoint, const std::string& accessKey, const std::string& secretKey) {
    m_storage.Avatar = std::make_shared<repository::MinIOStorage>(endpoint, accessKey, secretKey);
    LOG(info) << "Storage manager initialized with endpoint: " << endpoint;
}

} // namespace msngr::profile