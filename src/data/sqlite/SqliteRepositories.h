#pragma once
#include "data/Repositories.h"
#include "data/sqlite/Database.h"

namespace lms::data {

class SqliteUserRepository final : public IUserRepository {
public:
    explicit SqliteUserRepository(Database& db) : db_(db) {}
    std::unique_ptr<User> findById(int id) override;
    std::unique_ptr<User> findByUsername(const std::string& username) override;
    int add(const User& user) override;
    void updatePasswordHash(int id, const std::string& hash) override;
    void updateUsername(int id, const std::string& username) override;
    void remove(int id) override;
    std::vector<UserRecord> listAll() override;
    int count() override;
    int countByRole(UserRole role) override;

private:
    Database& db_;
};

class SqliteDepartmentRepository final : public IDepartmentRepository {
public:
    explicit SqliteDepartmentRepository(Database& db) : db_(db) {}
    int add(const std::string& name) override;
    std::optional<Department> findById(int id) override;
    std::vector<Department> list() override;
    void rename(int id, const std::string& name) override;
    void remove(int id) override;

private:
    Database& db_;
};

class SqliteCourseRepository final : public ICourseRepository {
public:
    explicit SqliteCourseRepository(Database& db) : db_(db) {}
    void add(const Course& course) override;
    std::optional<Course> find(const std::string& code) override;
    std::vector<Course> list() override;
    std::vector<Course> listByDepartment(int deptId) override;
    void updateTitle(const std::string& code, const std::string& title) override;
    void remove(const std::string& code) override;

private:
    Database& db_;
};

class SqliteTeachingRepository final : public ITeachingRepository {
public:
    explicit SqliteTeachingRepository(Database& db) : db_(db) {}
    void assign(int lecturerId, const std::string& courseCode) override;
    void unassign(int lecturerId, const std::string& courseCode) override;
    bool isAssigned(int lecturerId, const std::string& courseCode) override;
    std::vector<Course> coursesOfLecturer(int lecturerId) override;

private:
    Database& db_;
};

class SqliteEnrollmentRepository final : public IEnrollmentRepository {
public:
    explicit SqliteEnrollmentRepository(Database& db) : db_(db) {}
    void enrol(int studentId, const std::string& courseCode) override;
    void unenrol(int studentId, const std::string& courseCode) override;
    bool isEnrolled(int studentId, const std::string& courseCode) override;
    std::vector<Course> coursesOfStudent(int studentId) override;
    std::vector<StudentInfo> studentsOfCourse(const std::string& courseCode) override;

private:
    Database& db_;
};

class SqliteAssignmentRepository final : public IAssignmentRepository {
public:
    explicit SqliteAssignmentRepository(Database& db) : db_(db) {}
    int add(const Assignment& a) override;
    std::optional<Assignment> find(int id) override;
    std::vector<Assignment> listByCourse(const std::string& courseCode) override;

private:
    Database& db_;
};

class SqliteExtensionRepository final : public IExtensionRepository {
public:
    explicit SqliteExtensionRepository(Database& db) : db_(db) {}
    int add(const Extension& e) override;
    std::optional<Extension> find(int studentId, int assignmentId) override;
    std::vector<Extension> listForStudent(int studentId) override;

private:
    Database& db_;
};

class SqliteSubmissionRepository final : public ISubmissionRepository {
public:
    explicit SqliteSubmissionRepository(Database& db) : db_(db) {}
    int add(const Submission& s) override;
    void update(const Submission& s) override;
    std::optional<Submission> findById(int id) override;
    std::optional<Submission> find(int studentId, int assignmentId) override;
    std::vector<Submission> listByStudent(int studentId) override;
    std::vector<SubmissionRow> listRowsByAssignment(int assignmentId) override;

private:
    Database& db_;
};

class SqliteMarkRepository final : public IMarkRepository {
public:
    explicit SqliteMarkRepository(Database& db) : db_(db) {}
    int add(const Mark& m) override;
    void update(const Mark& m) override;
    std::optional<Mark> findById(int id) override;
    std::optional<Mark> findBySubmissionId(int submissionId) override;
    std::optional<MarkRow> findRow(int markId) override;
    std::vector<MarkRow> listForDepartment(int deptId, std::optional<MarkState> stateFilter) override;
    std::vector<MarkRow> listForStudent(int studentId, bool onlyReleased) override;

private:
    Database& db_;
};

class SqliteAuditRepository final : public IAuditRepository {
public:
    explicit SqliteAuditRepository(Database& db) : db_(db) {}
    int append(const AuditEntry& e) override;
    std::vector<AuditEntry> listForEntity(const std::string& entityType, int entityId) override;
    std::vector<AuditEntry> listRecent(int limit) override;

private:
    Database& db_;
};

}  // namespace lms::data
