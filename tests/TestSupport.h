#pragma once
#include <gtest/gtest.h>

#include <memory>

#include "System.h"
#include "common/Exceptions.h"

namespace lms::test {

/// Fast stand-in for Argon2 so thousands of accounts can be created in milliseconds.
class FakeHasher final : public IPasswordHasher {
public:
    std::string hash(const std::string& p) const override { return "fake$" + p; }
    bool verify(const std::string& p, const std::string& h) const override { return h == "fake$" + p; }
};

/// A small school: Science dept (HoD), Humanities dept (HoD), one lecturer teaching SCI101,
/// two students enrolled in SCI101, one assignment due 2026-11-10 12:00 UTC (max 20).
/// "Now" starts at 2026-11-01 08:00 UTC and can be moved with clock.set().
struct World {
    data::Database db{":memory:"};
    FixedClock clock{DateTime::fromParts(2026, 11, 1, 8, 0, 0)};
    FakeHasher hasher;
    System sys{db, clock, hasher};

    std::unique_ptr<User> admin, hodSci, hodHum, lecturer, otherLecturer, alice, bob;
    int assignmentId = 0;
    static DateTime deadline() { return DateTime::fromParts(2026, 11, 10, 12, 0, 0); }

    World() {
        db.initialiseSchema();
        sys.accounts.createFirstAdministrator("admin", "Admin#1234");
        admin = sys.users.findByUsername("admin");
        const int sci = sys.catalog.createDepartment(*admin, "Science");
        const int hum = sys.catalog.createDepartment(*admin, "Humanities");
        hodSci = make(UserRole::HoD, "hod.sci", "", sci);
        hodHum = make(UserRole::HoD, "hod.hum", "", hum);
        lecturer = make(UserRole::Lecturer, "lect.a", "L1", 0);
        otherLecturer = make(UserRole::Lecturer, "lect.b", "L2", 0);
        alice = make(UserRole::Student, "alice", "S1", 0);
        bob = make(UserRole::Student, "bob", "S2", 0);
        sys.catalog.createCourse(*admin, "SCI101", "Science", sci);
        sys.catalog.createCourse(*admin, "HIS101", "History", hum);
        sys.catalog.assignLecturer(*admin, lecturer->id(), "SCI101");
        sys.catalog.enrolStudent(*admin, alice->id(), "SCI101");
        sys.catalog.enrolStudent(*admin, bob->id(), "SCI101");
        assignmentId = sys.assignmentService.createAssignment(*lecturer, "SCI101", "Lab 1", deadline(), 20.0);
    }

    std::unique_ptr<User> make(UserRole r, const std::string& name, const std::string& no, int dept) {
        NewUserSpec s;
        s.role = r; s.username = name; s.password = "Passw0rd!"; s.departmentId = dept;
        if (r == UserRole::Student) s.studentNo = no;
        if (r == UserRole::Lecturer) s.staffNo = no;
        return sys.users.findById(sys.accounts.createUser(*admin, s));
    }

    /// Alice submits at `when`, lecturer marks `raw`; returns the mark id.
    int submitAndMark(const User& student, DateTime when, double raw) {
        clock.set(when);
        auto r = sys.submissionService.submit(student, assignmentId, "work.pdf");
        return sys.marking.markSubmission(*lecturer, r.submissionId, raw, "feedback", "").markId;
    }
};

}  // namespace lms::test
