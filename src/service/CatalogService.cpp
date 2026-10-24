#include "service/CatalogService.h"

#include "common/Exceptions.h"
#include "common/Validation.h"
#include "service/AccessControl.h"

namespace lms {

int CatalogService::createDepartment(const User& actor, const std::string& name) {
    requireAction(actor, Action::ManageDepartments);
    const std::string n = validation::requireNonBlank(name, "Department name", 100);
    try {
        return depts_.add(n);
    } catch (const ConstraintViolation&) {
        throw BusinessRuleError("A department with that name already exists.");
    }
}

void CatalogService::renameDepartment(const User& actor, int id, const std::string& name) {
    requireAction(actor, Action::ManageDepartments);
    const std::string n = validation::requireNonBlank(name, "Department name", 100);
    if (!depts_.findById(id)) throw NotFoundError("Department not found.");
    try {
        depts_.rename(id, n);
    } catch (const ConstraintViolation&) {
        throw BusinessRuleError("A department with that name already exists.");
    }
}

void CatalogService::deleteDepartment(const User& actor, int id) {
    requireAction(actor, Action::ManageDepartments);
    if (!depts_.findById(id)) throw NotFoundError("Department not found.");
    try {
        depts_.remove(id);
    } catch (const ConstraintViolation&) {
        throw BusinessRuleError("The department still has courses or a Head of Department and cannot be deleted.");
    }
}

std::vector<Department> CatalogService::listDepartments(const User& actor) {
    requireAction(actor, Action::ManageDepartments);
    return depts_.list();
}

void CatalogService::createCourse(const User& actor, const std::string& code, const std::string& title, int deptId) {
    requireAction(actor, Action::ManageCourses);
    const std::string c = validation::requireCourseCode(code);
    const std::string t = validation::requireNonBlank(title, "Course title", 150);
    if (!depts_.findById(deptId)) throw NotFoundError("Department not found.");
    if (courses_.find(c)) throw BusinessRuleError("A course with that code already exists.");
    courses_.add(Course(c, t, deptId));
}

void CatalogService::updateCourseTitle(const User& actor, const std::string& code, const std::string& title) {
    requireAction(actor, Action::ManageCourses);
    const std::string t = validation::requireNonBlank(title, "Course title", 150);
    const std::string c = validation::requireCourseCode(code);
    if (!courses_.find(c)) throw NotFoundError("Course not found.");
    courses_.updateTitle(c, t);
}

void CatalogService::deleteCourse(const User& actor, const std::string& code) {
    requireAction(actor, Action::ManageCourses);
    const std::string c = validation::requireCourseCode(code);
    if (!courses_.find(c)) throw NotFoundError("Course not found.");
    try {
        courses_.remove(c);
    } catch (const ConstraintViolation&) {
        throw BusinessRuleError("The course has assignments and cannot be deleted.");
    }
}

std::vector<Course> CatalogService::listCourses(const User& actor) {
    requireAction(actor, Action::ManageCourses);
    return courses_.list();
}

void CatalogService::assignLecturer(const User& actor, int lecturerId, const std::string& courseCode) {
    requireAction(actor, Action::AssignLecturers);
    const std::string c = validation::requireCourseCode(courseCode);
    auto lecturer = users_.findById(lecturerId);
    if (!lecturer || lecturer->role() != UserRole::Lecturer) throw NotFoundError("Lecturer not found.");
    if (!courses_.find(c)) throw NotFoundError("Course not found.");
    if (teaching_.isAssigned(lecturerId, c)) throw BusinessRuleError("That lecturer is already assigned to the course.");
    teaching_.assign(lecturerId, c);
}

void CatalogService::unassignLecturer(const User& actor, int lecturerId, const std::string& courseCode) {
    requireAction(actor, Action::AssignLecturers);
    const std::string c = validation::requireCourseCode(courseCode);
    if (!teaching_.isAssigned(lecturerId, c)) throw NotFoundError("That lecturer is not assigned to the course.");
    teaching_.unassign(lecturerId, c);
}

void CatalogService::enrolStudent(const User& actor, int studentId, const std::string& courseCode) {
    requireAction(actor, Action::EnrolStudents);
    const std::string c = validation::requireCourseCode(courseCode);
    auto student = users_.findById(studentId);
    if (!student || student->role() != UserRole::Student) throw NotFoundError("Student not found.");
    if (!courses_.find(c)) throw NotFoundError("Course not found.");
    if (enrol_.isEnrolled(studentId, c)) throw BusinessRuleError("The student is already enrolled in that course.");
    enrol_.enrol(studentId, c);
}

void CatalogService::unenrolStudent(const User& actor, int studentId, const std::string& courseCode) {
    requireAction(actor, Action::EnrolStudents);
    const std::string c = validation::requireCourseCode(courseCode);
    if (!enrol_.isEnrolled(studentId, c)) throw NotFoundError("The student is not enrolled in that course.");
    enrol_.unenrol(studentId, c);
}

}  // namespace lms
