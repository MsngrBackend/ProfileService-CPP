#include "minio_storage.hpp"
#include <stdexcept>

namespace msngr::profile::repository {

MinIOStorage::MinIOStorage(const std::string& endpoint, const std::string& access_key, const std::string& secret_key) {
    endpoint_ = endpoint;
    access_key_ = access_key;
    secret_key_ = secret_key;
}

MinIOStorage::~MinIOStorage() = default;

std::string MinIOStorage::Upload(const std::string& user_id, const std::vector<uint8_t>& data, const std::string& content_type) {
    if (data.empty()) {
        throw std::runtime_error("Avatar payload is empty");
    }

    std::string extension = "jpg";
    if (content_type == "image/png") {
        extension = "png";
    } else if (content_type == "image/webp") {
        extension = "webp";
    }

    std::string base_url = endpoint_.empty() ? "http://localhost:9000" : endpoint_;
    if (base_url.rfind("http://", 0) != 0 && base_url.rfind("https://", 0) != 0) {
        base_url = "http://" + base_url;
    }

    // Temporary lightweight implementation for local testing in Docker.
    return base_url + "/" + std::string(bucket_name_) + "/" + user_id + "/avatar." + extension;
}

void MinIOStorage::Delete(const std::string& user_id) {
    (void)user_id;
}

} // namespace msngr::profile::repository
