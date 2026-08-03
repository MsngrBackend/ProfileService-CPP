#ifndef MSNGR__PROFILE__STORAGE__STORAGE_MANAGER_HPP_
#define MSNGR__PROFILE__STORAGE__STORAGE_MANAGER_HPP_

#include "profile_service/storage/minio_storage.hpp"
#include <memory>
#include <string>

namespace msngr::profile {

struct Storage {
  std::shared_ptr<repository::AvatarStorage> Avatar;
};

class StorageManager {
public:
  StorageManager(std::string endpoint, std::string accessKey, std::string secretKey);

  Storage GetStorage() const
  {
    return m_storage;
  }

private:
  Storage m_storage;
};

} // namespace msngr::profile

#endif // MSNGR__PROFILE__STORAGE__STORAGE_MANAGER_HPP_