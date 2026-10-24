#pragma once
#include "common/Clock.h"
#include "data/Repositories.h"
#include "domain/PenaltyStrategy.h"

namespace lms {

struct SubmitResult {
    int submissionId = 0;
    bool wasResubmission = false;
    SubmissionState state = SubmissionState::Submitted;
    int daysLate = 0;
    DateTime effectiveDeadline;
    DateTime submittedAt;
    std::string penaltyPolicy;  ///< description of the active late-penalty rule
};

/// FR-08: submit and resubmit (conflicts C2, C3, C4).
class SubmissionService {
public:
    SubmissionService(ISubmissionRepository& submissions, IAssignmentRepository& assignments,
                      IEnrollmentRepository& enrol, IExtensionRepository& extensions,
                      const IPenaltyStrategy& penalty, IClock& clock, ITransactionManager& tx)
        : submissions_(submissions), assignments_(assignments), enrol_(enrol), extensions_(extensions),
          penalty_(penalty), clock_(clock), tx_(tx) {}

    /// First submission (on time -> SUBMITTED, after the effective deadline -> LATE, never rejected),
    /// or resubmission (only before the effective deadline and before marking -> RESUBMITTED).
    SubmitResult submit(const User& actor, int assignmentId, const std::string& fileRef);

private:
    ISubmissionRepository& submissions_;
    IAssignmentRepository& assignments_;
    IEnrollmentRepository& enrol_;
    IExtensionRepository& extensions_;
    const IPenaltyStrategy& penalty_;
    IClock& clock_;
    ITransactionManager& tx_;
};

}  // namespace lms
