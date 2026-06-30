#ifndef MSNGR__PROFILE__REPOSITORY__DB_CONNECTION_HPP_
#define MSNGR__PROFILE__REPOSITORY__DB_CONNECTION_HPP_

#include <pqxx/pqxx>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

namespace msngr::profile::repository {

class IDatabaseConnection {
public:
  virtual ~IDatabaseConnection() = default;
  virtual pqxx::connection & Connection() = 0;
  virtual bool IsOpen() const = 0;
};

class PostgresSQLConnection : public IDatabaseConnection {
public:
  explicit PostgresSQLConnection(std::string connection_string)
    : m_connectionString(std::move(connection_string)),
      m_connection(std::make_shared<pqxx::connection>(m_connectionString)) 
  {
    if (!m_connection->is_open()) {
      throw std::runtime_error("Failed to connect to database");
    }
  }

  pqxx::connection & Connection() override {
    return *m_connection;
  }

  bool IsOpen() const override {
    return m_connection && m_connection->is_open();
  }

private:
  std::string m_connectionString;
  std::shared_ptr<pqxx::connection> m_connection;
};

} // namespace msngr::profile::repository

#endif  // MSNGR__PROFILE__REPOSITORY__DB_CONNECTION_HPP_