cmake_minimum_required(VERSION 3.20)
project(ProfileService VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Boost REQUIRED COMPONENTS system program_options log log_setup thread filesystem regex date_time)
find_package(Threads REQUIRED)

include(FetchContent)
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(nlohmann_json)

# For vcpkg-installed packages
find_package(PkgConfig REQUIRED)

# Set pkg-config paths for vcpkg
if(DEFINED ENV{VCPKG_ROOT})
    set(VCPKG_LIB_DIR "$ENV{VCPKG_ROOT}/installed/${CMAKE_HOST_SYSTEM_PROCESSOR}-linux/lib")
    set(VCPKG_PKG_CONFIG_DIR "$ENV{VCPKG_ROOT}/installed/${CMAKE_HOST_SYSTEM_PROCESSOR}-linux/lib/pkgconfig")
    set(ENV{PKG_CONFIG_PATH} "${VCPKG_PKG_CONFIG_DIR}:$ENV{PKG_CONFIG_PATH}")
    link_directories(${VCPKG_LIB_DIR})
endif()

pkg_check_modules(PQXX REQUIRED libpqxx)
pkg_check_modules(PUGIXML REQUIRED pugixml)
pkg_check_modules(INIH REQUIRED inih)
pkg_check_modules(CURLPP REQUIRED libcurlpp)
pkg_check_modules(CURL REQUIRED libcurl)

# libpq
find_library(PQ_LIB pq REQUIRED)
find_path(PQ_INCLUDE_DIR libpq-fe.h
    PATHS /usr/include/postgresql /usr/include
    REQUIRED
)

# minio-cpp
find_library(MINIO_LIB miniocpp
    PATHS /usr/local/lib /usr/lib ${VCPKG_LIB_DIR}
    REQUIRED
)
find_path(MINIO_INCLUDE_DIR miniocpp/client.h
    PATHS /usr/local/include /usr/include $ENV{VCPKG_ROOT}/installed/${CMAKE_HOST_SYSTEM_PROCESSOR}-linux/include
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
  ${CMAKE_CURRENT_SOURCE_DIR}/include/profile_service
  ${CMAKE_CURRENT_SOURCE_DIR}/include/profile_service/handlers
  ${CMAKE_CURRENT_SOURCE_DIR}/include/profile_service/repository
  ${CMAKE_CURRENT_SOURCE_DIR}/include/profile_service/domain
  ${CMAKE_CURRENT_SOURCE_DIR}/include/profile_service/utils
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${Boost_INCLUDE_DIRS}
  ${PQXX_INCLUDE_DIRS}
  ${PQ_INCLUDE_DIR}
  ${MINIO_INCLUDE_DIR}
  ${PUGIXML_INCLUDE_DIRS}
  ${INIH_INCLUDE_DIRS}
  ${CURLPP_INCLUDE_DIRS}
  ${CURL_INCLUDE_DIRS}
)

target_link_libraries(profile_service
  ${Boost_LIBRARIES}
  ${PQXX_LIBRARIES}
  ${PQ_LIB}
  ${MINIO_LIB}
  ${PUGIXML_LIBRARIES}
  ${INIH_LIBRARIES}
  ${CURLPP_LIBRARIES}
  ${CURL_LIBRARIES}
  Threads::Threads
  nlohmann_json::nlohmann_json
  z
  dl
)

target_compile_options(profile_service PRIVATE -Wall -Wextra -O2 -Wno-unused-parameter)
install(TARGETS profile_service DESTINATION bin)
