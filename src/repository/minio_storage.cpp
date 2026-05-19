#include "minio_storage.hpp"
#include <miniocpp/client.h>
#include <sstream>
#include <stdexcept>

namespace msngr::profile::repository {

    MinIOStorage::MinIOStorage(const std::string& endpoint, const std::string& access_key, const std::string& secret_key) {
        minio::s3::BaseUrl base_url(endpoint);
        base_url.https = false; // Use true in production

        minio::creds::StaticProvider provider(access_key, secret_key);
        client_ = std::make_unique<minio::s3::Client>(base_url, &provider);
    }

    MinIOStorage::~MinIOStorage() = default;

    std::string MinIOStorage::Upload(const std::string& user_id, const std::vector<uint8_t>& data, const std::string& content_type) {
        std::string object_name = user_id + "/avatar.jpg";

        minio::s3::MakeBucketArgs make_args;
        make_args.bucket = bucket_name_;
        client_->MakeBucket(make_args);

        std::string body(data.begin(), data.end());
        std::istringstream stream(body);

        minio::s3::PutObjectArgs put_args(stream, static_cast<long>(data.size()), 0);
        put_args.bucket = bucket_name_;
        put_args.object = object_name;
        put_args.content_type = content_type;

        // Client::PutObject принимает PutObjectArgs (не ApiArgs)
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

        client_->RemoveObject(remove_args);
    }

} // namespace msngr::profile::repository