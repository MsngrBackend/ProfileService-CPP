#include "profile_service/auth/auth_service.hpp"

namespace msngr::profile {

bool AuthService::Check(const std::string & userId, const std::string & scope) const {
  // Simple implementation - just check that userId is not empty
  // In production, this would validate with a token service
  (void)scope; // Unused in simple implementation
  return !userId.empty();
}

} // namespace msngr::profile