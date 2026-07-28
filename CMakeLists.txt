cmake_minimum_required(VERSION 3.14)
project(profile_service VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find required packages
find_package(Boost REQUIRED COMPONENTS 
    system 
    thread 
    log 
    log_setup
    program_options
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)
find_package(nlohmann_json REQUIRED)

# Source files
set(SOURCES
    src/main.cpp
    src/application.cpp
    src/api_spec.cpp
    src/http/http_server.cpp
    src/routing/router.cpp
    src/auth/auth_service.cpp
    src/services/service_factory.cpp
    src/services/profile_service.cpp
    src/services/contacts_service.cpp
    src/services/privacy_service.cpp
    src/services/favorites_service.cpp
    src/services/notifications_service.cpp
    src/persistence/database_manager.cpp
    src/persistence/query_executor.cpp
    src/persistence/postgres_connection.cpp
    src/persistence/repositories/profile_repository.cpp
    src/persistence/repositories/contacts_repository.cpp
    src/persistence/repositories/privacy_repository.cpp
    src/persistence/repositories/favorites_repository.cpp
    src/persistence/repositories/notifications_repository.cpp
    src/persistence/repositories/mappers.cpp
    src/storage/storage_manager.cpp
    src/storage/minio_storage.cpp
)

# Header directories
include_directories(include)

# Create executable
add_executable(profile_service ${SOURCES})

# Link libraries
target_link_libraries(profile_service
    ${Boost_LIBRARIES}
    ${PQXX_LIBRARIES}
    nlohmann_json::nlohmann_json
    pthread
    boost_log
    boost_log_setup
)

# Compiler flags
target_compile_options(profile_service PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -O2
)

# Copy compile commands for clangd (optional)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Print summary
message(STATUS "")
message(STATUS "Profile Service Build Configuration:")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Boost Include: ${Boost_INCLUDE_DIRS}")
message(STATUS "  Boost Libraries: ${Boost_LIBRARIES}")
message(STATUS "  PostgreSQL: ${PQXX_LIBRARIES}")
message(STATUS "  nlohmann_json: ${nlohmann_json_INCLUDE_DIRS}")
message(STATUS "")
message(STATUS "Build with: make -j$(nproc)")
message(STATUS "Run: ./profile_service --address 0.0.0.0 --port 8082")
message(STATUS "")