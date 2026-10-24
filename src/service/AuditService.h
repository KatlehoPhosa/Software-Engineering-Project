#pragma once
#include <vector>

#include "data/Repositories.h"
#include "service/MarkEvents.h"

namespace lms {

/// Records every mark change (who, when, old value, new value, reason) and lets authorised
/// users read the history. Implements SubmissionObserver so mark services need not know it exists.
class AuditService final : public SubmissionObserver {
public:
    AuditService(IAuditRepository& audit, IMarkRepository& marks) : audit_(audit), marks_(marks) {}

    void onMarkEvent(const MarkEvent& e) override;

    /// Administrator: any mark. HoD: only marks of courses in their own department.
    std::vector<AuditEntry> historyOfMark(const User& actor, int markId);
    /// Administrator only.
    std::vector<AuditEntry> recent(const User& actor, int limit);

private:
    IAuditRepository& audit_;
    IMarkRepository& marks_;
};

}  // namespace lms
