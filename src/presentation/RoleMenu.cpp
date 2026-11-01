#include "presentation/RoleMenu.h"

#include <algorithm>
#include <filesystem>
#include <iterator>
#include <map>

#include "common/Exceptions.h"
#include "common/Validation.h"

namespace lms {

// ======================================================================= base
void RoleMenuBase::run(const User& user) {
    onLogin(user);
    const auto actions = user.allowedActions();
    for (;;) {
        std::vector<std::string> labels;
        std::transform(actions.begin(), actions.end(), std::back_inserter(labels), [](Action a) { return label(a); });
        const int pick = con_.choose(user.roleLabel() + " menu - " + user.username(), labels, "Log out");
        if (pick == 0) return;
        try {
            dispatch(user, actions[static_cast<std::size_t>(pick - 1)]);
        } catch (const LmsError& e) {
            con_.error(e.what());
        }
    }
}

void RoleMenuBase::changePassword(const User& user) {
    const std::string oldPw = con_.promptPassword("Current password");
    const std::string newPw = con_.promptPassword("New password (min 8 characters)");
    const std::string again = con_.promptPassword("Repeat new password");
    if (newPw != again) throw ValidationError("The two new passwords do not match.");
    sys_.auth.changeOwnPassword(user, oldPw, newPw);
    con_.println("Password changed. Use the new password next time you log in.");
}

namespace {

std::string pad(const std::string& s, std::size_t w) {
    return s.size() >= w ? s.substr(0, w) : s + std::string(w - s.size(), ' ');
}

void showCourses(Console& con, const std::vector<Course>& cs) {
    if (cs.empty()) { con.println("  (none)"); return; }
    for (const auto& c : cs) con.println("  " + pad(c.code(), 10) + c.title());
}

std::string markText(const MarkRow& r) {
    return r.finalMark ? Console::money(*r.finalMark) + " / " + Console::money(r.maxMark) : "-";
}

// ======================================================================= student
class StudentMenu final : public RoleMenuBase {
public:
    using RoleMenuBase::RoleMenuBase;

protected:
    void onLogin(const User& u) override {
        for (const auto& m : sys_.notifier.drain(u.id())) con_.println("  * " + m);
    }

    void dispatch(const User& u, Action a) override {
        switch (a) {
            case Action::ViewMyCourses:
                con_.heading("My courses");
                showCourses(con_, sys_.portal.myCourses(u));
                break;
            case Action::ViewMyAssignments: listAssignments(u); break;
            case Action::SubmitAssignment: submit(u); break;
            case Action::ViewReleasedMarks: marks(u); break;
            case Action::ChangeOwnPassword: changePassword(u); break;
            default: break;
        }
    }

private:
    std::vector<AssignmentView> listAssignments(const User& u) {
        auto list = sys_.portal.myAssignments(u);
        con_.heading("My assignments");
        if (list.empty()) con_.println("  (no assignments yet)");
        for (const auto& v : list) {
            con_.println("  [" + std::to_string(v.assignment.id()) + "] " + v.assignment.courseCode() + " - " +
                         v.assignment.title());
            std::string due = "       due " + con_.local(v.effectiveDeadline());
            if (v.extraDays > 0)
                due += " (extended by " + std::to_string(v.extraDays) + " day(s); original: " +
                       con_.local(v.assignment.deadline()) + ")";
            due += "  max " + Console::money(v.assignment.maxMark()) + "  status: " + label(v.stage);
            con_.println(due);
        }
        return list;
    }

    void submit(const User& u) {
        auto list = listAssignments(u);
        if (list.empty()) return;
        const int id = con_.promptInt("Assignment number to submit (0 to cancel)", 0, 1000000);
        if (id == 0) return;
        const std::string file = con_.prompt("Path of your file");
        std::error_code ec;
        if (!file.empty() && !std::filesystem::exists(file, ec) &&
            !con_.confirm("That file was not found on this computer. Submit the reference anyway?"))
            return;
        const SubmitResult r = sys_.submissionService.submit(u, id, file);
        con_.println(r.wasResubmission ? "Resubmission saved." : "Submission saved.");
        con_.println("Your deadline for this assignment: " + con_.local(r.effectiveDeadline));
        if (r.state == SubmissionState::Late)
            con_.println("This work is LATE by " + std::to_string(r.daysLate) + " day(s). Penalty rule: " + r.penaltyPolicy + ".");
        else
            con_.println("Received on time. You may resubmit until the deadline.");
    }

