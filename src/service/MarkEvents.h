#pragma once
#include <optional>
#include <string>
#include <vector>

#include "common/DateTime.h"

namespace lms {

enum class MarkEventType { Marked, Remarked, Moderated, Released };

/// Something that happened to a mark. Observers decide what to do with it.
struct MarkEvent {
    MarkEventType type = MarkEventType::Marked;
    int markId = 0;
    int studentId = 0;
    int actorId = 0;
    std::optional<std::string> oldValue;
    std::string newValue;
    std::string reason;
    DateTime when;
    std::string description;  ///< e.g. "SCI101 / Lab report 1"
};

/// Observer pattern: services publish mark events without knowing who listens.
class SubmissionObserver {
public:
    virtual ~SubmissionObserver() = default;
    virtual void onMarkEvent(const MarkEvent& event) = 0;
};

class MarkEventPublisher {
public:
    void attach(SubmissionObserver* observer) { observers_.push_back(observer); }
    /// Observers run synchronously inside the caller's transaction, so a failing audit write
    /// rolls the mark change back: a mark can never change without a log entry.
    void publish(const MarkEvent& e) const {
        for (auto* o : observers_) o->onMarkEvent(e);
    }

private:
    std::vector<SubmissionObserver*> observers_;
};

}  // namespace lms
