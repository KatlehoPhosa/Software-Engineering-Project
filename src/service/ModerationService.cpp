#include "service/ModerationService.h"

#include "common/Exceptions.h"
#include "common/Validation.h"
#include "service/AccessControl.h"

namespace lms {
namespace {
const HoD& asHod(const User& u) { return static_cast<const HoD&>(u); }  // role verified by requireAction first
}  // namespace

std::vector<MarkRow> ModerationService::reviewMarks(const User& actor, std::optional<MarkState> filter) {
    requireAction(actor, Action::ReviewDepartmentMarks);
    return marks_.listForDepartment(asHod(actor).departmentId(), filter);
}

MarkRow ModerationService::loadOwnedRow(const HoD& hod, int markId) {
    auto row = marks_.findRow(markId);
    if (!row) throw NotFoundError("Mark not found.");
    if (row->departmentId != hod.departmentId())
        throw AuthorizationError("This mark belongs to a course outside your department.");
    return *row;
}

void ModerationService::moderate(const User& actor, int markId, std::optional<double> adjustedFinal,
                                 const std::string& reason) {
    requireAction(actor, Action::ModerateMark);
    const HoD& hod = asHod(actor);
    auto tx = tx_.begin();
    const MarkRow row = loadOwnedRow(hod, markId);
    auto mark = marks_.findById(markId);
    if (!mark) throw NotFoundError("Mark not found.");

    const std::string before = mark->auditValue();
    std::string why = validation::trim(reason);
    if (adjustedFinal) {
        if (*adjustedFinal < 0.0 || *adjustedFinal > row.maxMark)
            throw ValidationError("Adjusted mark must be between 0 and the assignment maximum.");
        if (why.empty()) throw ValidationError("A reason is required when you change a mark.");
        mark->adjustFinal(*adjustedFinal);
    } else if (why.empty()) {
        why = "Approved without change";
    }
    mark->moderate();
    marks_.update(*mark);

    MarkEvent ev;
    ev.type = MarkEventType::Moderated;
    ev.markId = markId;
    ev.studentId = row.studentId;
    ev.actorId = actor.id();
    ev.oldValue = before;
    ev.newValue = mark->auditValue();
    ev.reason = why;
    ev.when = clock_.now();
    ev.description = row.courseCode + " / " + row.assignmentTitle;
    events_.publish(ev);
    tx->commit();
}

void ModerationService::releaseOne(const HoD& hod, int markId) {
    const MarkRow row = loadOwnedRow(hod, markId);
    auto mark = marks_.findById(markId);
    if (!mark) throw NotFoundError("Mark not found.");
    const std::string before = mark->auditValue();
    mark->release();  // BusinessRuleError unless MODERATED
    marks_.update(*mark);

    MarkEvent ev;
    ev.type = MarkEventType::Released;
    ev.markId = markId;
    ev.studentId = row.studentId;
    ev.actorId = hod.id();
    ev.oldValue = before;
    ev.newValue = mark->auditValue();
    ev.reason = "Released to students";
    ev.when = clock_.now();
    ev.description = row.courseCode + " / " + row.assignmentTitle;
    events_.publish(ev);
}

void ModerationService::release(const User& actor, int markId) {
    requireAction(actor, Action::ReleaseMarks);
    auto tx = tx_.begin();
    releaseOne(asHod(actor), markId);
    tx->commit();
}

int ModerationService::releaseAllModerated(const User& actor) {
    requireAction(actor, Action::ReleaseMarks);
    const HoD& hod = asHod(actor);
    auto tx = tx_.begin();
    const auto rows = marks_.listForDepartment(hod.departmentId(), MarkState::Moderated);
    for (const auto& r : rows) releaseOne(hod, r.markId);
    tx->commit();
    return static_cast<int>(rows.size());
}

}  // namespace lms
