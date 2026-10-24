#pragma once
#include <vector>

#include "common/Clock.h"
#include "data/Repositories.h"
#include "domain/PenaltyStrategy.h"
#include "service/MarkEvents.h"

namespace lms {

struct MarkResult {
    int markId = 0;
    double rawMark = 0;
    double finalMark = 0;
    int daysLate = 0;
    bool wasRemark = false;
};

/// FR-09: lecturers mark submissions and write feedback.
class MarkingService {
public:
    MarkingService(ISubmissionRepository& submissions, IMarkRepository& marks, IAssignmentRepository& assignments,
                   IExtensionRepository& extensions, ITeachingRepository& teaching, const IPenaltyStrategy& penalty,
                   const MarkEventPublisher& events, IClock& clock, ITransactionManager& tx)
        : submissions_(submissions), marks_(marks), assignments_(assignments), extensions_(extensions),
          teaching_(teaching), penalty_(penalty), events_(events), clock_(clock), tx_(tx) {}

    std::vector<SubmissionRow> submissionsFor(const User& actor, int assignmentId);

    /// Saves raw mark + feedback, applies the late penalty using the effective deadline, and
    /// logs the change. `reason` may be blank for the first marking only.
    MarkResult markSubmission(const User& actor, int submissionId, double rawMark, const std::string& feedback,
                              const std::string& reason);

private:
    ISubmissionRepository& submissions_;
    IMarkRepository& marks_;
    IAssignmentRepository& assignments_;
    IExtensionRepository& extensions_;
    ITeachingRepository& teaching_;
    const IPenaltyStrategy& penalty_;
    const MarkEventPublisher& events_;
    IClock& clock_;
    ITransactionManager& tx_;
};

}  // namespace lms
