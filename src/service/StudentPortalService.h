#pragma once
#include <optional>
#include <vector>

#include "data/Repositories.h"

namespace lms {

/// One line of the student's assignment list.
struct AssignmentView {
    Assignment assignment;
    std::optional<Submission> submission;
    LifecycleStage stage = LifecycleStage::NotSubmitted;
};

/// FR-07 and FR-11: read-only views for students.
class StudentPortalService {
public:
    StudentPortalService(IEnrollmentRepository& enrol, IAssignmentRepository& assignments,
                         ISubmissionRepository& submissions, IMarkRepository& marks)
        : enrol_(enrol), assignments_(assignments), submissions_(submissions), marks_(marks) {}

    std::vector<Course> myCourses(const User& actor);
    /// Only assignments of courses the student is enrolled in.
    std::vector<AssignmentView> myAssignments(const User& actor);
    /// Only RELEASED marks; enforced in the query and again here.
    std::vector<MarkRow> myReleasedMarks(const User& actor);

private:
    IEnrollmentRepository& enrol_;
    IAssignmentRepository& assignments_;
    ISubmissionRepository& submissions_;
    IMarkRepository& marks_;
};

}  // namespace lms
