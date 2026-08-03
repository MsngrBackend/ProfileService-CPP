#include "profile_service/storage/minio_storage.hpp"
#include <stdexcept>

namespace msngr::profile::repository {

MinIOStorage::MinIOStorage(std::string endpoint, std::string accessKey, std::string secretKey)
    : m_endpoint(std::move(endpoint)), m_accessKey(std::move(accessKey)), m_secretKey(std::move(secretKey)) {}

MinIOStorage::~MinIOStorage() = default;

std::string MinIOStorage::Upload(
    const std::string& userId,
    const std::vector<uint8_t>& data,
    const std::string& contentType)
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

    std::string baseUrl = m_endpoint.empty() ? "http://localhost:9000" : m_endpoint;
    if (baseUrl.rfind("http://", 0) != 0 && baseUrl.rfind("https://", 0) != 0) {
        baseUrl = "http://" + baseUrl;
    }

    // Temporary lightweight implementation for local testing in Docker
    return baseUrl + "/" + std::string(BUCKET_NAME) + "/" + userId + "/avatar." + extension;
}

void MinIOStorage::Delete(const std::string& userId) {
    (void)userId;
    // In production, this would delete the file from MinIO
}

} // namespace msngr::profile::repository