    void marks(const User& u) {
        con_.heading("My marks and feedback");
        auto rows = sys_.portal.myReleasedMarks(u);
        if (rows.empty()) con_.println("  No marks have been released to you yet.");
        for (const auto& r : rows) {
            con_.println("  " + r.courseCode + " - " + r.assignmentTitle + ": " + markText(r));
            if (r.daysLate() > 0) con_.println("       (submitted " + std::to_string(r.daysLate()) + " day(s) late)");
            con_.println("       Feedback: " + r.feedback.value_or("(none)"));
        }
    }
};

// ======================================================================= lecturer
class LecturerMenu final : public RoleMenuBase {
public:
    using RoleMenuBase::RoleMenuBase;

protected:
    void dispatch(const User& u, Action a) override {
        switch (a) {
            case Action::ViewTeachingCourses: overview(u); break;
            case Action::CreateAssignment: createAssignment(u); break;
            case Action::GrantExtension: grantExtension(u); break;
            case Action::MarkSubmission: mark(u); break;
            case Action::ChangeOwnPassword: changePassword(u); break;
            default: break;
        }
    }

private:
    std::string pickCourse(const User& u) {
        auto cs = sys_.assignmentService.myCourses(u);
        con_.heading("Your courses");
        showCourses(con_, cs);
        if (cs.empty()) throw BusinessRuleError("You are not assigned to any course yet. Ask the administrator.");
        return con_.prompt("Course code");
    }

    void printAssignments(const std::vector<Assignment>& as) {
        if (as.empty()) con_.println("  (no assignments)");
        for (const auto& a : as)
            con_.println("  [" + std::to_string(a.id()) + "] " + a.title() + "  due " + con_.local(a.deadline()) +
                         "  max " + Console::money(a.maxMark()));
    }

    void overview(const User& u) {
        for (const auto& c : sys_.assignmentService.myCourses(u)) {
            con_.heading(c.code() + " " + c.title());
            printAssignments(sys_.assignmentService.assignmentsOfCourse(u, c.code()));
        }
    }

    void createAssignment(const User& u) {
        const std::string code = pickCourse(u);
        const std::string title = con_.prompt("Assignment title");
        const DateTime deadline = con_.promptDateTime("Deadline");
        const double max = con_.promptDouble("Maximum mark", 0.01, 1000);
        const int id = sys_.assignmentService.createAssignment(u, code, title, deadline, max);
        con_.println("Assignment created with number " + std::to_string(id) + ".");
    }

    void grantExtension(const User& u) {
        const std::string code = pickCourse(u);
        printAssignments(sys_.assignmentService.assignmentsOfCourse(u, code));
        const int aid = con_.promptInt("Assignment number", 1, 1000000);

        con_.heading("Extension scope");
        con_.println("  1. Individual student");
        con_.println("  2. Whole class");
        const int scope = con_.promptInt("Choose extension scope", 1, 2);

        int studentId = 0;
        if (scope == 1) {
            con_.heading("Enrolled students");
            for (const auto& student : sys_.assignmentService.studentsOfCourse(u, code))
                con_.println("  [" + std::to_string(student.id) + "] " + student.studentNo + "  " + student.username);
            studentId = con_.promptInt("Student id (the number in brackets)", 1, 1000000);
        }

        const int days = con_.promptInt("Extra days (1-30)", 1, 30);
        const std::string why = con_.prompt("Reason for the extension");

        if (scope == 1) {
            sys_.assignmentService.grantExtension(u, aid, studentId, days, why);
            con_.println("Extension of " + std::to_string(days) + " day(s) granted to the student.");
        } else {
            sys_.assignmentService.grantClassExtension(u, aid, days, why);
            con_.println("Extension of " + std::to_string(days) + " day(s) granted to the whole class.");
        }
    }

