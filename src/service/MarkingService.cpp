#include "service/MarkingService.h"

#include "common/Exceptions.h"
#include "common/Validation.h"
#include "service/AccessControl.h"

namespace lms {

std::vector<SubmissionRow> MarkingService::submissionsFor(const User& actor, int assignmentId) {
    requireAction(actor, Action::MarkSubmission);
    auto a = assignments_.find(assignmentId);
    if (!a) throw NotFoundError("Assignment not found.");
    if (!teaching_.isAssigned(actor.id(), a->courseCode())) throw AuthorizationError("You are not assigned to that course.");
    return submissions_.listRowsByAssignment(assignmentId);
}

MarkResult MarkingService::markSubmission(const User& actor, int submissionId, double rawMark,
                                          const std::string& feedback, const std::string& reason) {
    requireAction(actor, Action::MarkSubmission);
    const std::string fb = validation::requireNonBlank(feedback, "Feedback", 4000);

    auto tx = tx_.begin();
    auto sub = submissions_.findById(submissionId);
    if (!sub) throw NotFoundError("Submission not found.");
    auto a = assignments_.find(sub->assignmentId());
    if (!a) throw NotFoundError("Assignment not found.");
    if (!teaching_.isAssigned(actor.id(), a->courseCode())) throw AuthorizationError("You are not assigned to that course.");
    if (rawMark < 0.0 || rawMark > a->maxMark())
        throw ValidationError("Mark must be between 0 and the assignment maximum (" + std::to_string(a->maxMark()) + ").");

    auto ext = extensions_.find(sub->studentId(), sub->assignmentId());
    const DateTime effective = a->getEffectiveDeadline(ext ? ext->extraDays() : 0);
    const int late = sub->daysLate(effective);

    auto existing = marks_.findBySubmissionId(submissionId);
    MarkResult res;
    MarkEvent ev;
    ev.actorId = actor.id();
    ev.studentId = sub->studentId();
    ev.when = clock_.now();
    ev.description = a->courseCode() + " / " + a->title();

    if (!existing) {
        Mark m = Mark::provisional(submissionId, rawMark, fb);
        m.applyPenalty(penalty_, late);
        res.markId = marks_.add(m);
        res.wasRemark = false;
        res.rawMark = m.rawMark();
        res.finalMark = *m.finalMark();
        sub->markAsMarked();
        submissions_.update(*sub);
        ev.type = MarkEventType::Marked;
        ev.markId = res.markId;
        ev.oldValue = std::nullopt;
        ev.newValue = m.auditValue();
        ev.reason = validation::isBlank(reason) ? "Initial marking" : validation::trim(reason);
    } else {
        if (validation::isBlank(reason)) throw ValidationError("A reason is required when changing an existing mark.");
        const std::string before = existing->auditValue();
        existing->remark(rawMark, fb);  // BusinessRuleError if already moderated/released
        existing->applyPenalty(penalty_, late);
        marks_.update(*existing);
        res.markId = existing->id();
        res.wasRemark = true;
        res.rawMark = existing->rawMark();
        res.finalMark = *existing->finalMark();
        ev.type = MarkEventType::Remarked;
        ev.markId = existing->id();
        ev.oldValue = before;
        ev.newValue = existing->auditValue();
        ev.reason = validation::trim(reason);
    }
    res.daysLate = late;
    events_.publish(ev);  // audit entry is written in this same transaction
    tx->commit();
    return res;
}

}  // namespace lms
