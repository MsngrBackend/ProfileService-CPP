#include "profile_service/application.hpp"
#include "profile_service/utils/logger.hpp"
#include "profile_service/api_spec.hpp"

namespace msngr::profile {

Application::Application(Config config) : m_config(std::move(config)) {}

Application::~Application(){
  Stop();
}

bool Application::Start()
{
  if (m_running.exchange(true)) {
    LOG(error) << "Application already running";
    return false;
  }

  m_mainThread = std::jthread([this](std::stop_token stopToken) {
    if (!Initialize()) {
      m_running.store(false);
      LOG(error) << "Failed to initialize application";
      return;
    }

    LOG(info) << "Application started successfully";
    m_httpServer->Run(stopToken);
    Shutdown();
  });

  return true;
}

void Application::Stop()
{
  if (!m_running.exchange(false)) {
      return;
  }

  if (m_httpServer) {
      m_httpServer->Stop();
  }

  m_mainThread.request_stop();
  if (m_mainThread.joinable()) {
      m_mainThread.join();
  }

  LOG(info) << "Application stopped";
}

bool Application::Initialize()
{
  try {
    LOG(info) << "Initializing DatabaseManager...";
    m_dbManager = std::make_unique<DatabaseManager>(m_config.DatabaseUrl);
    auto repos = m_dbManager->GetRepositories();

    LOG(info) << "Initializing StorageManager...";
    m_storageManager = std::make_unique<StorageManager>(
      m_config.MinioEndpoint,
      m_config.MinioAccessKey,
      m_config.MinioSecretKey
    );
    auto storage = m_storageManager->GetStorage();

    LOG(info) << "Initializing AuthService...";
    m_authService = std::make_unique<AuthService>();

    LOG(info) << "Initializing ServiceFactory...";
    m_serviceFactory = std::make_unique<ServiceFactory>(repos, storage);

    LOG(info) << "Initializing RequestRouter...";
    m_router = std::make_unique<RequestRouter>(
      m_serviceFactory->GetServices(),
      m_authService.get()
    );
    m_router->BuildRoutes();

    LOG(info) << "Initializing HttpServer...";
    m_httpServer = std::make_unique<HttpServer>(
      m_config.Address,
      m_config.Port,
      [this](const HttpRequest& req) {
          return m_router->Route(req);
      }
    );

    LOG(info) << "Application initialized successfully";
    return true;

  } catch (const std::exception& e) {
    LOG(error) << "Failed to initialize application: " << e.what();
    return false;
  }
}

void Application::Shutdown() {
  LOG(info) << "Shutting down application...";
  m_httpServer.reset();
  m_router.reset();
  m_serviceFactory.reset();
  m_authService.reset();
  m_storageManager.reset();
  m_dbManager.reset();
  LOG(info) << "Application shutdown complete";
}

} // namespace msngr::profile