#ifndef MSNGR__PROFILE__REPOSITORY__QUERY_EXECUTOR_HPP_
#define MSNGR__PROFILE__REPOSITORY__QUERY_EXECUTOR_HPP_

#include "db_interface.hpp"
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <optional>

namespace msngr::profile::repository {

class QueryExecutor {
public:
    explicit QueryExecutor(std::unique_ptr<IDatabaseConnection> connection);
    
    std::unique_ptr<IResultSet> ExecuteSelect(
        const std::string& query,
        const std::vector<DBValue>& params = {}
    );
    
    void ExecuteModify(
        const std::string& query,
        const std::vector<DBValue>& params = {}
    );
    
    // Execute in a transaction
    template<typename Func>
    void ExecuteInTransaction(Func operation) {
        if (!m_connection || !m_connection->IsOpen()) {
            throw std::runtime_error("Database connection is not open");
        }
        auto txn = m_connection->BeginTransaction();
        operation(*txn);
        txn->Commit();
    }
    
    // Helper methods to extract values from rows
    static std::optional<std::string> GetStringValue(const IRow* row, const std::string& column);
    static std::optional<int64_t> GetIntValue(const IRow* row, const std::string& column);
    static std::optional<bool> GetBoolValue(const IRow* row, const std::string& column);
    static std::optional<double> GetDoubleValue(const IRow* row, const std::string& column);
    static std::vector<uint8_t> GetBlobValue(const IRow* row, const std::string& column);
    static bool HasColumn(const IRow* row, const std::string& column);

private:
    std::unique_ptr<IDatabaseConnection> m_connection;
};

} // namespace msngr::profile::repository

#endif // MSNGR__PROFILE__REPOSITORY__QUERY_EXECUTOR_HPP_