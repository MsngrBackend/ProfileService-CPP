#ifndef MSNGR__PROFILE__REPOSITORY__MINIO_STORAGE_HPP_
#define MSNGR__PROFILE__REPOSITORY__MINIO_STORAGE_HPP_

#include <memory>
#include <string>
#include <vector>

namespace msngr::profile::repository {

class AvatarStorage {
public:
  virtual ~AvatarStorage() = default;
  virtual std::string Upload(const std::string & userId, const std::vector<uint8_t> & data, const std::string & content_type) = 0;
  virtual void Delete(const std::string & userId) = 0;
};

class MinIOStorage : public AvatarStorage {
public:
  MinIOStorage(const std::string & endpoint, const std::string & access_key, const std::string & secret_key);
  ~MinIOStorage();

  std::string Upload(const std::string & userId, const std::vector<uint8_t> & data, const std::string & content_type);
  void Delete(const std::string & userId);

private:
  std::string m_endpoint;
  std::string m_accessKey;
  std::string m_secretKey;
  static constexpr const char* BUCKET_NAME = "avatars";
};

} // namespace msngr::profile::repository

#endif  // MSNGR__PROFILE__REPOSITORY__MINIO_STORAGE_HPP_
