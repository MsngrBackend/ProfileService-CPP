#include "postgres_connection.hpp"
#include <stdexcept>
#include <sstream>
#include <pqxx/util>

namespace msngr::profile::repository {

// PostgresRow implementation
PostgresRow::PostgresRow(const pqxx::row& row) : m_row(row) {}

bool PostgresRow::IsNull(size_t index) const {
    return m_row[index].is_null();
}

bool PostgresRow::IsNull(const std::string& column) const {
    return m_row[column].is_null();
}

DBValue PostgresRow::GetValue(size_t index) const {
    if (m_row[index].is_null()) {
        return nullptr;
    }
    
    const auto& field = m_row[index];
    
    try {
        // Get the type name from oid
        std::string typeName = field.type().name();
        
        if (typeName == "bool") return field.as<bool>();
        if (typeName == "int2" || typeName == "int4" || typeName == "int8") 
            return field.as<int64_t>();
        if (typeName == "float4" || typeName == "float8") 
            return field.as<double>();
        if (typeName == "text" || typeName == "varchar" || typeName == "char" || 
            typeName == "bpchar" || typeName == "name") 
            return field.as<std::string>();
        if (typeName == "bytea") {
            // For bytea, we need to handle it differently
            auto blob = field.as<std::vector<uint8_t>>();
            return blob;
        }
        // Default to string
        return field.as<std::string>();
    } catch (const std::exception&) {
        return field.as<std::string>();
    }
}

DBValue PostgresRow::GetValue(const std::string& column) const
{
  if (m_row[column].is_null()) {
      return nullptr;
  }
  
  const auto& field = m_row[column];
  
  try {
      std::string typeName = field.type().name();
      
      if (typeName == "bool") return field.as<bool>();
      if (typeName == "int2" || typeName == "int4" || typeName == "int8") 
          return field.as<int64_t>();
      if (typeName == "float4" || typeName == "float8") 
          return field.as<double>();
      if (typeName == "text" || typeName == "varchar" || typeName == "char" ||
          typeName == "bpchar" || typeName == "name") 
          return field.as<std::string>();
      if (typeName == "bytea") {
          auto blob = field.as<std::vector<uint8_t>>();
          return blob;
      }
      return field.as<std::string>();
  } catch (const std::exception &) {
      return field.as<std::string>();
  }
}

size_t PostgresRow::Size() const {
    return m_row.size();
}

// PostgresResultSet implementation
PostgresResultSet::PostgresResultSet(pqxx::result result) : m_result(std::move(result)) {}

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

// PostgresTransaction implementation
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
            // For bytea, we need to encode properly
            // This is a simplified version - in production, use proper bytea encoding
            std::string result;
            result.reserve(arg.size() * 2);
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
    pqxx::work& txn,
    const std::string& query,
    const std::vector<DBValue>& params
) {
    if (params.empty()) {
        return txn.exec(query);
    }
    
    // Build parameterized query
    std::vector<std::string> stringParams;
    stringParams.reserve(params.size());
    for (const auto& param : params) {
        stringParams.push_back(ConvertValueToPQXX(param));
    }
    
    // Use exec_params with string parameters
    return txn.exec_params(query, stringParams.begin(), stringParams.end());
}

std::unique_ptr<IResultSet> PostgresTransaction::Execute(const std::string& query) {
    auto result = m_txn->exec(query);
    return std::make_unique<PostgresResultSet>(std::move(result));
}

std::unique_ptr<IResultSet> PostgresTransaction::ExecuteParams(
    const std::string& query,
    const std::vector<DBValue>& params
) {
    auto result = ExecuteWithParams(*m_txn, query, params);
    return std::make_unique<PostgresResultSet>(std::move(result));
}

void PostgresTransaction::Commit() {
    if (m_txn && !m_committed) {
        m_txn->commit();
        m_committed = true;
    }
}

void PostgresTransaction::Rollback() {
    if (m_txn && !m_committed) {
        m_txn->abort();
        m_committed = true;
    }
}

// PostgresConnection implementation
PostgresConnection::PostgresConnection(const std::string& connectionString) {
    try {
        m_connection = std::make_unique<pqxx::connection>(connectionString);
        m_isOpen = m_connection->is_open();
        if (!m_isOpen) {
            throw std::runtime_error("Failed to connect to PostgreSQL");
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("PostgreSQL connection error: " + std::string(e.what()));
    }
}

PostgresConnection::~PostgresConnection() {
    Close();
}

bool PostgresConnection::IsOpen() const {
    return m_isOpen && m_connection && m_connection->is_open();
}

std::unique_ptr<ITransaction> PostgresConnection::BeginTransaction() {
    if (!IsOpen()) {
        throw std::runtime_error("Connection is not open");
    }
    return std::make_unique<PostgresTransaction>(*m_connection);
}

std::unique_ptr<IResultSet> PostgresConnection::Execute(const std::string& query) {
    if (!IsOpen()) {
        throw std::runtime_error("Connection is not open");
    }
    
    pqxx::work txn(*m_connection);
    auto result = txn.exec(query);
    txn.commit();
    return std::make_unique<PostgresResultSet>(std::move(result));
}

std::unique_ptr<IResultSet> PostgresConnection::ExecuteParams(
    const std::string& query,
    const std::vector<DBValue>& params
) {
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