    void mark(const User& u) {
        const std::string code = pickCourse(u);
        printAssignments(sys_.assignmentService.assignmentsOfCourse(u, code));
        const int aid = con_.promptInt("Assignment number", 1, 1000000);
        auto rows = sys_.marking.submissionsFor(u, aid);
        con_.heading("Submissions");
        if (rows.empty()) { con_.println("  (no submissions yet)"); return; }
        std::map<int, SubmissionRow> byId;
        for (const auto& r : rows) {
            byId.emplace(r.submissionId, r);
            std::string status = label(r.state);
            if (r.daysLate() > 0) status += ", " + std::to_string(r.daysLate()) + " day(s) late";
            if (r.markState) status += ", mark: " + label(*r.markState);
            con_.println("  [" + std::to_string(r.submissionId) + "] " + r.studentNo + " " + r.studentUsername + " - " +
                         status + " (" + r.fileRef + ")");
        }
        const int sid = con_.promptInt("Submission number to mark (0 to cancel)", 0, 1000000);
        if (sid == 0) return;
        auto it = byId.find(sid);
        if (it == byId.end()) throw NotFoundError("That submission is not in the list above.");
        const double raw = con_.promptDouble("Mark (before any late penalty)", 0, 1000);
        const std::string fb = con_.prompt("Written feedback for the student");
        std::string reason;
        if (it->second.markState) reason = con_.prompt("Reason for changing the existing mark");
        const MarkResult r = sys_.marking.markSubmission(u, sid, raw, fb, reason);
        con_.println(std::string(r.wasRemark ? "Mark updated" : "Mark saved") + ": raw " + Console::money(r.rawMark) +
                     ", after late penalty " + Console::money(r.finalMark) + " (" + std::to_string(r.daysLate) +
                     " day(s) late).");
        con_.println("The student will see it only after the Head of Department has moderated and released it.");
    }
};

// ======================================================================= head of department
class HodMenu final : public RoleMenuBase {
public:
    using RoleMenuBase::RoleMenuBase;

protected:
    void dispatch(const User& u, Action a) override {
        switch (a) {
            case Action::ReviewDepartmentMarks: review(u); break;
            case Action::ModerateMark: moderate(u); break;
            case Action::ReleaseMarks: release(u); break;
            case Action::ChangeOwnPassword: changePassword(u); break;
            default: break;
        }
    }

private:
    void printRows(const std::vector<MarkRow>& rows) {
        if (rows.empty()) { con_.println("  (no marks)"); return; }
        for (const auto& r : rows) {
            con_.println("  [" + std::to_string(r.markId) + "] " + r.courseCode + " / " + r.assignmentTitle + " / " +
                         r.studentNo + ": raw " + Console::money(r.rawMark) + ", final " + markText(r) + ", late " +
                         std::to_string(r.daysLate()) + "d, " + label(r.markState));
        }
    }

    void review(const User& u) {
        con_.heading("Department marks");
        printRows(sys_.moderation.reviewMarks(u));
        const int id = con_.promptInt("Mark number to see its change history (0 to skip)", 0, 1000000);
        if (id == 0) return;
        for (const auto& e : sys_.audit.historyOfMark(u, id)) {
            con_.println("  " + con_.local(e.timestamp()) + "  user#" + std::to_string(e.actorId()) + "  " +
                         e.oldValue().value_or("(none)") + " -> " + e.newValue() + "  because: " + e.reason());
        }
    }

    void moderate(const User& u) {
        con_.heading("Marks awaiting moderation");
        auto rows = sys_.moderation.reviewMarks(u, MarkState::Provisional);
        printRows(rows);
        if (rows.empty()) return;
        const int id = con_.promptInt("Mark number to moderate (0 to cancel)", 0, 1000000);
        if (id == 0) return;
        std::optional<double> adjusted;
        std::string reason;
        if (!con_.confirm("Approve the mark exactly as it is")) {
            adjusted = con_.promptDouble("New final mark", 0, 1000);
            reason = con_.prompt("Reason for changing the mark");
        }
        sys_.moderation.moderate(u, id, adjusted, reason);
        con_.println("Mark moderated. It is not visible to the student until you release it.");
    }

