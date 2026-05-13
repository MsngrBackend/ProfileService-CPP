#ifndef MSNGR__PROFILE__SERVER_HPP_
#define MSNGR__PROFILE__SERVER_HPP_

#include <memory>
#include <string>
#include <atomic>
#include <thread>
#include <stop_token>

namespace msngr::profile {

struct Endpoint {
  std::string Address = "127.0.0.1";
  uint16_t Port = 8082;
};

class Server {
public:
  Server(std::shared_ptr<void> repository, Endpoint endpoint);
  ~Server();

  bool Start();
  void Stop();

private:
  class Impl;
  std::unique_ptr<Impl> m_impl;
  std::shared_ptr<void> m_repository;
  Endpoint m_endpoint;
  std::atomic<bool> m_running{false};
  std::jthread m_thread;
};

} // namespace msngr::profile

#endif  // MSNGR__PROFILE__SERVER_HPP_
