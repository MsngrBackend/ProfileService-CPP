#ifndef MSNGR__PROFILE__STORAGE__MINIO_STORAGE_HPP_
#define MSNGR__PROFILE__STORAGE__MINIO_STORAGE_HPP_

#include "profile_service/storage/avatar_storage.hpp"
#include <cstdint>
#include <string>

namespace msngr::profile::repository {

class MinIOStorage : public AvatarStorage {
public:
  MinIOStorage(std::string endpoint, std::string accessKey, std::string secretKey);
  ~MinIOStorage();

  std::string Upload(
      const std::string & userId,
      const std::vector<uint8_t> & data,
      const std::string & contentType
  ) override;

  void Delete(const std::string& userId) override;

private:
    std::string m_endpoint;
    std::string m_accessKey;
    std::string m_secretKey;

    static constexpr const char* BUCKET_NAME = "avatars";
};

} // namespace msngr::profile::repository

#endif // MSNGR__PROFILE__STORAGE__MINIO_STORAGE_HPP_