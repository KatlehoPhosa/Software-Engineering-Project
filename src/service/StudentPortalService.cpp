#include "service/StudentPortalService.h"

#include <map>

#include "service/AccessControl.h"

namespace lms {

std::vector<Course> StudentPortalService::myCourses(const User& actor) {
    requireAction(actor, Action::ViewMyCourses);
    return enrol_.coursesOfStudent(actor.id());
}

std::vector<AssignmentView> StudentPortalService::myAssignments(const User& actor) {
    requireAction(actor, Action::ViewMyAssignments);
    std::map<int, Submission> mine;
    for (auto& s : submissions_.listByStudent(actor.id())) mine.emplace(s.assignmentId(), s);

    std::vector<AssignmentView> out;
    for (const auto& c : enrol_.coursesOfStudent(actor.id())) {
        for (const auto& a : assignments_.listByCourse(c.code())) {
            AssignmentView v{a, std::nullopt, LifecycleStage::NotSubmitted};
            auto it = mine.find(a.id());
            if (it != mine.end()) {
                v.submission = it->second;
                std::optional<Mark> m = marks_.findBySubmissionId(it->second.id());
                v.stage = lifecycleOf(&it->second, m ? &*m : nullptr);
            }
            out.push_back(std::move(v));
        }
    }
    return out;
}

std::vector<MarkRow> StudentPortalService::myReleasedMarks(const User& actor) {
    requireAction(actor, Action::ViewReleasedMarks);
    auto rows = marks_.listForStudent(actor.id(), true);
    std::vector<MarkRow> safe;
    for (auto& r : rows)
        if (r.markState == MarkState::Released) safe.push_back(std::move(r));  // defence in depth
    return safe;
}

}  // namespace lms
