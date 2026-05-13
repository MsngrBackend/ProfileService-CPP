#include "minio_storage.hpp"
#include <minio/client.h>
#include <sstream>
#include <stdexcept>

namespace msngr::profile::repository {

MinIOStorage::MinIOStorage(const std::string& endpoint, const std::string& access_key, const std::string& secret_key) {
    client_ = std::make_unique<minio::s3::BaseClient>(
        endpoint, access_key, secret_key, 
        minio::s3::Protocol::kHttp  // Use HTTPS in production
    );
}

MinIOStorage::~MinIOStorage() = default;

std::string MinIOStorage::Upload(const std::string& user_id, const std::vector<uint8_t>& data, const std::string& content_type) {
    std::string object_name = user_id + "/avatar.jpg";
    
    // Create bucket if not exists
    minio::s3::MakeBucketArgs make_args;
    make_args.bucket = bucket_name_;
    auto make_resp = client_->MakeBucket(make_args);
    // Ignore bucket already exists error
    
    // Upload object
    minio::s3::PutObjectArgs put_args;
    put_args.bucket = bucket_name_;
    put_args.object = object_name;
    put_args.content_type = content_type;
    put_args.body = std::string(data.begin(), data.end());
    
    auto put_resp = client_->PutObject(put_args);
    if (!put_resp) {
        throw std::runtime_error("Failed to upload avatar: " + put_resp.Error().String());
    }
    
    return "/" + std::string(bucket_name_) + "/" + object_name;
}

void MinIOStorage::Delete(const std::string& user_id) {
    std::string object_name = user_id + "/avatar.jpg";
    
    minio::s3::RemoveObjectArgs remove_args;
    remove_args.bucket = bucket_name_;
    remove_args.object = object_name;
    
    auto resp = client_->RemoveObject(remove_args);
    // Ignore errors for non-existent objects
}

} // namespace msngr::profile::repository