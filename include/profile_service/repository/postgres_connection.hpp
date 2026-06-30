#ifndef MSNGR__PROFILE__REPOSITORY__POSTGRES_CONNECTION_HPP_
#define MSNGR__PROFILE__REPOSITORY__POSTGRES_CONNECTION_HPP_

#include "db_interface.hpp"
#include <pqxx/pqxx>
#include <memory>
#include <string>

namespace msngr::profile::repository {

class PostgresRow : public IRow {
public:
    explicit PostgresRow(const pqxx::row& row);
    
    bool IsNull(size_t index) const override;
    bool IsNull(const std::string& column) const override;
    DBValue GetValue(size_t index) const override;
    DBValue GetValue(const std::string& column) const override;
    size_t Size() const override;

private:
    pqxx::row m_row;
};

class PostgresResultSet : public IResultSet {
public:
    explicit PostgresResultSet(pqxx::result result);
    
    bool Empty() const override;
    size_t Size() const override;
    std::unique_ptr<IRow> GetRow(size_t index) const override;
    std::unique_ptr<IRow> GetRow() override;
    bool Next() override;

private:
    pqxx::result m_result;
    size_t m_currentIndex = 0;
};

class PostgresTransaction : public ITransaction {
public:
  explicit PostgresTransaction(pqxx::connection & conn);
  ~PostgresTransaction();
  
  std::unique_ptr<IResultSet> Execute(const std::string & query) override;
  std::unique_ptr<IResultSet> ExecuteParams(
    const std::string & query,
    const std::vector<DBValue> & params
  ) override;
  void Commit() override;
  void Rollback() override;

  // Make this public so PostgresConnection can use it
  static pqxx::result ExecuteWithParams(
    pqxx::work & txn,
    const std::string & query,
    const std::vector<DBValue>& params
  );

private:
  std::unique_ptr<pqxx::work> m_txn;
  bool m_committed = false;
  pqxx::connection& m_conn;

  static std::string ConvertValueToPQXX(const DBValue & value);
};

class PostgresConnection : public IDatabaseConnection {
public:
  explicit PostgresConnection(const std::string& connectionString);
  ~PostgresConnection();

  bool IsOpen() const override;
  std::unique_ptr<ITransaction> BeginTransaction() override;
  std::unique_ptr<IResultSet> Execute(const std::string & query) override;
  std::unique_ptr<IResultSet> ExecuteParams(
    const std::string & query,
    const std::vector<DBValue> & params
  ) override;
  void Close() override;

private:
  std::unique_ptr<pqxx::connection> m_connection;
  bool m_isOpen = false;
};

} // namespace msngr::profile::repository

#endif // MSNGR__PROFILE__REPOSITORY__POSTGRES_CONNECTION_HPP_