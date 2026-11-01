#include "domain/Entities.h"

#include <cmath>
#include <iomanip>
#include <sstream>

#include "common/Exceptions.h"
#include "common/Validation.h"

namespace lms {
namespace {
std::string num(double v) {
    std::ostringstream o;
    o << std::fixed << std::setprecision(2) << v;
    return o.str();
}
}  // namespace

Submission Submission::createNew(int studentId, int assignmentId, DateTime now, DateTime effectiveDeadline,
                                 std::string fileRef) {
    const SubmissionState st = now <= effectiveDeadline ? SubmissionState::Submitted : SubmissionState::Late;
    return Submission(0, studentId, assignmentId, now, std::move(fileRef), st);
}

void Submission::resubmit(DateTime now, DateTime effectiveDeadline, std::string fileReference) {
    if (state_ == SubmissionState::Marked)
        throw BusinessRuleError("This work has already been marked and can no longer be resubmitted.");
    if (now > effectiveDeadline)
        throw BusinessRuleError("The deadline has passed: resubmission is only allowed before the deadline.");
    submittedAt_ = now;
    fileRef_ = std::move(fileReference);
    state_ = SubmissionState::Resubmitted;
}

int Submission::daysLate(DateTime effectiveDeadline) const { return lateDays(submittedAt_, effectiveDeadline); }

Mark Mark::provisional(int submissionId, double rawMark, std::string feedback) {
    return Mark(0, submissionId, rawMark, std::nullopt, std::move(feedback), MarkState::Provisional);
}

void Mark::remark(double rawMarkValue, std::string feedbackText) {
    if (state_ != MarkState::Provisional)
        throw BusinessRuleError("This mark is already moderated; only the Head of Department can change it.");
    rawMark_ = rawMarkValue;
    feedback_ = std::move(feedbackText);
    finalMark_.reset();
}

void Mark::applyPenalty(const IPenaltyStrategy& strategy, int daysLate) {
    finalMark_ = strategy.apply(rawMark_, daysLate);
}

void Mark::adjustFinal(double newFinal) {
    if (state_ != MarkState::Provisional)
        throw BusinessRuleError("Only a mark awaiting moderation can be adjusted.");
    if (newFinal < 0.0) throw ValidationError("Final mark must not be negative.");
    finalMark_ = newFinal;
}

void Mark::moderate() {
    if (state_ != MarkState::Provisional) throw BusinessRuleError("Only a mark awaiting moderation can be moderated.");
    if (!finalMark_) throw BusinessRuleError("The mark has no final value yet.");
    state_ = MarkState::Moderated;
}

void Mark::release() {
    if (state_ != MarkState::Moderated) throw BusinessRuleError("A mark must be moderated before it is released.");
    state_ = MarkState::Released;
}

std::string Mark::auditValue() const {
    return "raw=" + num(rawMark_) + ";final=" + (finalMark_ ? num(*finalMark_) : std::string("none")) +
           ";state=" + toDbString(state_);
}

LifecycleStage lifecycleOf(const Submission* s, const Mark* m) {
    if (!s) return LifecycleStage::NotSubmitted;
    if (m) {
        switch (m->state()) {
            case MarkState::Released: return LifecycleStage::Released;
            case MarkState::Moderated: return LifecycleStage::Moderated;
            case MarkState::Provisional: return LifecycleStage::Marked;
        }
    }
    switch (s->state()) {
        case SubmissionState::Submitted: return LifecycleStage::Submitted;
        case SubmissionState::Late: return LifecycleStage::Late;
        case SubmissionState::Resubmitted: return LifecycleStage::Resubmitted;
        case SubmissionState::Marked: return LifecycleStage::Marked;
    }
    return LifecycleStage::NotSubmitted;
}

std::string label(LifecycleStage s) {
    switch (s) {
        case LifecycleStage::NotSubmitted: return "Not submitted";
        case LifecycleStage::Submitted: return "Submitted";
        case LifecycleStage::Late: return "Submitted late";
        case LifecycleStage::Resubmitted: return "Resubmitted";
        case LifecycleStage::Marked: return "Marked (awaiting moderation)";
        case LifecycleStage::Moderated: return "Moderated";
        case LifecycleStage::Released: return "Released";
    }
    return "?";
}

}  // namespace lms
