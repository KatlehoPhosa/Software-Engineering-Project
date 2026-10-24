#pragma once
#include <string>

namespace lms {

enum class UserRole { Student, Lecturer, HoD, Admin };
enum class SubmissionState { Submitted, Late, Resubmitted, Marked };
enum class MarkState { Provisional, Moderated, Released };

/// Database representation (matches the CHECK constraints in schema.sql).
std::string toDbString(UserRole r);
std::string toDbString(SubmissionState s);
std::string toDbString(MarkState s);
UserRole userRoleFromDb(const std::string& s);
SubmissionState submissionStateFromDb(const std::string& s);
MarkState markStateFromDb(const std::string& s);

/// Human-readable label for menus and reports.
std::string label(UserRole r);
std::string label(SubmissionState s);
std::string label(MarkState s);

}  // namespace lms
