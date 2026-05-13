#ifndef MSNGR__PROFILE__REPOSITORY__DB_CONNECTION_HPP_
#define MSNGR__PROFILE__REPOSITORY__DB_CONNECTION_HPP_

#include <pqxx/pqxx>
#include <memory>
#include <string>

namespace msngr::profile::repository {

class DBConnection {
public:
    static std::shared_ptr<pqxx::connection> Create(const std::string& connection_string) {
        auto conn = std::make_shared<pqxx::connection>(connection_string);
        if (!conn->is_open()) {
            throw std::runtime_error("Failed to connect to database");
        }
        return conn;
    }
};

} // namespace msngr::profile::repository

#endif  // MSNGR__PROFILE__REPOSITORY__DB_CONNECTION_HPP_