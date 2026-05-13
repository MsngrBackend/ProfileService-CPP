#ifndef MSNGR__PROFILE__REPOSITORY__MINIO_STORAGE_HPP_
#define MSNGR__PROFILE__REPOSITORY__MINIO_STORAGE_HPP_

#include "repository.hpp"
#include <minio/client.h>
#include <memory>
#include <string>
#include <vector>

namespace msngr::profile::repository {

class MinIOStorage : public AvatarStorage {
public:
    MinIOStorage(const std::string& endpoint, const std::string& access_key, const std::string& secret_key);
    ~MinIOStorage();
    
    std::string Upload(const std::string& user_id, const std::vector<uint8_t>& data, const std::string& content_type) override;
    void Delete(const std::string& user_id) override;
    
private:
    std::unique_ptr<minio::s3::BaseClient> client_;
    static constexpr const char* bucket_name_ = "avatars";
};

} // namespace msngr::profile::repository

#endif  // MSNGR__PROFILE__REPOSITORY__MINIO_STORAGE_HPP_