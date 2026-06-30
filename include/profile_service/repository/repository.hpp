#ifndef MSNGR__PROFILE__REPOSITORY__REPOSITORY_HPP_
#define MSNGR__PROFILE__REPOSITORY__REPOSITORY_HPP_

#include "profile_service/domain/models.hpp"
#include <memory>
#include <optional>
#include <vector>

namespace msngr::profile::repository {

class ProfileRepository {
public:
    virtual ~ProfileRepository() = default;
    virtual domain::Profile Create(const std::string & userId) = 0;
    virtual domain::Profile GetByID(const std::string & userId) = 0;
    virtual void Update(const domain::Profile & profile) = 0;
    virtual void UpdateAvatarURL(const std::string & userId, const std::string & url) = 0;
};

class ContactsRepository {
public:
    virtual ~ContactsRepository() = default;
    virtual std::vector<domain::Contact> List(const std::string & ownerId) = 0;
    virtual void Add(const domain::Contact & contact) = 0;
    virtual void Remove(const std::string & ownerId, const std::string& contactId) = 0;
};

class PrivacyRepository {
public:
    virtual ~PrivacyRepository() = default;
    virtual domain::PrivacySettings Get(const std::string & userId) = 0;
    virtual void Update(const domain::PrivacySettings & settings) = 0;
};

class NotificationRepository {
public:
    virtual ~NotificationRepository() = default;
    virtual domain::NotificationSettings Get(const std::string & userId, const std::optional<std::string> & chatId) = 0;
    virtual void Upsert(const domain::NotificationSettings & settings) = 0;
};

class FavoriteRepository {
public:
    virtual ~FavoriteRepository() = default;
    virtual std::vector<domain::Favorite> List(const std::string & userId) = 0;
    virtual void Add(const std::string & userId, const std::string & chatId) = 0;
    virtual void Remove(const std::string & userId, const std::string & chatId) = 0;
};

class AvatarStorage {
public:
    virtual ~AvatarStorage() = default;
    virtual std::string Upload(const std::string & userId, const std::vector<uint8_t> & data, const std::string & content_type) = 0;
    virtual void Delete(const std::string & userId) = 0;
};

} // namespace msngr::profile::repository

#endif  // MSNGR__PROFILE__REPOSITORY__REPOSITORY_HPP_
