#pragma once
#include <sqlite3.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <string>

#include "data/Repositories.h"

namespace lms::data {

/// RAII owner of one SQLite connection.
/// Every connection: foreign keys ON, busy timeout (waits instead of failing when another
/// process holds the write lock), WAL journal for file databases (readers never block the writer).
class Database {
public:
    explicit Database(const std::string& path);  ///< use ":memory:" for tests
    ~Database();
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    void initialiseSchema();                 ///< runs the embedded schema script (idempotent)
    void exec(const std::string& sql);       ///< for fixed, parameter-free statements only
    sqlite3* handle() const { return db_; }
    std::int64_t lastInsertId() const { return sqlite3_last_insert_rowid(db_); }

    /// Translate a SQLite result code into LmsError subclasses.
    [[noreturn]] void throwError(int rc, const std::string& context) const;

private:
    sqlite3* db_ = nullptr;
};

/// Prepared statement with positional (?1, ?2, ...) parameter binding. All user-supplied
/// values reach SQL only through bind(), never by string concatenation.
class Statement {
public:
    template <typename... Args>
    Statement(Database& db, const char* sql, const Args&... args) : db_(db) {
        prepare(sql);
        int i = 1;
        (bind(i++, args), ...);
    }
    ~Statement();
    Statement(const Statement&) = delete;
    Statement& operator=(const Statement&) = delete;

    bool step();  ///< true when a row is available, false when finished
    void run();   ///< execute a statement that returns no rows

    int getInt(int col) const { return sqlite3_column_int(stmt_, col); }
    std::int64_t getInt64(int col) const { return sqlite3_column_int64(stmt_, col); }
    double getDouble(int col) const { return sqlite3_column_double(stmt_, col); }
    std::string getText(int col) const;
    bool isNull(int col) const { return sqlite3_column_type(stmt_, col) == SQLITE_NULL; }

private:
    void prepare(const char* sql);
    void bind(int i, int v) { check(sqlite3_bind_int(stmt_, i, v)); }
    void bind(int i, std::int64_t v) { check(sqlite3_bind_int64(stmt_, i, v)); }
    void bind(int i, double v) { check(sqlite3_bind_double(stmt_, i, v)); }
    void bind(int i, const std::string& v) {
        check(sqlite3_bind_text(stmt_, i, v.c_str(), static_cast<int>(v.size()), SQLITE_TRANSIENT));
    }
    void bind(int i, const char* v) { v ? bind(i, std::string(v)) : bind(i, nullptr); }
    void bind(int i, std::nullptr_t) { check(sqlite3_bind_null(stmt_, i)); }
    void bind(int i, const std::optional<std::string>& v) { v ? bind(i, *v) : bind(i, nullptr); }
    void bind(int i, const std::optional<double>& v) { v ? bind(i, *v) : bind(i, nullptr); }
    void bind(int i, DateTime v) { bind(i, v.toString()); }
    void check(int rc) { if (rc != SQLITE_OK) db_.throwError(rc, "bind"); }

    Database& db_;
    sqlite3_stmt* stmt_ = nullptr;
};

/// BEGIN IMMEDIATE takes the write lock up front: all writers are serialised
/// (the "single writer" mitigation for risk R2).
class SqliteTransactionManager final : public ITransactionManager {
public:
    explicit SqliteTransactionManager(Database& db) : db_(db) {}
    std::unique_ptr<ITransactionScope> begin() override;

private:
    Database& db_;
};

}  // namespace lms::data
