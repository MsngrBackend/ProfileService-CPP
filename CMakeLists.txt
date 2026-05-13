cmake_minimum_required(VERSION 3.20)
project(ProfileService VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find packages
find_package(Boost REQUIRED COMPONENTS system program_options)
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)

# For nlohmann_json
include(FetchContent)
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(nlohmann_json)

# For libpqxx
pkg_check_modules(PQXX REQUIRED libpqxx)

# For libpq
find_library(PQ_LIB pq)
find_path(PQ_INCLUDE_DIR libpq-fe.h)

# For libminio-cpp
find_library(MINIO_LIB minio)
find_path(MINIO_INCLUDE_DIR minio/client.h)

# Source files
set(SOURCES
  src/main.cpp
  src/server.cpp
  src/api_spec.cpp
  src/handlers/profile_handler.cpp
  src/handlers/contacts_handler.cpp
  src/handlers/privacy_handler.cpp
  src/handlers/favorites_handler.cpp
  src/handlers/notifications_handler.cpp
  src/handlers/handler_factory.cpp
  src/repository/profile_repository.cpp
  src/repository/contacts_repository.cpp
  src/repository/privacy_repository.cpp
  src/repository/favorites_repository.cpp
  src/repository/notifications_repository.cpp
  src/repository/minio_storage.cpp
)

# Create executable
add_executable(profile_service ${SOURCES})

# Include directories
target_include_directories(profile_service PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${Boost_INCLUDE_DIRS}
  ${PQXX_INCLUDE_DIRS}
  ${PQ_INCLUDE_DIR}
  ${MINIO_INCLUDE_DIR}
)

# Link libraries
target_link_libraries(profile_service
  ${Boost_LIBRARIES}
  ${PQXX_LIBRARIES}
  ${PQ_LIB}
  ${MINIO_LIB}
  Threads::Threads
  nlohmann_json::nlohmann_json
)

# Set compiler options
target_compile_options(profile_service PRIVATE -Wall -Wextra -O2)

# Install
install(TARGETS profile_service DESTINATION bin)