#ifndef MSNGR__PROFILE__STORAGE__AVATAR_STORAGE_HPP_
#define MSNGR__PROFILE__STORAGE__AVATAR_STORAGE_HPP_

#include <cstdint>
#include <string>
#include <vector>

namespace msngr::profile::repository {

class AvatarStorage {
public:
  virtual ~AvatarStorage() = default;

  virtual std::string Upload(
    const std::string & userId,
    const std::vector<uint8_t> & data,
    const std::string & contentType
  ) = 0;

  virtual void Delete(const std::string & userId) = 0;
};

} // namespace msngr::profile::repository

#endif // MSNGR__PROFILE__STORAGE__AVATAR_STORAGE_HPP_