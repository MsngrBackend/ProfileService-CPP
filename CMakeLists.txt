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
set(PROFILE_SERVICE_SOURCES
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

set(MAIN_SOURCE src/main.cpp)

# Header directories
include_directories(include)

add_compile_options(-Wall -Wextra -Wpedantic -O2)

# Create library target
add_library(profile_service_lib STATIC ${PROFILE_SERVICE_SOURCES})
target_include_directories(profile_service_lib PRIVATE include)
target_link_libraries(profile_service_lib PUBLIC
    Boost::system
    Boost::thread
    Boost::log
    Boost::log_setup
    Boost::program_options
    ${PQXX_LIBRARIES}
    nlohmann_json::nlohmann_json
    pthread
)

# Create executable
add_executable(profile_service ${MAIN_SOURCE})
target_include_directories(profile_service PRIVATE include)
target_link_libraries(profile_service PRIVATE profile_service_lib)

# Link libraries for tests

# Test configuration
find_package(Boost REQUIRED COMPONENTS unit_test_framework)
find_package(Catch2 REQUIRED)
find_package(GTest REQUIRED)

enable_testing()

add_executable(api_spec_catch2 tests/api_spec_catch2.cpp)
target_include_directories(api_spec_catch2 PRIVATE include)
target_link_libraries(api_spec_catch2 PRIVATE profile_service_lib Catch2::Catch2WithMain)
add_test(NAME ApiSpecCatch2 COMMAND api_spec_catch2)

add_executable(api_spec_boost tests/api_spec_boost.cpp)
target_include_directories(api_spec_boost PRIVATE include)
target_link_libraries(api_spec_boost PRIVATE profile_service_lib Boost::unit_test_framework)
add_test(NAME ApiSpecBoost COMMAND api_spec_boost)

add_executable(api_spec_gtest tests/api_spec_gtest.cpp)
target_include_directories(api_spec_gtest PRIVATE include)
target_link_libraries(api_spec_gtest PRIVATE profile_service_lib GTest::gtest GTest::gtest_main pthread)
add_test(NAME ApiSpecGTest COMMAND api_spec_gtest)

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