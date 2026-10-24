#pragma once
#include <map>
#include <string>
#include <vector>

#include "service/MarkEvents.h"

namespace lms {

/// Observer that tells a student when a mark has been released.
/// v1.0 keeps notifications in memory for the running session only: the approved
/// database design (Gate 2) has no notification table.
class StudentNotifier final : public SubmissionObserver {
public:
    void onMarkEvent(const MarkEvent& e) override {
        if (e.type == MarkEventType::Released)
            pending_[e.studentId].push_back("Your mark for " + e.description + " has been released.");
    }
    /// Returns and clears the messages waiting for this student.
    std::vector<std::string> drain(int studentId) {
        auto it = pending_.find(studentId);
        if (it == pending_.end()) return {};
        std::vector<std::string> out = std::move(it->second);
        pending_.erase(it);
        return out;
    }

private:
    std::map<int, std::vector<std::string>> pending_;
};

}  // namespace lms
