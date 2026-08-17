#include "profile_service/application.hpp"
#include "profile_service/utils/logger.hpp"

#include <boost/program_options.hpp>
#include <csignal>
#include <cstdlib>
#include <iostream>

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#include <pthread.h>

namespace po = boost::program_options;

int main(int argc, char** argv) {
  try {
    msngr::profile::Application::Config config;

    po::options_description options("Profile Service options");
    options.add_options()
      ("help,h", "Show help")
      ("address,a", po::value<std::string>(&config.Address)->default_value("0.0.0.0"), "Bind address")
      ("port,p", po::value<uint16_t>(&config.Port)->default_value(8082), "Bind port")
      ("tls", po::value<bool>(&config.EnableTls)->default_value(false), "Enable TLS/HTTPS")
      ("cert", po::value<std::string>(&config.TlsCertFile)->default_value("server.crt"), "TLS certificate file")
      ("key", po::value<std::string>(&config.TlsKeyFile)->default_value("server.key"), "TLS private key file")
      ("dhparam", po::value<std::string>(&config.TlsDhFile)->default_value(""), "TLS DH parameters file");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, options), vm);
    po::notify(vm);

    if (vm.count("help")) {
      std::cout << options << std::endl;
      return 0;
    }

    // Check required environment variables
    const char* dbUrl = std::getenv("DATABASE_URL");
    if (!dbUrl) {
      LOG(error) << "DATABASE_URL environment variable not set";
      return 1;
    }
    config.DatabaseUrl = dbUrl;

    // Optional environment variables
    const char* minioEndpoint = std::getenv("MINIO_ENDPOINT");
    if (minioEndpoint) {
      config.MinioEndpoint = minioEndpoint;
    }

    const char* minioAccess = std::getenv("MINIO_ACCESS_KEY");
    if (minioAccess) {
      config.MinioAccessKey = minioAccess;
    }

    const char* minioSecret = std::getenv("MINIO_SECRET_KEY");
    if (minioSecret) {
      config.MinioSecretKey = minioSecret;
    }

    LOG(info) << "Starting Profile Service...";
    LOG(info) << "  Address: " << config.Address;
    LOG(info) << "  Port: " << config.Port;
    LOG(info) << "  TLS: " << (config.EnableTls ? "enabled" : "disabled");
    LOG(info) << "  Database: " << config.DatabaseUrl;
    LOG(info) << "  MinIO: " << config.MinioEndpoint;

    msngr::profile::Application app(std::move(config));

    if (!app.Start()) {
      LOG(error) << "Failed to start application";
      return 1;
    }

    LOG(info) << "Profile service started successfully";

    // Wait for signals
    sigset_t waitSet;
    sigemptyset(&waitSet);
    sigaddset(&waitSet, SIGINT);
    sigaddset(&waitSet, SIGTERM);
    pthread_sigmask(SIG_BLOCK, &waitSet, nullptr);

    int sig = 0;
    sigwait(&waitSet, &sig);

    LOG(info) << "Received signal " << sig << ", shutting down...";

    app.Stop();
    LOG(info) << "Profile service stopped";
    return 0;

  } catch (const std::exception& e) {
    LOG(error) << "Fatal error: " << e.what();
    return 1;
  }
}