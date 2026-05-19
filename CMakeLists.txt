cmake_minimum_required(VERSION 3.20)

project(ProfileService VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# =========================================================
# Build type
# =========================================================

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# =========================================================
# Dependencies
# =========================================================

find_package(Threads REQUIRED)

find_package(Boost REQUIRED COMPONENTS
    system
    program_options
    log
    log_setup
    thread
    filesystem
    regex
    date_time
)

find_package(OpenSSL REQUIRED)

find_package(PkgConfig REQUIRED)

# =========================================================
# JSON
# =========================================================

include(FetchContent)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)

FetchContent_MakeAvailable(nlohmann_json)

# =========================================================
# pkg-config packages
# =========================================================

pkg_check_modules(PQXX REQUIRED libpqxx)

pkg_check_modules(PUGIXML REQUIRED pugixml)

pkg_check_modules(INIH REQUIRED inih)

pkg_check_modules(CURL REQUIRED libcurl)

pkg_check_modules(CURLPP REQUIRED libcurlpp)

# =========================================================
# PostgreSQL client library
# =========================================================

find_library(PQ_LIB pq REQUIRED)

find_path(PQ_INCLUDE_DIR libpq-fe.h
    PATHS
        /usr/include/postgresql
        /usr/include
    REQUIRED
)

# =========================================================
# MinIO C++
# =========================================================

find_library(MINIO_LIB miniocpp
    PATHS
        /usr/local/lib
        /usr/lib
    REQUIRED
)

find_path(MINIO_INCLUDE_DIR miniocpp/client.h
    PATHS
        /usr/local/include
        /usr/include
    REQUIRED
)

# =========================================================
# Sources
# =========================================================

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

# =========================================================
# Executable
# =========================================================

add_executable(profile_service ${SOURCES})

# =========================================================
# Includes
# =========================================================

target_include_directories(profile_service PRIVATE

    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src

    ${Boost_INCLUDE_DIRS}

    ${PQXX_INCLUDE_DIRS}
    ${PQ_INCLUDE_DIR}

    ${MINIO_INCLUDE_DIR}

    ${PUGIXML_INCLUDE_DIRS}

    ${INIH_INCLUDE_DIRS}

    ${CURL_INCLUDE_DIRS}

    ${CURLPP_INCLUDE_DIRS}
)

# =========================================================
# Linking
# =========================================================

target_link_libraries(profile_service PRIVATE

    # Boost
    ${Boost_LIBRARIES}

    # Threads
    Threads::Threads

    # OpenSSL
    OpenSSL::SSL
    OpenSSL::Crypto

    # PostgreSQL
    ${PQXX_LIBRARIES}
    ${PQ_LIB}

    # XML
    ${PUGIXML_LIBRARIES}

    # inih
    ${INIH_LIBRARIES}

    # Curl
    ${CURL_LIBRARIES}

    # curlpp
    ${CURLPP_LIBRARIES}

    # MinIO
    ${MINIO_LIB}

    # JSON
    nlohmann_json::nlohmann_json

    # Linux libs
    z
    dl
)

# =========================================================
# Compiler flags
# =========================================================

target_compile_options(profile_service PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -O2
    -Wno-unused-parameter
)

# =========================================================
# Install
# =========================================================

install(TARGETS profile_service DESTINATION bin)