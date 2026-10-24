#include "domain/Enums.h"

#include "common/Exceptions.h"

namespace lms {

std::string toDbString(UserRole r) {
    switch (r) {
        case UserRole::Student: return "STUDENT";
        case UserRole::Lecturer: return "LECTURER";
        case UserRole::HoD: return "HOD";
        case UserRole::Admin: return "ADMIN";
    }
    throw DatabaseError("Unknown user role.");
}

std::string toDbString(SubmissionState s) {
    switch (s) {
        case SubmissionState::Submitted: return "SUBMITTED";
        case SubmissionState::Late: return "LATE";
        case SubmissionState::Resubmitted: return "RESUBMITTED";
        case SubmissionState::Marked: return "MARKED";
    }
    throw DatabaseError("Unknown submission state.");
}

std::string toDbString(MarkState s) {
    switch (s) {
        case MarkState::Provisional: return "PROVISIONAL";
        case MarkState::Moderated: return "MODERATED";
        case MarkState::Released: return "RELEASED";
    }
    throw DatabaseError("Unknown mark state.");
}

UserRole userRoleFromDb(const std::string& s) {
    if (s == "STUDENT") return UserRole::Student;
    if (s == "LECTURER") return UserRole::Lecturer;
    if (s == "HOD") return UserRole::HoD;
    if (s == "ADMIN") return UserRole::Admin;
    throw DatabaseError("Corrupt data: unknown role '" + s + "'.");
}

SubmissionState submissionStateFromDb(const std::string& s) {
    if (s == "SUBMITTED") return SubmissionState::Submitted;
    if (s == "LATE") return SubmissionState::Late;
    if (s == "RESUBMITTED") return SubmissionState::Resubmitted;
    if (s == "MARKED") return SubmissionState::Marked;
    throw DatabaseError("Corrupt data: unknown submission state '" + s + "'.");
}

MarkState markStateFromDb(const std::string& s) {
    if (s == "PROVISIONAL") return MarkState::Provisional;
    if (s == "MODERATED") return MarkState::Moderated;
    if (s == "RELEASED") return MarkState::Released;
    throw DatabaseError("Corrupt data: unknown mark state '" + s + "'.");
}

std::string label(UserRole r) {
    switch (r) {
        case UserRole::Student: return "Student";
        case UserRole::Lecturer: return "Lecturer";
        case UserRole::HoD: return "Head of Department";
        case UserRole::Admin: return "Administrator";
    }
    return "?";
}

std::string label(SubmissionState s) {
    switch (s) {
        case SubmissionState::Submitted: return "Submitted";
        case SubmissionState::Late: return "Late";
        case SubmissionState::Resubmitted: return "Resubmitted";
        case SubmissionState::Marked: return "Marked";
    }
    return "?";
}

std::string label(MarkState s) {
    switch (s) {
        case MarkState::Provisional: return "Awaiting moderation";
        case MarkState::Moderated: return "Moderated (not yet released)";
        case MarkState::Released: return "Released";
    }
    return "?";
}

}  // namespace lms
