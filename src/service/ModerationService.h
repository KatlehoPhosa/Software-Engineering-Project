#pragma once
#include <optional>
#include <vector>

#include "common/Clock.h"
#include "data/Repositories.h"
#include "service/MarkEvents.h"

namespace lms {

/// FR-10 and conflict C1: the Head of Department reviews, moderates and releases marks for
/// their own department. Students see a mark only after release().
class ModerationService {
public:
    ModerationService(IMarkRepository& marks, const MarkEventPublisher& events, IClock& clock,
                      ITransactionManager& tx)
        : marks_(marks), events_(events), clock_(clock), tx_(tx) {}

    std::vector<MarkRow> reviewMarks(const User& actor, std::optional<MarkState> filter = std::nullopt);

    /// PROVISIONAL -> MODERATED. Optionally adjusts the final mark; an adjustment needs a reason.
    void moderate(const User& actor, int markId, std::optional<double> adjustedFinal, const std::string& reason);

    /// MODERATED -> RELEASED for one mark.
    void release(const User& actor, int markId);
    /// Releases every MODERATED mark in the HoD's department; returns how many were released.
    int releaseAllModerated(const User& actor);

private:
    MarkRow loadOwnedRow(const HoD& hod, int markId);
    void releaseOne(const HoD& hod, int markId);

    IMarkRepository& marks_;
    const MarkEventPublisher& events_;
    IClock& clock_;
    ITransactionManager& tx_;
};

}  // namespace lms
