#include "profile_service/persistence/postgres_connection.hpp"
#include <stdexcept>
#include <sstream>
#include <string>

namespace msngr::profile::repository {

// ==================== PostgresRow ====================

PostgresRow::PostgresRow(const pqxx::row& row) : m_row(row) {}

bool PostgresRow::IsNull(size_t index) const {
    return m_row[static_cast<int>(index)].is_null();
}

bool PostgresRow::IsNull(const std::string& column) const {
    return m_row[column].is_null();
}

DBValue PostgresRow::GetValue(size_t index) const {
    const auto& field = m_row[static_cast<int>(index)];
    if (field.is_null()) {
        return nullptr;
    }

    try {
        pqxx::oid typeOid = field.type();
        
        if (typeOid == 16) { // bool
            return field.as<bool>();
        } else if (typeOid == 20 || typeOid == 21 || typeOid == 23) { // int8, int2, int4
            return field.as<int64_t>();
        } else if (typeOid == 700 || typeOid == 701) { // float4, float8
            return field.as<double>();
        } else if (typeOid == 25 || typeOid == 1043 || typeOid == 1042) { // text, varchar, char
            return field.as<std::string>();
        } else if (typeOid == 17) { // bytea
            pqxx::binarystring bs(field);
            std::vector<uint8_t> data(bs.begin(), bs.end());
            return data;
        } else {
            return field.as<std::string>();
        }
    } catch (const std::exception&) {
        return field.as<std::string>();
    }
}

DBValue PostgresRow::GetValue(const std::string& column) const {
    const auto& field = m_row[column];
    if (field.is_null()) {
        return nullptr;
    }

    try {
        pqxx::oid typeOid = field.type();
        
        if (typeOid == 16) {
            return field.as<bool>();
        } else if (typeOid == 20 || typeOid == 21 || typeOid == 23) {
            return field.as<int64_t>();
        } else if (typeOid == 700 || typeOid == 701) {
            return field.as<double>();
        } else if (typeOid == 25 || typeOid == 1043 || typeOid == 1042) {
            return field.as<std::string>();
        } else if (typeOid == 17) {
            pqxx::binarystring bs(field);
            std::vector<uint8_t> data(bs.begin(), bs.end());
            return data;
        } else {
            return field.as<std::string>();
        }
    } catch (const std::exception&) {
        return field.as<std::string>();
    }
}

size_t PostgresRow::Size() const {
    return m_row.size();
}

// ==================== PostgresResultSet ====================

PostgresResultSet::PostgresResultSet(pqxx::result result)
    : m_result(std::move(result)) {}

bool PostgresResultSet::Empty() const {
    return m_result.empty();
}

size_t PostgresResultSet::Size() const {
    return m_result.size();
}

std::unique_ptr<IRow> PostgresResultSet::GetRow(size_t index) const {
    if (index >= m_result.size()) {
        throw std::out_of_range("Row index out of range");
    }
    return std::make_unique<PostgresRow>(m_result[index]);
}

std::unique_ptr<IRow> PostgresResultSet::GetRow() {
    if (m_currentIndex >= m_result.size()) {
        return nullptr;
    }
    return std::make_unique<PostgresRow>(m_result[m_currentIndex++]);
}

bool PostgresResultSet::Next() {
    return ++m_currentIndex < m_result.size();
}


PostgresTransaction::PostgresTransaction(pqxx::connection& conn)
    : m_conn(conn) {
    m_txn = std::make_unique<pqxx::work>(conn);
}

PostgresTransaction::~PostgresTransaction() {
    if (!m_committed && m_txn) {
        try {
            m_txn->abort();
        } catch (...) {}
    }
}

std::string PostgresTransaction::ConvertValueToPQXX(const DBValue& value) {
  return std::visit([](auto&& arg) -> std::string {
      using T = std::decay_t<decltype(arg)>;
      if constexpr (std::is_same_v<T, std::nullptr_t>) {
          return "";
      } else if constexpr (std::is_same_v<T, bool>) {
          return arg ? "true" : "false";
      } else if constexpr (std::is_same_v<T, int64_t>) {
          return std::to_string(arg);
      } else if constexpr (std::is_same_v<T, double>) {
          return std::to_string(arg);
      } else if constexpr (std::is_same_v<T, std::string>) {
          return arg;
      } else if constexpr (std::is_same_v<T, std::vector<uint8_t>>) {
          // For bytea, we need to escape it properly
          std::string result;
          result.reserve(arg.size() * 2 + 2);
          result = "\\x";
          for (uint8_t byte : arg) {
              char hex[3];
              snprintf(hex, sizeof(hex), "%02x", byte);
              result += hex;
          }
          return result;
      }
      return "";
  }, value);
}

pqxx::result PostgresTransaction::ExecuteWithParams(
  pqxx::work & txn,
  const std::string & query,
  const std::vector<DBValue> & params)
{
  if (params.empty()) {
    return txn.exec(query);
  }

  // For libpqxx 6.x, we need to use exec_params with individual arguments.
  // Since we can't pass a vector directly, we use a switch on the number of params.
  // This is the safest approach for libpqxx 6.x.

  std::vector<std::string> stringParams;
  stringParams.reserve(params.size());
  for (const auto & param : params) {
    stringParams.push_back(ConvertValueToPQXX(param));
  }

  switch (stringParams.size()) {
    case 0:
      return txn.exec(query);
    case 1:
      return txn.exec_params(query, stringParams[0]);
    case 2:
      return txn.exec_params(query, stringParams[0], stringParams[1]);
    case 3:
      return txn.exec_params(query, stringParams[0], stringParams[1], stringParams[2]);
    case 4:
      return txn.exec_params(query, stringParams[0], stringParams[1], stringParams[2], stringParams[3]);
    case 5:
      return txn.exec_params(query, stringParams[0], stringParams[1], stringParams[2], stringParams[3], stringParams[4]);
    case 6:
      return txn.exec_params(query, stringParams[0], stringParams[1], stringParams[2], stringParams[3], stringParams[4], stringParams[5]);
    case 7:
      return txn.exec_params(query, stringParams[0], stringParams[1], stringParams[2], stringParams[3], stringParams[4], stringParams[5], stringParams[6]);
    case 8:
      return txn.exec_params(query, stringParams[0], stringParams[1], stringParams[2], stringParams[3], stringParams[4], stringParams[5], stringParams[6], stringParams[7]);
    case 9:
      return txn.exec_params(query, stringParams[0], stringParams[1], stringParams[2], stringParams[3], stringParams[4], stringParams[5], stringParams[6], stringParams[7], stringParams[8]);
    case 10:
      return txn.exec_params(query, stringParams[0], stringParams[1], stringParams[2], stringParams[3], stringParams[4], stringParams[5], stringParams[6], stringParams[7], stringParams[8], stringParams[9]);
    default:
      // For more than 10 parameters, fall back to building the query manually
      // This is not ideal but should rarely be needed
      std::string fullQuery = query;
      for (size_t i = 0; i < stringParams.size(); ++i) {
        std::string placeholder = "$" + std::to_string(i + 1);
        size_t pos = fullQuery.find(placeholder);
        if (pos != std::string::npos) {
          fullQuery.replace(pos, placeholder.length(), "'" + stringParams[i] + "'");
        }
      }
      return txn.exec(fullQuery);
  }
}

std::unique_ptr<IResultSet> PostgresTransaction::Execute(const std::string & query)
{
  auto result = m_txn->exec(query);
  return std::make_unique<PostgresResultSet>(std::move(result));
}

std::unique_ptr<IResultSet> PostgresTransaction::ExecuteParams(
  const std::string & query,
  const std::vector<DBValue> & params)
{
  auto result = ExecuteWithParams(*m_txn, query, params);
  return std::make_unique<PostgresResultSet>(std::move(result));
}

void PostgresTransaction::Commit()
{
  if (m_txn && !m_committed) {
    m_txn->commit();
    m_committed = true;
  }
}

void PostgresTransaction::Rollback()
{
  if (m_txn && !m_committed) {
    m_txn->abort();
    m_committed = true;
  }
}

PostgresConnection::PostgresConnection(std::string connectionString)
{
  try {
    m_connection = std::make_unique<pqxx::connection>(std::move(connectionString));
    m_isOpen = m_connection->is_open();
    if (!m_isOpen) {
      throw std::runtime_error("Failed to connect to PostgreSQL");
    }
  } catch (const std::exception & e) {
    throw std::runtime_error("PostgreSQL connection error: " + std::string(e.what()));
  }
}

PostgresConnection::~PostgresConnection() {
  Close();
}

bool PostgresConnection::IsOpen() const {
  return m_isOpen && m_connection && m_connection->is_open();
}

std::unique_ptr<ITransaction> PostgresConnection::BeginTransaction()
{
  if (!IsOpen()) {
    throw std::runtime_error("Connection is not open");
  }
  return std::make_unique<PostgresTransaction>(*m_connection);
}

std::unique_ptr<IResultSet> PostgresConnection::Execute(const std::string & query)
{
  if (!IsOpen()) {
    throw std::runtime_error("Connection is not open");
  }

  pqxx::work txn(*m_connection);
  auto result = txn.exec(query);
  txn.commit();
  return std::make_unique<PostgresResultSet>(std::move(result));
}

std::unique_ptr<IResultSet> PostgresConnection::ExecuteParams(
  const std::string & query,
  const std::vector<DBValue> & params)
{
  if (!IsOpen()) {
    throw std::runtime_error("Connection is not open");
  }

  pqxx::work txn(*m_connection);
  auto result = PostgresTransaction::ExecuteWithParams(txn, query, params);
  txn.commit();
  return std::make_unique<PostgresResultSet>(std::move(result));
}

void PostgresConnection::Close() {
  if (m_connection) {
    try {
      m_connection->close();
    } catch (...) {}
    m_isOpen = false;
  }
}

} // namespace msngr::profile::repository