#pragma once
#include <optional>
#include <string>

#include "common/DateTime.h"
#include "domain/Enums.h"
#include "domain/PenaltyStrategy.h"

namespace lms {

/// Whole days late, rounded up (1 second late = 1 day late); 0 when on time.
inline int lateDays(DateTime submittedAt, DateTime effectiveDeadline) {
    if (submittedAt <= effectiveDeadline) return 0;
    const std::int64_t diff = submittedAt.epochSeconds() - effectiveDeadline.epochSeconds();
    return static_cast<int>((diff + DateTime::kSecondsPerDay - 1) / DateTime::kSecondsPerDay);
}

class Department {
public:
    Department(int id, std::string name) : id_(id), name_(std::move(name)) {}
    int id() const { return id_; }
    const std::string& name() const { return name_; }

private:
    int id_;
    std::string name_;
};

class Course {
public:
    Course(std::string code, std::string title, int deptId)
        : code_(std::move(code)), title_(std::move(title)), deptId_(deptId) {}
    const std::string& code() const { return code_; }
    const std::string& title() const { return title_; }
    int deptId() const { return deptId_; }

private:
    std::string code_;
    std::string title_;
    int deptId_;
};

class Assignment {
public:
    Assignment(int id, std::string title, DateTime deadline, double maxMark, std::string courseCode,
               int createdBy)
        : id_(id), title_(std::move(title)), deadline_(deadline), maxMark_(maxMark),
          courseCode_(std::move(courseCode)), createdBy_(createdBy) {}
    int id() const { return id_; }
    const std::string& title() const { return title_; }
    DateTime deadline() const { return deadline_; }
    double maxMark() const { return maxMark_; }
    const std::string& courseCode() const { return courseCode_; }
    int createdBy() const { return createdBy_; }

    /// Original deadline plus the student's extension (conflict C4).
    DateTime getEffectiveDeadline(int extraDays) const { return deadline_.addDays(extraDays); }

private:
    int id_;
    std::string title_;
    DateTime deadline_;
    double maxMark_;
    std::string courseCode_;
    int createdBy_;
};

class Extension {
public:
    Extension(int id, int studentId, int assignmentId, int extraDays, int grantedBy, std::string reason)
        : id_(id), studentId_(studentId), assignmentId_(assignmentId), extraDays_(extraDays),
          grantedBy_(grantedBy), reason_(std::move(reason)) {}
    int id() const { return id_; }
    int studentId() const { return studentId_; }
    int assignmentId() const { return assignmentId_; }
    int extraDays() const { return extraDays_; }
    int grantedBy() const { return grantedBy_; }
    const std::string& reason() const { return reason_; }

private:
    int id_, studentId_, assignmentId_, extraDays_, grantedBy_;
    std::string reason_;
};

/// A student's submission and its life cycle:
///   (none) -> SUBMITTED | LATE  --resubmit before effective deadline--> RESUBMITTED --> MARKED
/// MODERATED and RELEASED belong to the Mark (see Mark::state); the combined view is
/// LifecycleStage in `lifecycleOf()`.
class Submission {
public:
    Submission(int id, int studentId, int assignmentId, DateTime submittedAt, std::string fileRef,
               SubmissionState state)
        : id_(id), studentId_(studentId), assignmentId_(assignmentId), submittedAt_(submittedAt),
          fileRef_(std::move(fileRef)), state_(state) {}

    /// First submission: SUBMITTED if on time, LATE if after the effective deadline.
    static Submission createNew(int studentId, int assignmentId, DateTime now, DateTime effectiveDeadline,
                                std::string fileRef);

    /// Replace the file. Only allowed before the effective deadline and before marking.
    /// Throws BusinessRuleError otherwise.
    void resubmit(DateTime now, DateTime effectiveDeadline, std::string fileRef);

    /// Whole days late, rounded up (1 second late = 1 day late). 0 when on time.
    int daysLate(DateTime effectiveDeadline) const;

    void markAsMarked() { state_ = SubmissionState::Marked; }

    int id() const { return id_; }
    int studentId() const { return studentId_; }
    int assignmentId() const { return assignmentId_; }
    DateTime submittedAt() const { return submittedAt_; }
    const std::string& fileRef() const { return fileRef_; }
    SubmissionState state() const { return state_; }

private:
    int id_, studentId_, assignmentId_;
    DateTime submittedAt_;
    std::string fileRef_;
    SubmissionState state_;
};

class Mark {
public:
    Mark(int id, int submissionId, double rawMark, std::optional<double> finalMark,
         std::optional<std::string> feedback, MarkState state)
        : id_(id), submissionId_(submissionId), rawMark_(rawMark), finalMark_(finalMark),
          feedback_(std::move(feedback)), state_(state) {}

    /// New mark written by a lecturer; the final mark is computed by applyPenalty().
    static Mark provisional(int submissionId, double rawMark, std::string feedback);

    /// Lecturer re-marks. Allowed only while PROVISIONAL.
    void remark(double rawMark, std::string feedback);
    /// Final mark = strategy(raw, daysLate).
    void applyPenalty(const IPenaltyStrategy& strategy, int daysLate);
    /// HoD adjusts the final mark during moderation (state must be PROVISIONAL).
    void adjustFinal(double newFinal);
    void moderate();  ///< PROVISIONAL -> MODERATED
    void release();   ///< MODERATED   -> RELEASED

    int id() const { return id_; }
    int submissionId() const { return submissionId_; }
    double rawMark() const { return rawMark_; }
    const std::optional<double>& finalMark() const { return finalMark_; }
    const std::optional<std::string>& feedback() const { return feedback_; }
    MarkState state() const { return state_; }

    /// Text recorded in the audit log, e.g. "raw=15;final=13.5;state=PROVISIONAL".
    std::string auditValue() const;

private:
    int id_, submissionId_;
    double rawMark_;
    std::optional<double> finalMark_;
    std::optional<std::string> feedback_;
    MarkState state_;
};

/// Combined view of Submission + Mark used by the state machine of the design.
enum class LifecycleStage { NotSubmitted, Submitted, Late, Resubmitted, Marked, Moderated, Released };
LifecycleStage lifecycleOf(const Submission* submission, const Mark* mark);
std::string label(LifecycleStage s);

class AuditEntry {
public:
    AuditEntry(int id, std::string entityType, int entityId, std::optional<std::string> oldValue,
               std::string newValue, int actorId, DateTime timestamp, std::string reason)
        : id_(id), entityType_(std::move(entityType)), entityId_(entityId), oldValue_(std::move(oldValue)),
          newValue_(std::move(newValue)), actorId_(actorId), timestamp_(timestamp), reason_(std::move(reason)) {}
    int id() const { return id_; }
    const std::string& entityType() const { return entityType_; }
    int entityId() const { return entityId_; }
    const std::optional<std::string>& oldValue() const { return oldValue_; }
    const std::string& newValue() const { return newValue_; }
    int actorId() const { return actorId_; }
    DateTime timestamp() const { return timestamp_; }
    const std::string& reason() const { return reason_; }

private:
    int id_;
    std::string entityType_;
    int entityId_;
    std::optional<std::string> oldValue_;
    std::string newValue_;
    int actorId_;
    DateTime timestamp_;
    std::string reason_;
};

}  // namespace lms
