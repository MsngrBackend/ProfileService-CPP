#include "profile_service/repository/minio_storage.hpp"
#include <stdexcept>

namespace msngr::profile::repository {

MinIOStorage::MinIOStorage(const std::string & endpoint, const std::string & accessKey, const std::string & secretKey)
{
  m_endpoint = endpoint;
  m_accessKey = accessKey;
  m_secretKey = secretKey;
}

MinIOStorage::~MinIOStorage() = default;

std::string MinIOStorage::Upload(const std::string & userId, const std::vector<uint8_t> & data, const std::string & contentType)
{
  if (data.empty()) {
    throw std::runtime_error("Avatar payload is empty");
  }

  std::string extension = "jpg";
  if (contentType == "image/png") {
    extension = "png";
  } else if (contentType == "image/webp") {
    extension = "webp";
  }

  std::string base_url = m_endpoint.empty() ? "http://localhost:9000" : m_endpoint;
  if (base_url.rfind("http://", 0) != 0 && base_url.rfind("https://", 0) != 0) {
    base_url = "http://" + base_url;
  }

  // Temporary lightweight implementation for local testing in Docker.
  return base_url + "/" + std::string(BUCKET_NAME) + "/" + userId + "/avatar." + extension;
}

void MinIOStorage::Delete(const std::string & userId)
{
  (void)userId;
}

} // namespace msngr::profile::repository
