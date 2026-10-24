#pragma once
#include <vector>

#include "common/Clock.h"
#include "data/Repositories.h"

namespace lms {

/// Lecturer functions around assignments: FR-06 (create) and individual extensions (C4).
class AssignmentService {
public:
    AssignmentService(IAssignmentRepository& assignments, ICourseRepository& courses, ITeachingRepository& teaching,
                      IEnrollmentRepository& enrol, IExtensionRepository& extensions, IClock& clock,
                      ITransactionManager& tx)
        : assignments_(assignments), courses_(courses), teaching_(teaching), enrol_(enrol),
          extensions_(extensions), clock_(clock), tx_(tx) {}

    int createAssignment(const User& actor, const std::string& courseCode, const std::string& title,
                         DateTime deadline, double maxMark);
    int grantExtension(const User& actor, int assignmentId, int studentId, int extraDays, const std::string& reason);

    std::vector<Course> myCourses(const User& actor);
    std::vector<StudentInfo> studentsOfCourse(const User& actor, const std::string& courseCode);
    std::vector<Assignment> assignmentsOfCourse(const User& actor, const std::string& courseCode);

    static constexpr int kMaxExtensionDays = 30;
    static constexpr double kMaxMarkLimit = 1000.0;

private:
    IAssignmentRepository& assignments_;
    ICourseRepository& courses_;
    ITeachingRepository& teaching_;
    IEnrollmentRepository& enrol_;
    IExtensionRepository& extensions_;
    IClock& clock_;
    ITransactionManager& tx_;
};

}  // namespace lms
