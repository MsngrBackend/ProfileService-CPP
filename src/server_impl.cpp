#include "profile_service/server_impl.hpp"
#include "profile_service/server.hpp"
#include "profile_service/query_manager.hpp"
#include "profile_service/request_manager.hpp"
#include "profile_service/auth_validator.hpp"
#include "profile_service/handlers/handler_factory.hpp"
#include "profile_service/utils/logger.hpp"

namespace msngr::profile {

ServerImpl::ServerImpl(const Endpoint & endpoint)
  : m_endpoint(endpoint) {
  InitializeComponents();
}

ServerImpl::~ServerImpl() = default;

void ServerImpl::InitializeComponents() {
  try {
    m_queryManager = std::make_shared<QueryManager>();

    m_handlerFactory = std::make_shared<handlers::HandlerFactory>(
      m_queryManager->GetProfileRepository(),
      m_queryManager->GetContactsRepository(),
      m_queryManager->GetPrivacyRepository(),
      m_queryManager->GetFavoriteRepository(),
      m_queryManager->GetNotificationRepository(),
      m_queryManager->GetAvatarStorage()
    );

    m_authChecker = std::make_shared<SimpleAuthChecker>();

    m_requestManager = std::make_shared<RequestManager>(
      m_queryManager,
      m_handlerFactory,
      m_authChecker
    );

    LOG(info) << "Server components initialized successfully";
  } catch (const std::exception & e) {
    LOG(error) << "Failed to initialize server components: " << e.what();
    throw;
  }
}

beast_http::response<beast_http::string_body> ServerImpl::Handle(
  const beast_http::request<beast_http::string_body>& request) {
  return m_requestManager->Handle(request);
}

} // namespace msngr::profile
