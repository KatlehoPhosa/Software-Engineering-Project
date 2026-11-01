#include "service/AuditService.h"

#include "common/Exceptions.h"
#include "common/Validation.h"
#include "service/AccessControl.h"

namespace lms {

void AuditService::onMarkEvent(const MarkEvent& e) {
    // entity_id has no foreign key, so the reference is checked here.
    if (!marks_.findById(e.markId)) throw NotFoundError("Cannot audit: mark does not exist.");
    const std::string reason = validation::requireNonBlank(e.reason, "Reason", 500);
    audit_.append(AuditEntry(0, "Mark", e.markId, e.oldValue, e.newValue, e.actorId, e.when, reason));
}

std::vector<AuditEntry> AuditService::historyOfMark(const User& actor, int markId) {
    if (actor.role() == UserRole::Admin) {
        requireAction(actor, Action::ViewAuditLog);
    } else if (actor.role() == UserRole::HoD) {
        requireAction(actor, Action::ReviewDepartmentMarks);
        auto row = marks_.findRow(markId);
        if (!row) throw NotFoundError("Mark not found.");
        if (row->departmentId != static_cast<const HoD&>(actor).departmentId())
            throw AuthorizationError("This mark belongs to another department.");
    } else {
        throw AuthorizationError("Your role is not allowed to view the audit log.");
    }
    return audit_.listForEntity("Mark", markId);
}

std::vector<AuditEntry> AuditService::recent(const User& actor, int limit) {
    requireAction(actor, Action::ViewAuditLog);
    if (limit < 1 || limit > 1000) throw ValidationError("Limit must be between 1 and 1000.");
    return audit_.listRecent(limit);
}

}  // namespace lms
