#include "service/AssignmentService.h"

#include "common/Exceptions.h"
#include "common/Validation.h"
#include "service/AccessControl.h"

namespace lms {

int AssignmentService::createAssignment(const User& actor, const std::string& courseCode, const std::string& title,
                                        DateTime deadline, double maxMark) {
    requireAction(actor, Action::CreateAssignment);
    const std::string code = validation::requireCourseCode(courseCode);
    const std::string t = validation::requireNonBlank(title, "Title", 200);
    if (!(maxMark > 0.0) || maxMark > kMaxMarkLimit)
        throw ValidationError("Maximum mark must be greater than 0 and at most 1000.");
    if (deadline <= clock_.now()) throw ValidationError("The deadline must be in the future.");
    if (!courses_.find(code)) throw NotFoundError("Course not found.");
    if (!teaching_.isAssigned(actor.id(), code)) throw AuthorizationError("You are not assigned to that course.");
    return assignments_.add(Assignment(0, t, deadline, maxMark, code, actor.id()));
}

int AssignmentService::grantExtension(const User& actor, int assignmentId, int studentId, int extraDays,
                                      const std::string& reason) {
    requireAction(actor, Action::GrantExtension);
    if (extraDays < 1 || extraDays > kMaxExtensionDays)
        throw ValidationError("An extension must be between 1 and 30 days.");
    const std::string why = validation::requireNonBlank(reason, "Reason", 500);
    auto tx = tx_.begin();
    auto a = assignments_.find(assignmentId);
    if (!a) throw NotFoundError("Assignment not found.");
    if (!teaching_.isAssigned(actor.id(), a->courseCode())) throw AuthorizationError("You are not assigned to that course.");
    if (!enrol_.isEnrolled(studentId, a->courseCode())) throw BusinessRuleError("That student is not enrolled in the course.");
    if (extensions_.find(studentId, assignmentId))
        throw BusinessRuleError("This student already has an extension for the assignment (one per student in v1.0).");
    int id = 0;
    try {
        id = extensions_.add(Extension(0, studentId, assignmentId, extraDays, actor.id(), why));
    } catch (const ConstraintViolation&) {
        throw BusinessRuleError("The extension could not be saved (it may already exist).");
    }
    tx->commit();
    return id;
}


int AssignmentService::grantClassExtension(const User& actor, int assignmentId, int extraDays,
                                           const std::string& reason) {
    requireAction(actor, Action::GrantExtension);
    if (extraDays < 1 || extraDays > kMaxExtensionDays)
        throw ValidationError("An extension must be between 1 and 30 days.");
    const std::string why = validation::requireNonBlank(reason, "Reason", 500);

    auto tx = tx_.begin();
    auto a = assignments_.find(assignmentId);
    if (!a) throw NotFoundError("Assignment not found.");
    if (!teaching_.isAssigned(actor.id(), a->courseCode()))
        throw AuthorizationError("You are not assigned to that course.");

    const auto students = enrol_.studentsOfCourse(a->courseCode());
    if (students.empty()) throw BusinessRuleError("There are no enrolled students in this class.");

    // Pre-check everyone before inserting, so an existing extension rejects the whole request.
    for (const auto& student : students) {
        if (extensions_.find(student.id, assignmentId))
            throw BusinessRuleError("At least one student already has an extension for this assignment.");
    }

    int firstId = 0;
    try {
        for (const auto& student : students) {
            const int id = extensions_.add(
                Extension(0, student.id, assignmentId, extraDays, actor.id(), why));
            if (firstId == 0) firstId = id;
        }
    } catch (const ConstraintViolation&) {
        throw BusinessRuleError("The class extension could not be saved; no changes were committed.");
    }

    tx->commit();
    return firstId;
}

std::vector<Course> AssignmentService::myCourses(const User& actor) {
    requireAction(actor, Action::ViewTeachingCourses);
    return teaching_.coursesOfLecturer(actor.id());
}

std::vector<Assignment> AssignmentService::assignmentsOfCourse(const User& actor, const std::string& courseCode) {
    requireAction(actor, Action::ViewTeachingCourses);
    const std::string code = validation::requireCourseCode(courseCode);
    if (!teaching_.isAssigned(actor.id(), code)) throw AuthorizationError("You are not assigned to that course.");
    return assignments_.listByCourse(code);
}

std::vector<StudentInfo> AssignmentService::studentsOfCourse(const User& actor, const std::string& courseCode) {
    requireAction(actor, Action::ViewTeachingCourses);
    const std::string code = validation::requireCourseCode(courseCode);
    if (!teaching_.isAssigned(actor.id(), code)) throw AuthorizationError("You are not assigned to that course.");
    return enrol_.studentsOfCourse(code);
}

}  // namespace lms