    void release(const User& u) {
        con_.heading("Moderated marks ready for release");
        auto rows = sys_.moderation.reviewMarks(u, MarkState::Moderated);
        printRows(rows);
        if (rows.empty()) return;
        if (con_.confirm("Release ALL of these marks to students")) {
            con_.println(std::to_string(sys_.moderation.releaseAllModerated(u)) + " mark(s) released.");
        } else {
            const int id = con_.promptInt("Mark number to release (0 to cancel)", 0, 1000000);
            if (id == 0) return;
            sys_.moderation.release(u, id);
            con_.println("Mark released.");
        }
    }
};

// ======================================================================= administrator
class AdminMenu final : public RoleMenuBase {
public:
    using RoleMenuBase::RoleMenuBase;

protected:
    void dispatch(const User& u, Action a) override {
        switch (a) {
            case Action::ManageUsers: users(u); break;
            case Action::ManageDepartments: departments(u); break;
            case Action::ManageCourses: courses(u); break;
            case Action::AssignLecturers: link(u, true); break;
            case Action::EnrolStudents: link(u, false); break;
            case Action::ViewAuditLog: audit(u); break;
            case Action::ChangeOwnPassword: changePassword(u); break;
            default: break;
        }
    }

private:
    void listUsers(const User& u, std::optional<UserRole> only = std::nullopt) {
        for (const auto& r : sys_.accounts.listUsers(u)) {
            if (only && r.role != *only) continue;
            std::string extra = r.role == UserRole::Student ? r.studentNo : r.role == UserRole::Lecturer ? r.staffNo
                              : r.role == UserRole::HoD ? "dept#" + std::to_string(r.departmentId) : "";
            con_.println("  [" + std::to_string(r.id) + "] " + pad(r.username, 18) + pad(label(r.role), 20) + extra);
        }
    }

    void users(const User& u) {
        for (;;) {
            const int c = con_.choose("User accounts", {"List users", "Create user", "Reset a password", "Rename a user", "Delete a user"});
            if (c == 0) return;
            try {
                if (c == 1) { con_.heading("Users"); listUsers(u); }
                else if (c == 2) createUser(u);
                else if (c == 3) {
                    listUsers(u);
                    const int id = con_.promptInt("User id", 1, 1000000);
                    sys_.accounts.resetPassword(u, id, con_.promptPassword("New password"));
                    con_.println("Password reset.");
                } else if (c == 4) {
                    listUsers(u);
                    const int id = con_.promptInt("User id", 1, 1000000);
                    sys_.accounts.renameUser(u, id, con_.prompt("New username"));
                    con_.println("Username changed.");
                } else if (c == 5) {
                    listUsers(u);
                    const int id = con_.promptInt("User id", 1, 1000000);
                    if (con_.confirm("Delete this account permanently")) {
                        sys_.accounts.deleteUser(u, id);
                        con_.println("Account deleted.");
                    }
                }
            } catch (const LmsError& e) { con_.error(e.what()); }
        }
    }

    void createUser(const User& u) {
        const int r = con_.choose("Role of the new user", {"Student", "Lecturer", "Head of Department", "Administrator"}, "Cancel");
        if (r == 0) return;
        NewUserSpec s;
        s.role = r == 1 ? UserRole::Student : r == 2 ? UserRole::Lecturer : r == 3 ? UserRole::HoD : UserRole::Admin;
        s.username = con_.prompt("Username");
        s.password = con_.promptPassword("Initial password (min 8 characters)");
        if (s.role == UserRole::Student) s.studentNo = con_.prompt("Student number");
        if (s.role == UserRole::Lecturer) s.staffNo = con_.prompt("Staff number");
        if (s.role == UserRole::HoD) {
            listDepartments(u);
            s.departmentId = con_.promptInt("Department id", 1, 1000000);
        }
        const int id = sys_.accounts.createUser(u, s);
        con_.println("Account created with id " + std::to_string(id) + ".");
    }

    void listDepartments(const User& u) {
        con_.heading("Departments");
        for (const auto& d : sys_.catalog.listDepartments(u)) con_.println("  [" + std::to_string(d.id()) + "] " + d.name());
    }

