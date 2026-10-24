#include "data/sqlite/SqliteRepositories.h"

#include "common/Exceptions.h"
#include "domain/UserFactory.h"

namespace lms::data {
namespace {

std::optional<std::string> optText(const Statement& st, int col) {
    return st.isNull(col) ? std::nullopt : std::optional<std::string>(st.getText(col));
}
std::optional<double> optDouble(const Statement& st, int col) {
    return st.isNull(col) ? std::nullopt : std::optional<double>(st.getDouble(col));
}

// ---------------------------------------------------------------- users
const char* kUserSelect =
    "SELECT u.id, u.username, u.password_hash, u.role, s.student_no, l.staff_no, h.department_id "
    "FROM \"user\" u "
    "LEFT JOIN student s ON s.user_id = u.id "
    "LEFT JOIN lecturer l ON l.user_id = u.id "
    "LEFT JOIN hod h ON h.user_id = u.id ";

UserRecord readUser(const Statement& st) {
    UserRecord r;
    r.id = st.getInt(0);
    r.username = st.getText(1);
    r.passwordHash = st.getText(2);
    r.role = userRoleFromDb(st.getText(3));
    r.studentNo = st.getText(4);
    r.staffNo = st.getText(5);
    r.departmentId = st.isNull(6) ? 0 : st.getInt(6);
    return r;
}

Course readCourse(const Statement& st) { return Course(st.getText(0), st.getText(1), st.getInt(2)); }

Assignment readAssignment(const Statement& st) {
    return Assignment(st.getInt(0), st.getText(1), DateTime::parse(st.getText(2)), st.getDouble(3),
                      st.getText(4), st.getInt(5));
}

Submission readSubmission(const Statement& st) {
    return Submission(st.getInt(0), st.getInt(1), st.getInt(2), DateTime::parse(st.getText(3)),
                      st.getText(4), submissionStateFromDb(st.getText(5)));
}

Mark readMark(const Statement& st) {
    return Mark(st.getInt(0), st.getInt(1), st.getDouble(2), optDouble(st, 3), optText(st, 4),
                markStateFromDb(st.getText(5)));
}

const char* kMarkRowSelect =
    "SELECT m.id, m.submission_id, a.id, st.user_id, st.student_no, u.username, a.course_code, c.dept_id, "
    "       a.title, a.max_mark, m.raw_mark, m.final_mark, m.feedback, m.state, s.submitted_at, a.deadline, "
    "       COALESCE(e.extra_days, 0) "
    "FROM mark m "
    "JOIN submission s ON s.id = m.submission_id "
    "JOIN assignment a ON a.id = s.assignment_id "
    "JOIN course c ON c.code = a.course_code "
    "JOIN student st ON st.user_id = s.student_id "
    "JOIN \"user\" u ON u.id = st.user_id "
    "LEFT JOIN extension e ON e.student_id = s.student_id AND e.assignment_id = s.assignment_id ";

MarkRow readMarkRow(const Statement& st) {
    MarkRow r;
    r.markId = st.getInt(0);
    r.submissionId = st.getInt(1);
    r.assignmentId = st.getInt(2);
    r.studentId = st.getInt(3);
    r.studentNo = st.getText(4);
    r.studentUsername = st.getText(5);
    r.courseCode = st.getText(6);
    r.departmentId = st.getInt(7);
    r.assignmentTitle = st.getText(8);
    r.maxMark = st.getDouble(9);
    r.rawMark = st.getDouble(10);
    r.finalMark = optDouble(st, 11);
    r.feedback = optText(st, 12);
    r.markState = markStateFromDb(st.getText(13));
    r.submittedAt = DateTime::parse(st.getText(14));
    r.deadline = DateTime::parse(st.getText(15));
    r.extraDays = st.getInt(16);
    return r;
}

}  // namespace

// ================================================================= users
std::unique_ptr<User> SqliteUserRepository::findById(int id) {
    Statement st(db_, (std::string(kUserSelect) + "WHERE u.id = ?1").c_str(), id);
    if (!st.step()) return nullptr;
    return UserFactory::hydrate(readUser(st));
}

std::unique_ptr<User> SqliteUserRepository::findByUsername(const std::string& username) {
    Statement st(db_, (std::string(kUserSelect) + "WHERE u.username = ?1").c_str(), username);
    if (!st.step()) return nullptr;
    return UserFactory::hydrate(readUser(st));
}

int SqliteUserRepository::add(const User& user) {
    const UserRecord r = user.toRecord();
    Statement(db_, "INSERT INTO \"user\"(username, password_hash, role) VALUES (?1, ?2, ?3)", r.username,
              r.passwordHash, toDbString(r.role)).run();
    const int id = static_cast<int>(db_.lastInsertId());
    switch (r.role) {
        case UserRole::Student:
            Statement(db_, "INSERT INTO student(user_id, student_no) VALUES (?1, ?2)", id, r.studentNo).run();
            break;
        case UserRole::Lecturer:
            Statement(db_, "INSERT INTO lecturer(user_id, staff_no) VALUES (?1, ?2)", id, r.staffNo).run();
            break;
        case UserRole::HoD:
            Statement(db_, "INSERT INTO hod(user_id, department_id) VALUES (?1, ?2)", id, r.departmentId).run();
            break;
        case UserRole::Admin:
            Statement(db_, "INSERT INTO admin(user_id) VALUES (?1)", id).run();
            break;
    }
    return id;
}

void SqliteUserRepository::updatePasswordHash(int id, const std::string& hash) {
    Statement(db_, "UPDATE \"user\" SET password_hash = ?1 WHERE id = ?2", hash, id).run();
}

void SqliteUserRepository::updateUsername(int id, const std::string& username) {
    Statement(db_, "UPDATE \"user\" SET username = ?1 WHERE id = ?2", username, id).run();
}

void SqliteUserRepository::remove(int id) { Statement(db_, "DELETE FROM \"user\" WHERE id = ?1", id).run(); }

std::vector<UserRecord> SqliteUserRepository::listAll() {
    Statement st(db_, (std::string(kUserSelect) + "ORDER BY u.role, u.username").c_str());
    std::vector<UserRecord> out;
    while (st.step()) out.push_back(readUser(st));
    return out;
}

int SqliteUserRepository::count() {
    Statement st(db_, "SELECT COUNT(*) FROM \"user\"");
    st.step();
    return st.getInt(0);
}

int SqliteUserRepository::countByRole(UserRole role) {
    Statement st(db_, "SELECT COUNT(*) FROM \"user\" WHERE role = ?1", toDbString(role));
    st.step();
    return st.getInt(0);
}

// ================================================================= departments
int SqliteDepartmentRepository::add(const std::string& name) {
    Statement(db_, "INSERT INTO department(name) VALUES (?1)", name).run();
    return static_cast<int>(db_.lastInsertId());
}

std::optional<Department> SqliteDepartmentRepository::findById(int id) {
    Statement st(db_, "SELECT id, name FROM department WHERE id = ?1", id);
    if (!st.step()) return std::nullopt;
    return Department(st.getInt(0), st.getText(1));
}

std::vector<Department> SqliteDepartmentRepository::list() {
    Statement st(db_, "SELECT id, name FROM department ORDER BY name");
    std::vector<Department> out;
    while (st.step()) out.emplace_back(st.getInt(0), st.getText(1));
    return out;
}

void SqliteDepartmentRepository::rename(int id, const std::string& name) {
    Statement(db_, "UPDATE department SET name = ?1 WHERE id = ?2", name, id).run();
}

void SqliteDepartmentRepository::remove(int id) { Statement(db_, "DELETE FROM department WHERE id = ?1", id).run(); }

// ================================================================= courses
void SqliteCourseRepository::add(const Course& c) {
    Statement(db_, "INSERT INTO course(code, title, dept_id) VALUES (?1, ?2, ?3)", c.code(), c.title(), c.deptId()).run();
}

std::optional<Course> SqliteCourseRepository::find(const std::string& code) {
    Statement st(db_, "SELECT code, title, dept_id FROM course WHERE code = ?1", code);
    if (!st.step()) return std::nullopt;
    return readCourse(st);
}

std::vector<Course> SqliteCourseRepository::list() {
    Statement st(db_, "SELECT code, title, dept_id FROM course ORDER BY code");
    std::vector<Course> out;
    while (st.step()) out.push_back(readCourse(st));
    return out;
}

std::vector<Course> SqliteCourseRepository::listByDepartment(int deptId) {
    Statement st(db_, "SELECT code, title, dept_id FROM course WHERE dept_id = ?1 ORDER BY code", deptId);
    std::vector<Course> out;
    while (st.step()) out.push_back(readCourse(st));
    return out;
}

void SqliteCourseRepository::updateTitle(const std::string& code, const std::string& title) {
    Statement(db_, "UPDATE course SET title = ?1 WHERE code = ?2", title, code).run();
}

void SqliteCourseRepository::remove(const std::string& code) { Statement(db_, "DELETE FROM course WHERE code = ?1", code).run(); }

// ================================================================= teaching
void SqliteTeachingRepository::assign(int lecturerId, const std::string& courseCode) {
    Statement(db_, "INSERT INTO course_lecturer(lecturer_id, course_code) VALUES (?1, ?2)", lecturerId, courseCode).run();
}

void SqliteTeachingRepository::unassign(int lecturerId, const std::string& courseCode) {
    Statement(db_, "DELETE FROM course_lecturer WHERE lecturer_id = ?1 AND course_code = ?2", lecturerId, courseCode).run();
}

bool SqliteTeachingRepository::isAssigned(int lecturerId, const std::string& courseCode) {
    Statement st(db_, "SELECT 1 FROM course_lecturer WHERE lecturer_id = ?1 AND course_code = ?2", lecturerId, courseCode);
    return st.step();
}

std::vector<Course> SqliteTeachingRepository::coursesOfLecturer(int lecturerId) {
    Statement st(db_,
                 "SELECT c.code, c.title, c.dept_id FROM course c "
                 "JOIN course_lecturer cl ON cl.course_code = c.code WHERE cl.lecturer_id = ?1 ORDER BY c.code",
                 lecturerId);
    std::vector<Course> out;
    while (st.step()) out.push_back(readCourse(st));
    return out;
}

// ================================================================= enrolment
void SqliteEnrollmentRepository::enrol(int studentId, const std::string& courseCode) {
    Statement(db_, "INSERT INTO enrollment(student_id, course_code) VALUES (?1, ?2)", studentId, courseCode).run();
}

void SqliteEnrollmentRepository::unenrol(int studentId, const std::string& courseCode) {
    Statement(db_, "DELETE FROM enrollment WHERE student_id = ?1 AND course_code = ?2", studentId, courseCode).run();
}

bool SqliteEnrollmentRepository::isEnrolled(int studentId, const std::string& courseCode) {
    Statement st(db_, "SELECT 1 FROM enrollment WHERE student_id = ?1 AND course_code = ?2", studentId, courseCode);
    return st.step();
}

std::vector<Course> SqliteEnrollmentRepository::coursesOfStudent(int studentId) {
    Statement st(db_,
                 "SELECT c.code, c.title, c.dept_id FROM course c "
                 "JOIN enrollment e ON e.course_code = c.code WHERE e.student_id = ?1 ORDER BY c.code",
                 studentId);
    std::vector<Course> out;
    while (st.step()) out.push_back(readCourse(st));
    return out;
}

std::vector<StudentInfo> SqliteEnrollmentRepository::studentsOfCourse(const std::string& courseCode) {
    Statement st(db_,
                 "SELECT s.user_id, s.student_no, u.username FROM enrollment e "
                 "JOIN student s ON s.user_id = e.student_id JOIN \"user\" u ON u.id = s.user_id "
                 "WHERE e.course_code = ?1 ORDER BY s.student_no",
                 courseCode);
    std::vector<StudentInfo> out;
    while (st.step()) out.push_back(StudentInfo{st.getInt(0), st.getText(1), st.getText(2)});
    return out;
}

// ================================================================= assignments
int SqliteAssignmentRepository::add(const Assignment& a) {
    Statement(db_,
              "INSERT INTO assignment(title, deadline, max_mark, course_code, created_by) VALUES (?1, ?2, ?3, ?4, ?5)",
              a.title(), a.deadline(), a.maxMark(), a.courseCode(), a.createdBy()).run();
    return static_cast<int>(db_.lastInsertId());
}

std::optional<Assignment> SqliteAssignmentRepository::find(int id) {
    Statement st(db_, "SELECT id, title, deadline, max_mark, course_code, created_by FROM assignment WHERE id = ?1", id);
    if (!st.step()) return std::nullopt;
    return readAssignment(st);
}

std::vector<Assignment> SqliteAssignmentRepository::listByCourse(const std::string& courseCode) {
    Statement st(db_,
                 "SELECT id, title, deadline, max_mark, course_code, created_by FROM assignment "
                 "WHERE course_code = ?1 ORDER BY deadline, id",
                 courseCode);
    std::vector<Assignment> out;
    while (st.step()) out.push_back(readAssignment(st));
    return out;
}

// ================================================================= extensions
int SqliteExtensionRepository::add(const Extension& e) {
    Statement(db_,
              "INSERT INTO extension(student_id, assignment_id, extra_days, granted_by, reason) VALUES (?1, ?2, ?3, ?4, ?5)",
              e.studentId(), e.assignmentId(), e.extraDays(), e.grantedBy(), e.reason()).run();
    return static_cast<int>(db_.lastInsertId());
}

std::optional<Extension> SqliteExtensionRepository::find(int studentId, int assignmentId) {
    Statement st(db_,
                 "SELECT id, student_id, assignment_id, extra_days, granted_by, reason FROM extension "
                 "WHERE student_id = ?1 AND assignment_id = ?2",
                 studentId, assignmentId);
    if (!st.step()) return std::nullopt;
    return Extension(st.getInt(0), st.getInt(1), st.getInt(2), st.getInt(3), st.getInt(4), st.getText(5));
}

std::vector<Extension> SqliteExtensionRepository::listForStudent(int studentId) {
    Statement st(db_,
                 "SELECT id, student_id, assignment_id, extra_days, granted_by, reason FROM extension WHERE student_id = ?1",
                 studentId);
    std::vector<Extension> out;
    while (st.step())
        out.emplace_back(st.getInt(0), st.getInt(1), st.getInt(2), st.getInt(3), st.getInt(4), st.getText(5));
    return out;
}

// ================================================================= submissions
int SqliteSubmissionRepository::add(const Submission& s) {
    Statement(db_,
              "INSERT INTO submission(student_id, assignment_id, submitted_at, file_ref, state) VALUES (?1, ?2, ?3, ?4, ?5)",
              s.studentId(), s.assignmentId(), s.submittedAt(), s.fileRef(), toDbString(s.state())).run();
    return static_cast<int>(db_.lastInsertId());
}

void SqliteSubmissionRepository::update(const Submission& s) {
    Statement(db_, "UPDATE submission SET submitted_at = ?1, file_ref = ?2, state = ?3 WHERE id = ?4",
              s.submittedAt(), s.fileRef(), toDbString(s.state()), s.id()).run();
}

std::optional<Submission> SqliteSubmissionRepository::findById(int id) {
    Statement st(db_, "SELECT id, student_id, assignment_id, submitted_at, file_ref, state FROM submission WHERE id = ?1", id);
    if (!st.step()) return std::nullopt;
    return readSubmission(st);
}

std::optional<Submission> SqliteSubmissionRepository::find(int studentId, int assignmentId) {
    Statement st(db_,
                 "SELECT id, student_id, assignment_id, submitted_at, file_ref, state FROM submission "
                 "WHERE student_id = ?1 AND assignment_id = ?2",
                 studentId, assignmentId);
    if (!st.step()) return std::nullopt;
    return readSubmission(st);
}

std::vector<Submission> SqliteSubmissionRepository::listByStudent(int studentId) {
    Statement st(db_,
                 "SELECT id, student_id, assignment_id, submitted_at, file_ref, state FROM submission WHERE student_id = ?1",
                 studentId);
    std::vector<Submission> out;
    while (st.step()) out.push_back(readSubmission(st));
    return out;
}

std::vector<SubmissionRow> SqliteSubmissionRepository::listRowsByAssignment(int assignmentId) {
    Statement st(db_,
                 "SELECT s.id, s.assignment_id, s.student_id, st.student_no, u.username, s.submitted_at, s.file_ref, "
                 "       s.state, m.state, a.deadline, COALESCE(e.extra_days, 0) "
                 "FROM submission s "
                 "JOIN student st ON st.user_id = s.student_id "
                 "JOIN \"user\" u ON u.id = s.student_id "
                 "JOIN assignment a ON a.id = s.assignment_id "
                 "LEFT JOIN mark m ON m.submission_id = s.id "
                 "LEFT JOIN extension e ON e.student_id = s.student_id AND e.assignment_id = s.assignment_id "
                 "WHERE s.assignment_id = ?1 ORDER BY st.student_no",
                 assignmentId);
    std::vector<SubmissionRow> out;
    while (st.step()) {
        SubmissionRow r;
        r.submissionId = st.getInt(0);
        r.assignmentId = st.getInt(1);
        r.studentId = st.getInt(2);
        r.studentNo = st.getText(3);
        r.studentUsername = st.getText(4);
        r.submittedAt = DateTime::parse(st.getText(5));
        r.fileRef = st.getText(6);
        r.state = submissionStateFromDb(st.getText(7));
        if (!st.isNull(8)) r.markState = markStateFromDb(st.getText(8));
        r.deadline = DateTime::parse(st.getText(9));
        r.extraDays = st.getInt(10);
        out.push_back(std::move(r));
    }
    return out;
}

// ================================================================= marks
int SqliteMarkRepository::add(const Mark& m) {
    Statement(db_,
              "INSERT INTO mark(submission_id, raw_mark, final_mark, feedback, state) VALUES (?1, ?2, ?3, ?4, ?5)",
              m.submissionId(), m.rawMark(), m.finalMark(), m.feedback(), toDbString(m.state())).run();
    return static_cast<int>(db_.lastInsertId());
}

void SqliteMarkRepository::update(const Mark& m) {
    Statement(db_, "UPDATE mark SET raw_mark = ?1, final_mark = ?2, feedback = ?3, state = ?4 WHERE id = ?5",
              m.rawMark(), m.finalMark(), m.feedback(), toDbString(m.state()), m.id()).run();
}

std::optional<Mark> SqliteMarkRepository::findById(int id) {
    Statement st(db_, "SELECT id, submission_id, raw_mark, final_mark, feedback, state FROM mark WHERE id = ?1", id);
    if (!st.step()) return std::nullopt;
    return readMark(st);
}

std::optional<Mark> SqliteMarkRepository::findBySubmissionId(int submissionId) {
    Statement st(db_, "SELECT id, submission_id, raw_mark, final_mark, feedback, state FROM mark WHERE submission_id = ?1",
                 submissionId);
    if (!st.step()) return std::nullopt;
    return readMark(st);
}

std::optional<MarkRow> SqliteMarkRepository::findRow(int markId) {
    Statement st(db_, (std::string(kMarkRowSelect) + "WHERE m.id = ?1").c_str(), markId);
    if (!st.step()) return std::nullopt;
    return readMarkRow(st);
}

std::vector<MarkRow> SqliteMarkRepository::listForDepartment(int deptId, std::optional<MarkState> filter) {
    std::vector<MarkRow> out;
    if (filter) {
        Statement st(db_,
                     (std::string(kMarkRowSelect) + "WHERE c.dept_id = ?1 AND m.state = ?2 ORDER BY a.course_code, a.id, st.student_no").c_str(),
                     deptId, toDbString(*filter));
        while (st.step()) out.push_back(readMarkRow(st));
    } else {
        Statement st(db_,
                     (std::string(kMarkRowSelect) + "WHERE c.dept_id = ?1 ORDER BY a.course_code, a.id, st.student_no").c_str(),
                     deptId);
        while (st.step()) out.push_back(readMarkRow(st));
    }
    return out;
}

std::vector<MarkRow> SqliteMarkRepository::listForStudent(int studentId, bool onlyReleased) {
    std::string sql = std::string(kMarkRowSelect) + "WHERE s.student_id = ?1 ";
    if (onlyReleased) sql += "AND m.state = 'RELEASED' ";  // students can never see unreleased marks
    sql += "ORDER BY a.course_code, a.id";
    Statement st(db_, sql.c_str(), studentId);
    std::vector<MarkRow> out;
    while (st.step()) out.push_back(readMarkRow(st));
    return out;
}

// ================================================================= audit
int SqliteAuditRepository::append(const AuditEntry& e) {
    Statement(db_,
              "INSERT INTO audit_entry(entity_type, entity_id, old_value, new_value, actor_id, timestamp, reason) "
              "VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7)",
              e.entityType(), e.entityId(), e.oldValue(), e.newValue(), e.actorId(), e.timestamp(), e.reason()).run();
    return static_cast<int>(db_.lastInsertId());
}

namespace {
AuditEntry readAudit(const Statement& st) {
    return AuditEntry(st.getInt(0), st.getText(1), st.getInt(2), optText(st, 3), st.getText(4), st.getInt(5),
                      DateTime::parse(st.getText(6)), st.getText(7));
}
}  // namespace

std::vector<AuditEntry> SqliteAuditRepository::listForEntity(const std::string& entityType, int entityId) {
    Statement st(db_,
                 "SELECT id, entity_type, entity_id, old_value, new_value, actor_id, timestamp, reason FROM audit_entry "
                 "WHERE entity_type = ?1 AND entity_id = ?2 ORDER BY id",
                 entityType, entityId);
    std::vector<AuditEntry> out;
    while (st.step()) out.push_back(readAudit(st));
    return out;
}

std::vector<AuditEntry> SqliteAuditRepository::listRecent(int limit) {
    Statement st(db_,
                 "SELECT id, entity_type, entity_id, old_value, new_value, actor_id, timestamp, reason FROM audit_entry "
                 "ORDER BY id DESC LIMIT ?1",
                 limit);
    std::vector<AuditEntry> out;
    while (st.step()) out.push_back(readAudit(st));
    return out;
}

}  // namespace lms::data
