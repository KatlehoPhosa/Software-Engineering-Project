#pragma once
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "domain/Entities.h"
#include "domain/User.h"

namespace lms {

// ---- Transactions ----------------------------------------------------------
/// RAII scope: rolls back on destruction unless commit() was called.
class ITransactionScope {
public:
    virtual ~ITransactionScope() = default;
    virtual void commit() = 0;
};

class ITransactionManager {
public:
    virtual ~ITransactionManager() = default;
    virtual std::unique_ptr<ITransactionScope> begin() = 0;
};

// ---- Read models (flattened joins) ------------------------------------------
/// One mark with the context a HoD, lecturer or student needs to see it.
struct MarkRow {
    int markId = 0;
    int submissionId = 0;
    int assignmentId = 0;
    int studentId = 0;
    std::string studentNo;
    std::string studentUsername;
    std::string courseCode;
    int departmentId = 0;
    std::string assignmentTitle;
    double maxMark = 0;
    double rawMark = 0;
    std::optional<double> finalMark;
    std::optional<std::string> feedback;
    MarkState markState = MarkState::Provisional;
    DateTime submittedAt;
    DateTime deadline;   ///< original assignment deadline
    int extraDays = 0;   ///< student's extension (0 if none)

    DateTime effectiveDeadline() const { return deadline.addDays(extraDays); }
    int daysLate() const;
};

/// A submission waiting to be marked (or already marked) with student details.
struct SubmissionRow {
    int submissionId = 0;
    int assignmentId = 0;
    int studentId = 0;
    std::string studentNo;
    std::string studentUsername;
    DateTime submittedAt;
    std::string fileRef;
    SubmissionState state = SubmissionState::Submitted;
    std::optional<MarkState> markState;
    DateTime deadline;
    int extraDays = 0;

    DateTime effectiveDeadline() const { return deadline.addDays(extraDays); }
    int daysLate() const;
};

struct StudentInfo {
    int id = 0;
    std::string studentNo;
    std::string username;
};

// ---- Repository interfaces (one per aggregate - Interface Segregation) ------
class IUserRepository {
public:
    virtual ~IUserRepository() = default;
    virtual std::unique_ptr<User> findById(int id) = 0;
    virtual std::unique_ptr<User> findByUsername(const std::string& username) = 0;
    virtual int add(const User& user) = 0;  ///< returns the new id
    virtual void updatePasswordHash(int id, const std::string& hash) = 0;
    virtual void updateUsername(int id, const std::string& username) = 0;
    virtual void remove(int id) = 0;
    virtual std::vector<UserRecord> listAll() = 0;
    virtual int count() = 0;
    virtual int countByRole(UserRole role) = 0;
};

class IDepartmentRepository {
public:
    virtual ~IDepartmentRepository() = default;
    virtual int add(const std::string& name) = 0;
    virtual std::optional<Department> findById(int id) = 0;
    virtual std::vector<Department> list() = 0;
    virtual void rename(int id, const std::string& name) = 0;
    virtual void remove(int id) = 0;
};

class ICourseRepository {
public:
    virtual ~ICourseRepository() = default;
    virtual void add(const Course& course) = 0;
    virtual std::optional<Course> find(const std::string& code) = 0;
    virtual std::vector<Course> list() = 0;
    virtual std::vector<Course> listByDepartment(int deptId) = 0;
    virtual void updateTitle(const std::string& code, const std::string& title) = 0;
    virtual void remove(const std::string& code) = 0;
};

/// Lecturer <-> Course ("teaches").
class ITeachingRepository {
public:
    virtual ~ITeachingRepository() = default;
    virtual void assign(int lecturerId, const std::string& courseCode) = 0;
    virtual void unassign(int lecturerId, const std::string& courseCode) = 0;
    virtual bool isAssigned(int lecturerId, const std::string& courseCode) = 0;
    virtual std::vector<Course> coursesOfLecturer(int lecturerId) = 0;
};

/// Student <-> Course ("enrolled in").
class IEnrollmentRepository {
public:
    virtual ~IEnrollmentRepository() = default;
    virtual void enrol(int studentId, const std::string& courseCode) = 0;
    virtual void unenrol(int studentId, const std::string& courseCode) = 0;
    virtual bool isEnrolled(int studentId, const std::string& courseCode) = 0;
    virtual std::vector<Course> coursesOfStudent(int studentId) = 0;
    virtual std::vector<StudentInfo> studentsOfCourse(const std::string& courseCode) = 0;
};

class IAssignmentRepository {
public:
    virtual ~IAssignmentRepository() = default;
    virtual int add(const Assignment& a) = 0;
    virtual std::optional<Assignment> find(int id) = 0;
    virtual std::vector<Assignment> listByCourse(const std::string& courseCode) = 0;
};

class IExtensionRepository {
public:
    virtual ~IExtensionRepository() = default;
    virtual int add(const Extension& e) = 0;
    virtual std::optional<Extension> find(int studentId, int assignmentId) = 0;
    virtual std::vector<Extension> listForStudent(int studentId) = 0;
};

class ISubmissionRepository {
public:
    virtual ~ISubmissionRepository() = default;
    virtual int add(const Submission& s) = 0;
    virtual void update(const Submission& s) = 0;
    virtual std::optional<Submission> findById(int id) = 0;
    virtual std::optional<Submission> find(int studentId, int assignmentId) = 0;
    virtual std::vector<Submission> listByStudent(int studentId) = 0;
    /// Submissions to an assignment with student details, ordered by student number.
    virtual std::vector<SubmissionRow> listRowsByAssignment(int assignmentId) = 0;
};

class IMarkRepository {
public:
    virtual ~IMarkRepository() = default;
    virtual int add(const Mark& m) = 0;
    virtual void update(const Mark& m) = 0;
    virtual std::optional<Mark> findById(int id) = 0;
    virtual std::optional<Mark> findBySubmissionId(int submissionId) = 0;
    virtual std::optional<MarkRow> findRow(int markId) = 0;
    virtual std::vector<MarkRow> listForDepartment(int deptId, std::optional<MarkState> stateFilter) = 0;
    /// onlyReleased = true is what students use: unreleased marks are never returned.
    virtual std::vector<MarkRow> listForStudent(int studentId, bool onlyReleased) = 0;
};

/// Append-only: there is deliberately no update or delete (AUDIT_ENTRY rules).
class IAuditRepository {
public:
    virtual ~IAuditRepository() = default;
    virtual int append(const AuditEntry& e) = 0;
    virtual std::vector<AuditEntry> listForEntity(const std::string& entityType, int entityId) = 0;
    virtual std::vector<AuditEntry> listRecent(int limit) = 0;
};

inline int MarkRow::daysLate() const { return lateDays(submittedAt, effectiveDeadline()); }
inline int SubmissionRow::daysLate() const { return lateDays(submittedAt, effectiveDeadline()); }

}  // namespace lms