    void departments(const User& u) {
        for (;;) {
            const int c = con_.choose("Departments", {"List", "Create", "Rename", "Delete"});
            if (c == 0) return;
            try {
                if (c == 1) listDepartments(u);
                else if (c == 2) con_.println("Created with id " + std::to_string(sys_.catalog.createDepartment(u, con_.prompt("Department name"))) + ".");
                else if (c == 3) {
                    listDepartments(u);
                    const int id = con_.promptInt("Department id", 1, 1000000);
                    sys_.catalog.renameDepartment(u, id, con_.prompt("New name"));
                    con_.println("Renamed.");
                } else if (c == 4) {
                    listDepartments(u);
                    const int id = con_.promptInt("Department id", 1, 1000000);
                    if (con_.confirm("Delete this department")) { sys_.catalog.deleteDepartment(u, id); con_.println("Deleted."); }
                }
            } catch (const LmsError& e) { con_.error(e.what()); }
        }
    }

    void courses(const User& u) {
        for (;;) {
            const int c = con_.choose("Courses", {"List", "Create", "Change title", "Delete"});
            if (c == 0) return;
            try {
                if (c == 1) { con_.heading("Courses"); showCourses(con_, sys_.catalog.listCourses(u)); }
                else if (c == 2) {
                    listDepartments(u);
                    const std::string code = con_.prompt("Course code (e.g. SCI101)");
                    const std::string title = con_.prompt("Course title");
                    const int dept = con_.promptInt("Department id", 1, 1000000);
                    sys_.catalog.createCourse(u, code, title, dept);
                    con_.println("Course created.");
                } else if (c == 3) {
                    showCourses(con_, sys_.catalog.listCourses(u));
                    const std::string code = con_.prompt("Course code");
                    sys_.catalog.updateCourseTitle(u, code, con_.prompt("New title"));
                    con_.println("Title changed.");
                } else if (c == 4) {
                    showCourses(con_, sys_.catalog.listCourses(u));
                    const std::string code = con_.prompt("Course code");
                    if (con_.confirm("Delete this course")) { sys_.catalog.deleteCourse(u, code); con_.println("Deleted."); }
                }
            } catch (const LmsError& e) { con_.error(e.what()); }
        }
    }

    void link(const User& u, bool lecturers) {
        const std::string who = lecturers ? "lecturer" : "student";
        const int c = con_.choose(lecturers ? "Lecturer assignments" : "Student enrolments", {"Add", "Remove"}, "Cancel");
        if (c == 0) return;
        con_.heading(lecturers ? "Lecturers" : "Students");
        listUsers(u, lecturers ? UserRole::Lecturer : UserRole::Student);
        const int id = con_.promptInt("Id of the " + who, 1, 1000000);
        con_.heading("Courses");
        showCourses(con_, sys_.catalog.listCourses(u));
        const std::string code = con_.prompt("Course code");
        if (lecturers) {
            c == 1 ? sys_.catalog.assignLecturer(u, id, code) : sys_.catalog.unassignLecturer(u, id, code);
        } else {
            c == 1 ? sys_.catalog.enrolStudent(u, id, code) : sys_.catalog.unenrolStudent(u, id, code);
        }
        con_.println("Done.");
    }

    void audit(const User& u) {
        const int n = con_.promptInt("How many recent entries (1-1000)", 1, 1000);
        con_.heading("Mark audit log (newest first)");
        for (const auto& e : sys_.audit.recent(u, n))
            con_.println("  " + con_.local(e.timestamp()) + "  " + e.entityType() + "#" + std::to_string(e.entityId()) +
                         "  by user#" + std::to_string(e.actorId()) + "  " + e.oldValue().value_or("(none)") + " -> " +
                         e.newValue() + "  because: " + e.reason());
    }
};

}  // namespace

std::unique_ptr<IRoleMenu> makeRoleMenu(const User& user, System& sys, Console& con) {
    switch (user.role()) {
        case UserRole::Student: return std::make_unique<StudentMenu>(sys, con);
        case UserRole::Lecturer: return std::make_unique<LecturerMenu>(sys, con);
        case UserRole::HoD: return std::make_unique<HodMenu>(sys, con);
        case UserRole::Admin: return std::make_unique<AdminMenu>(sys, con);
    }
    throw LmsError("Unknown role.");
}

}  // namespace lms
