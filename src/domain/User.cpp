#include "domain/User.h"

#include <algorithm>

namespace lms {

bool User::can(Action a) const {
    const auto actions = allowedActions();
    return std::find(actions.begin(), actions.end(), a) != actions.end();
}

std::string label(Action a) {
    switch (a) {
        case Action::ChangeOwnPassword: return "Change my password";
        case Action::ManageUsers: return "Manage user accounts";
        case Action::ManageDepartments: return "Manage departments";
        case Action::ManageCourses: return "Manage courses";
        case Action::AssignLecturers: return "Assign lecturers to courses";
        case Action::EnrolStudents: return "Enrol students in courses";
        case Action::ViewAuditLog: return "View mark audit log";
        case Action::ViewTeachingCourses: return "View my courses and assignments";
        case Action::CreateAssignment: return "Create an assignment";
        case Action::GrantExtension: return "Grant a deadline extension";
        case Action::MarkSubmission: return "Mark submissions and give feedback";
        case Action::ReviewDepartmentMarks: return "Review department marks";
        case Action::ModerateMark: return "Moderate a mark";
        case Action::ReleaseMarks: return "Release moderated marks";
        case Action::ViewMyCourses: return "View my courses";
        case Action::ViewMyAssignments: return "View my assignments";
        case Action::SubmitAssignment: return "Submit or resubmit an assignment";
        case Action::ViewReleasedMarks: return "View my marks and feedback";
    }
    return "?";
}

std::vector<Action> Student::allowedActions() const {
    return {Action::ViewMyCourses, Action::ViewMyAssignments, Action::SubmitAssignment,
            Action::ViewReleasedMarks, Action::ChangeOwnPassword};
}

std::vector<Action> Lecturer::allowedActions() const {
    return {Action::ViewTeachingCourses, Action::CreateAssignment, Action::GrantExtension,
            Action::MarkSubmission, Action::ChangeOwnPassword};
}

std::vector<Action> HoD::allowedActions() const {
    return {Action::ReviewDepartmentMarks, Action::ModerateMark, Action::ReleaseMarks,
            Action::ChangeOwnPassword};
}

std::vector<Action> Admin::allowedActions() const {
    return {Action::ManageUsers, Action::ManageDepartments, Action::ManageCourses,
            Action::AssignLecturers, Action::EnrolStudents, Action::ViewAuditLog,
            Action::ChangeOwnPassword};
}

UserRecord Student::toRecord() const {
    UserRecord r;
    r.id = id(); r.username = username(); r.passwordHash = passwordHash();
    r.role = UserRole::Student; r.studentNo = studentNo_;
    return r;
}

UserRecord Lecturer::toRecord() const {
    UserRecord r;
    r.id = id(); r.username = username(); r.passwordHash = passwordHash();
    r.role = UserRole::Lecturer; r.staffNo = staffNo_;
    return r;
}

UserRecord HoD::toRecord() const {
    UserRecord r;
    r.id = id(); r.username = username(); r.passwordHash = passwordHash();
    r.role = UserRole::HoD; r.departmentId = departmentId_;
    return r;
}

UserRecord Admin::toRecord() const {
    UserRecord r;
    r.id = id(); r.username = username(); r.passwordHash = passwordHash();
    r.role = UserRole::Admin;
    return r;
}

}  // namespace lms
