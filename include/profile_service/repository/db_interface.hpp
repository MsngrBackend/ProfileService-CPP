#ifndef MSNGR__PROFILE__REPOSITORY__DB_INTERFACE_HPP_
#define MSNGR__PROFILE__REPOSITORY__DB_INTERFACE_HPP_

#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <variant>
#include <any>

namespace msngr::profile::repository {

// Type-erased value that can hold various DB types
using DBValue = std::variant<
    std::nullptr_t,
    bool,
    int64_t,
    double,
    std::string,
    std::vector<uint8_t>
>;

// Row interface - abstract access to a result row
class IRow {
public:
    virtual ~IRow() = default;
    virtual bool IsNull(size_t index) const = 0;
    virtual bool IsNull(const std::string& column) const = 0;
    virtual DBValue GetValue(size_t index) const = 0;
    virtual DBValue GetValue(const std::string& column) const = 0;
    virtual size_t Size() const = 0;
};

// Result set interface
class IResultSet {
public:
    virtual ~IResultSet() = default;
    virtual bool Empty() const = 0;
    virtual size_t Size() const = 0;
    virtual std::unique_ptr<IRow> GetRow(size_t index) const = 0;
    virtual std::unique_ptr<IRow> GetRow() = 0; // For iteration
    virtual bool Next() = 0;
};

// Transaction interface
class ITransaction {
public:
    virtual ~ITransaction() = default;
    virtual std::unique_ptr<IResultSet> Execute(const std::string& query) = 0;
    virtual std::unique_ptr<IResultSet> ExecuteParams(
        const std::string& query, 
        const std::vector<DBValue>& params
    ) = 0;
    virtual void Commit() = 0;
    virtual void Rollback() = 0;
};

// Connection interface
class IDatabaseConnection {
public:
    virtual ~IDatabaseConnection() = default;
    virtual bool IsOpen() const = 0;
    virtual std::unique_ptr<ITransaction> BeginTransaction() = 0;
    virtual std::unique_ptr<IResultSet> Execute(const std::string& query) = 0;
    virtual std::unique_ptr<IResultSet> ExecuteParams(
        const std::string& query, 
        const std::vector<DBValue>& params
    ) = 0;
    virtual void Close() = 0;
};

// Connection factory
class IDBConnectionFactory {
public:
    virtual ~IDBConnectionFactory() = default;
    virtual std::unique_ptr<IDatabaseConnection> CreateConnection() = 0;
};

} // namespace msngr::profile::repository

#endif // MSNGR__PROFILE__REPOSITORY__DB_INTERFACE_HPP_