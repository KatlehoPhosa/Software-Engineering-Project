#pragma once
#include <string>

namespace lms {

/// Every operation a signed-in user can attempt. Each User subclass declares which of these
/// its role may perform (role-based access control, FR-02); services enforce it.
enum class Action {
    ChangeOwnPassword,
    // Administrator
    ManageUsers,
    ManageDepartments,
    ManageCourses,
    AssignLecturers,
    EnrolStudents,
    ViewAuditLog,
    // Lecturer
    ViewTeachingCourses,
    CreateAssignment,
    GrantExtension,
    MarkSubmission,
    // Head of Department
    ReviewDepartmentMarks,
    ModerateMark,
    ReleaseMarks,
    // Student
    ViewMyCourses,
    ViewMyAssignments,
    SubmitAssignment,
    ViewReleasedMarks
};

std::string label(Action a);

}  // namespace lms
