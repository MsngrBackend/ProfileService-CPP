#ifndef MSNGR__PROFILE__APPLICATION_HPP_
#define MSNGR__PROFILE__APPLICATION_HPP_

#include "profile_service/http/http_server.hpp"
#include "profile_service/routing/router.hpp"
#include "profile_service/services/service_factory.hpp"
#include "profile_service/persistence/database_manager.hpp"
#include "profile_service/storage/storage_manager.hpp"
#include "profile_service/auth/auth_service.hpp"

#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <stop_token>

namespace msngr::profile {

class Application
{
public:
  struct Config
  {
    std::string Address = "0.0.0.0";
    uint16_t Port = 8082;
    std::string DatabaseUrl;
    std::string MinioEndpoint = "localhost:9000";
    std::string MinioAccessKey;
    std::string MinioSecretKey;
  };

  explicit Application(Config config);
  ~Application();

  Application(const Application &) = delete;
  Application& operator=(const Application&) = delete;
  Application(Application &&) = delete;
  Application& operator=(Application &&) = delete;

  bool Start();
  void Stop();
  inline bool IsRunning() const
  {
    return m_running.load();
  }

private:
  bool Initialize();
  void Shutdown();

  Config m_config;
  std::atomic<bool> m_running{false};
  std::jthread m_mainThread;

  std::unique_ptr<DatabaseManager> m_dbManager;
  std::unique_ptr<StorageManager> m_storageManager;
  std::unique_ptr<AuthService> m_authService;
  std::unique_ptr<ServiceFactory> m_serviceFactory;
  std::unique_ptr<RequestRouter> m_router;
  std::unique_ptr<HttpServer> m_httpServer;
};

} // namespace msngr::profile

#endif // MSNGR__PROFILE__APPLICATION_HPP_