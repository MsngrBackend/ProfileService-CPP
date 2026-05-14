cmake_minimum_required(VERSION 3.20)
project(ProfileService VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Boost REQUIRED COMPONENTS system program_options log log_setup thread filesystem regex date_time)
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)

include(FetchContent)
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(nlohmann_json)

pkg_check_modules(PQXX REQUIRED libpqxx)

# libpq — ищем явно
find_library(PQ_LIB pq REQUIRED)
find_path(PQ_INCLUDE_DIR libpq-fe.h
    PATHS /usr/include/postgresql /usr/include
    REQUIRED
)

# minio-cpp — установлен через cmake --install, лежит в /usr/local
find_library(MINIO_LIB miniocpp
    PATHS /usr/local/lib
    REQUIRED
)
find_path(MINIO_INCLUDE_DIR miniocpp/client.h
    PATHS /usr/local/include
    REQUIRED
)

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

add_executable(profile_service ${SOURCES})

target_include_directories(profile_service PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/include/profile_service        # добавить
  ${CMAKE_CURRENT_SOURCE_DIR}/include/profile_service/handlers  # добавить
  ${CMAKE_CURRENT_SOURCE_DIR}/include/profile_service/repository # добавить
  ${CMAKE_CURRENT_SOURCE_DIR}/include/profile_service/domain    # добавить
  ${CMAKE_CURRENT_SOURCE_DIR}/include/profile_service/utils     # добавить
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${Boost_INCLUDE_DIRS}
  ${PQXX_INCLUDE_DIRS}
  ${PQ_INCLUDE_DIR}
  ${MINIO_INCLUDE_DIR}
  /vcpkg/installed/x64-linux/include
)

target_link_libraries(profile_service
  ${Boost_LIBRARIES}
  ${PQXX_LIBRARIES}
  ${PQ_LIB}
  ${MINIO_LIB}
  Threads::Threads
  nlohmann_json::nlohmann_json
  z
  boost_log
  boost_log_setup
  boost_thread
  boost_filesystem
  boost_regex
  boost_date_time
  /vcpkg/installed/x64-linux/lib/libcurlpp.a
  /vcpkg/installed/x64-linux/lib/libcurl.a
  /vcpkg/installed/x64-linux/lib/libssl.a
  /vcpkg/installed/x64-linux/lib/libcrypto.a
  /vcpkg/installed/x64-linux/lib/libpugixml.a
  /vcpkg/installed/x64-linux/lib/libINIReader.a
  /vcpkg/installed/x64-linux/lib/libinih.a
  dl
)

target_compile_options(profile_service PRIVATE -Wall -Wextra -O2)
install(TARGETS profile_service DESTINATION bin)