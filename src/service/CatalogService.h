#pragma once
#include <vector>

#include "data/Repositories.h"

namespace lms {

/// FR-04 / FR-05: departments, courses, lecturer assignment and student enrolment (administrator only).
class CatalogService {
public:
    CatalogService(IDepartmentRepository& depts, ICourseRepository& courses, ITeachingRepository& teaching,
                   IEnrollmentRepository& enrol, IUserRepository& users, ITransactionManager& tx)
        : depts_(depts), courses_(courses), teaching_(teaching), enrol_(enrol), users_(users), tx_(tx) {}

    int createDepartment(const User& actor, const std::string& name);
    void renameDepartment(const User& actor, int id, const std::string& name);
    void deleteDepartment(const User& actor, int id);
    std::vector<Department> listDepartments(const User& actor);

    void createCourse(const User& actor, const std::string& code, const std::string& title, int deptId);
    void updateCourseTitle(const User& actor, const std::string& code, const std::string& title);
    void deleteCourse(const User& actor, const std::string& code);
    std::vector<Course> listCourses(const User& actor);

    void assignLecturer(const User& actor, int lecturerId, const std::string& courseCode);
    void unassignLecturer(const User& actor, int lecturerId, const std::string& courseCode);
    void enrolStudent(const User& actor, int studentId, const std::string& courseCode);
    void unenrolStudent(const User& actor, int studentId, const std::string& courseCode);

private:
    IDepartmentRepository& depts_;
    ICourseRepository& courses_;
    ITeachingRepository& teaching_;
    IEnrollmentRepository& enrol_;
    IUserRepository& users_;
    ITransactionManager& tx_;
};

}  // namespace lms
