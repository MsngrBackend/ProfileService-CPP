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

# Use pkg-config for everything possible
pkg_check_modules(PQXX REQUIRED libpqxx)
pkg_check_modules(CURLPP libcurlpp)
pkg_check_modules(CURL REQUIRED libcurl)
pkg_check_modules(PUGIXML REQUIRED pugixml)
pkg_check_modules(INIH REQUIRED inih)

# libpq
find_library(PQ_LIB pq REQUIRED)
find_path(PQ_INCLUDE_DIR libpq-fe.h
    PATHS /usr/include/postgresql /usr/include
    REQUIRED
)

# minio-cpp
find_library(MINIO_LIB miniocpp
    PATHS /usr/local/lib /usr/lib
    REQUIRED
)
find_path(MINIO_INCLUDE_DIR miniocpp/client.h
    PATHS /usr/local/include /usr/include
    REQUIRED
)

# Try vcpkg if pkg-config didn't find curlpp
if(NOT CURLPP_FOUND)
    find_path(CURLPP_INCLUDE_DIR curlpp/Easy.hpp
        PATHS /vcpkg/installed/${CMAKE_HOST_SYSTEM_PROCESSOR}-linux/include
              /vcpkg/installed/arm64-linux/include
              /vcpkg/installed/aarch64-linux/include
              /usr/local/include
        REQUIRED
    )
    find_library(CURLPP_LIB curlpp
        PATHS /vcpkg/installed/${CMAKE_HOST_SYSTEM_PROCESSOR}-linux/lib
              /vcpkg/installed/arm64-linux/lib
              /vcpkg/installed/aarch64-linux/lib
              /usr/local/lib
        REQUIRED
    )
    add_library(CURLPP::curlpp UNKNOWN IMPORTED)
    set_target_properties(CURLPP::curlpp PROPERTIES
        IMPORTED_LOCATION ${CURLPP_LIB}
        INTERFACE_INCLUDE_DIRECTORIES ${CURLPP_INCLUDE_DIR}
    )
endif()

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
  ${CURLPP_INCLUDE_DIRS}
  ${CURL_INCLUDE_DIRS}
  ${PUGIXML_INCLUDE_DIRS}
  ${INIH_INCLUDE_DIRS}
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
  dl
)

if(CURLPP_FOUND)
    target_link_libraries(profile_service ${CURLPP_LIBRARIES} ${CURL_LIBRARIES})
else()
    target_link_libraries(profile_service CURLPP::curlpp ${CURL_LIBRARIES})
endif()

if(PUGIXML_FOUND)
    target_link_libraries(profile_service ${PUGIXML_LIBRARIES})
    target_include_directories(profile_service PRIVATE ${PUGIXML_INCLUDE_DIRS})
endif()

if(INIH_FOUND)
    target_link_libraries(profile_service ${INIH_LIBRARIES})
    target_include_directories(profile_service PRIVATE ${INIH_INCLUDE_DIRS})
endif()

target_compile_options(profile_service PRIVATE -Wall -Wextra -O2)
install(TARGETS profile_service DESTINATION bin)
