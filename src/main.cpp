#include <signal.h>
#include <pthread.h>
#include "profile_service/server.hpp"
#include "utils/logger.hpp"

#include <boost/program_options.hpp>
#include <csignal>
#include <cstdlib>
#include <memory>
#include <iostream>

namespace po = boost::program_options;

int main(int argc, char** argv) 
{
  try {
    std::string address = "0.0.0.0";
    uint16_t port = 8082;

    po::options_description options("Profile Service options");
    options.add_options()
      ("help,h", "Show help")
      ("address,a", po::value<std::string>(&address)->default_value(address), "Bind address")
      ("port,p", po::value<uint16_t>(&port)->default_value(port), "Bind port");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, options), vm);
    po::notify(vm);

    if (vm.count("help")) {
      std::cout << options << std::endl;
      return 0;
    }

    // Check if DATABASE_URL is set
    const char* db_url = std::getenv("DATABASE_URL");
    if (!db_url) {
      LOG(error) << "DATABASE_URL environment variable not set";
      return 1;
    }

    msngr::profile::Endpoint endpoint;
    endpoint.Address = address;
    endpoint.Port = port;

    msngr::profile::Server server(endpoint);

    if (!server.Start()) {
      LOG(error) << "Failed to start profile service server";
      return 1;
    }

    LOG(info) << "Profile service started at http://" << address << ":" << port;

    // Wait for signals
    sigset_t waitSet;
    sigemptyset(&waitSet);
    sigaddset(&waitSet, SIGINT);
    sigaddset(&waitSet, SIGTERM);
    pthread_sigmask(SIG_BLOCK, &waitSet, nullptr);

    int sig = 0;
    sigwait(&waitSet, &sig);

    server.Stop();
    return 0;
  } catch (const std::exception & e) {
    LOG(error) << "Fatal error: " << e.what();
    return 1;
  }
}