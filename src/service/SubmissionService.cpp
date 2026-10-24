#include "service/SubmissionService.h"

#include "common/Exceptions.h"
#include "common/Validation.h"
#include "service/AccessControl.h"

namespace lms {

SubmitResult SubmissionService::submit(const User& actor, int assignmentId, const std::string& fileRef) {
    requireAction(actor, Action::SubmitAssignment);
    const std::string file = validation::requireNonBlank(fileRef, "File reference", 260);

    auto tx = tx_.begin();
    auto a = assignments_.find(assignmentId);
    if (!a) throw NotFoundError("Assignment not found.");
    if (!enrol_.isEnrolled(actor.id(), a->courseCode())) throw AuthorizationError("You are not enrolled in this course.");

    auto ext = extensions_.find(actor.id(), assignmentId);
    const DateTime effective = a->getEffectiveDeadline(ext ? ext->extraDays() : 0);
    const DateTime now = clock_.now();

    SubmitResult r;
    r.effectiveDeadline = effective;
    r.submittedAt = now;
    r.penaltyPolicy = penalty_.describe();

    auto existing = submissions_.find(actor.id(), assignmentId);
    if (!existing) {
        Submission s = Submission::createNew(actor.id(), assignmentId, now, effective, file);
        r.submissionId = submissions_.add(s);
        r.state = s.state();
        r.daysLate = s.daysLate(effective);
    } else {
        existing->resubmit(now, effective, file);  // throws BusinessRuleError if marked or past deadline
        submissions_.update(*existing);
        r.submissionId = existing->id();
        r.wasResubmission = true;
        r.state = existing->state();
        r.daysLate = 0;
    }
    tx->commit();
    return r;
}

}  // namespace lms
