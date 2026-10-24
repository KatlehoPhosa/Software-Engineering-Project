#include "data/sqlite/Database.h"

#include "common/Exceptions.h"
#include "data/sqlite/SchemaSql.h"

namespace lms::data {

Database::Database(const std::string& path) {
    int rc = sqlite3_open_v2(path.c_str(), &db_, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
    if (rc != SQLITE_OK) {
        std::string msg = db_ ? sqlite3_errmsg(db_) : "out of memory";
        if (db_) sqlite3_close(db_);
        db_ = nullptr;
        throw DatabaseError("Cannot open database '" + path + "': " + msg);
    }
    sqlite3_extended_result_codes(db_, 1);
    sqlite3_busy_timeout(db_, 5000);
    exec("PRAGMA foreign_keys = ON;");
    if (path != ":memory:") exec("PRAGMA journal_mode = WAL;");
}

Database::~Database() {
    if (db_) sqlite3_close(db_);
}

void Database::initialiseSchema() { exec(schemaSql()); }

void Database::exec(const std::string& sql) {
    char* err = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err);
    if (rc != SQLITE_OK) {
        std::string msg = err ? err : "unknown error";
        sqlite3_free(err);
        throw DatabaseError("Database error: " + msg);
    }
}

void Database::throwError(int rc, const std::string& context) const {
    const std::string detail = std::string(sqlite3_errmsg(db_));
    if ((rc & 0xFF) == SQLITE_CONSTRAINT) throw ConstraintViolation("Constraint violated (" + detail + ")");
    if ((rc & 0xFF) == SQLITE_BUSY || (rc & 0xFF) == SQLITE_LOCKED)
        throw DatabaseError("The database is busy. Please try again in a moment.");
    throw DatabaseError("Database error during " + context + ": " + detail);
}

void Statement::prepare(const char* sql) {
    int rc = sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt_, nullptr);
    if (rc != SQLITE_OK) db_.throwError(rc, "prepare");
}

Statement::~Statement() {
    if (stmt_) sqlite3_finalize(stmt_);
}

bool Statement::step() {
    int rc = sqlite3_step(stmt_);
    if (rc == SQLITE_ROW) return true;
    if (rc == SQLITE_DONE) return false;
    db_.throwError(rc, "step");
}

void Statement::run() {
    while (step()) {
    }
}

std::string Statement::getText(int col) const {
    const unsigned char* t = sqlite3_column_text(stmt_, col);
    return t ? std::string(reinterpret_cast<const char*>(t)) : std::string();
}

namespace {
class SqliteTransactionScope final : public ITransactionScope {
public:
    explicit SqliteTransactionScope(Database& db) : db_(db) { db_.exec("BEGIN IMMEDIATE;"); }
    ~SqliteTransactionScope() override {
        if (!done_) {
            sqlite3_exec(db_.handle(), "ROLLBACK;", nullptr, nullptr, nullptr);  // never throws from a destructor
        }
    }
    void commit() override {
        db_.exec("COMMIT;");
        done_ = true;
    }

private:
    Database& db_;
    bool done_ = false;
};
}  // namespace

std::unique_ptr<ITransactionScope> SqliteTransactionManager::begin() {
    if (!sqlite3_get_autocommit(db_.handle())) throw DatabaseError("A transaction is already open.");
    return std::make_unique<SqliteTransactionScope>(db_);
}

}  // namespace lms::data
