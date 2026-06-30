#include "query_executor.hpp"
#include <stdexcept>

namespace msngr::profile::repository {

QueryExecutor::QueryExecutor(std::unique_ptr<IDatabaseConnection> connection)
    : m_connection(std::move(connection)) {}

std::unique_ptr<IResultSet> QueryExecutor::ExecuteSelect(
    const std::string& query,
    const std::vector<DBValue>& params
) {
    if (!m_connection || !m_connection->IsOpen()) {
        throw std::runtime_error("Database connection is not open");
    }
    
    if (params.empty()) {
        return m_connection->Execute(query);
    }
    return m_connection->ExecuteParams(query, params);
}

void QueryExecutor::ExecuteModify(
    const std::string& query,
    const std::vector<DBValue>& params
) {
    if (!m_connection || !m_connection->IsOpen()) {
        throw std::runtime_error("Database connection is not open");
    }
    
    if (params.empty()) {
        m_connection->Execute(query);
    } else {
        m_connection->ExecuteParams(query, params);
    }
}

std::optional<std::string> QueryExecutor::GetStringValue(const IRow* row, const std::string& column) {
    if (!row) return std::nullopt;
    if (row->IsNull(column)) return std::nullopt;
    
    auto value = row->GetValue(column);
    if (std::holds_alternative<std::string>(value)) {
        return std::get<std::string>(value);
    }
    return std::nullopt;
}

std::optional<int64_t> QueryExecutor::GetIntValue(const IRow* row, const std::string& column) {
    if (!row) return std::nullopt;
    if (row->IsNull(column)) return std::nullopt;
    
    auto value = row->GetValue(column);
    if (std::holds_alternative<int64_t>(value)) {
        return std::get<int64_t>(value);
    }
    return std::nullopt;
}

std::optional<bool> QueryExecutor::GetBoolValue(const IRow* row, const std::string& column) {
    if (!row) return std::nullopt;
    if (row->IsNull(column)) return std::nullopt;
    
    auto value = row->GetValue(column);
    if (std::holds_alternative<bool>(value)) {
        return std::get<bool>(value);
    }
    return std::nullopt;
}

std::optional<double> QueryExecutor::GetDoubleValue(const IRow* row, const std::string& column) {
    if (!row) return std::nullopt;
    if (row->IsNull(column)) return std::nullopt;
    
    auto value = row->GetValue(column);
    if (std::holds_alternative<double>(value)) {
        return std::get<double>(value);
    }
    return std::nullopt;
}

std::vector<uint8_t> QueryExecutor::GetBlobValue(const IRow* row, const std::string& column) {
    if (!row || row->IsNull(column)) {
        return {};
    }
    
    auto value = row->GetValue(column);
    if (std::holds_alternative<std::vector<uint8_t>>(value)) {
        return std::get<std::vector<uint8_t>>(value);
    }
    return {};
}

bool QueryExecutor::HasColumn(const IRow* row, const std::string& column) {
    if (!row) return false;
    // Try to access the column - if it throws, it doesn't exist
    try {
        row->GetValue(column);
        return true;
    } catch (...) {
        return false;
    }
}

} // namespace msngr::profile::repository