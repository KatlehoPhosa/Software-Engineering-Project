#include <chrono>
#include <cstdio>
#include <sstream>

#include "TestSupport.h"
#include "presentation/Application.h"
#include "security/SodiumPasswordHasher.h"

using namespace lms;
using lms::test::World;

// ======================================================================= security: real Argon2id
TEST(SodiumHasher, SaltedArgon2idHashes) {
    SodiumPasswordHasher h;
    const std::string a = h.hash("Correct#Horse1"), b = h.hash("Correct#Horse1");
    EXPECT_NE(a, b);                                         // random salt each time
    EXPECT_EQ(a.rfind("$argon2id$", 0), 0u);                 // algorithm + parameters + salt are in the string
    EXPECT_EQ(a.find("Correct#Horse1"), std::string::npos);  // no plain text
    EXPECT_TRUE(h.verify("Correct#Horse1", a));
    EXPECT_FALSE(h.verify("correct#horse1", a));
    EXPECT_FALSE(h.verify("x", "not-a-hash"));
}

TEST(SodiumHasher, StoredPasswordColumnHoldsOnlyTheHash) {   // NFR-03 end to end through the real stack
    data::Database db(":memory:"); db.initialiseSchema();
    FixedClock clock(DateTime::fromParts(2026, 11, 1));
    SodiumPasswordHasher real;
    System sys(db, clock, real);
    sys.accounts.createFirstAdministrator("admin", "Admin#1234");
    data::Statement st(db, "SELECT password_hash FROM \"user\" WHERE username = ?1", "admin");
    ASSERT_TRUE(st.step());
    const std::string stored = st.getText(0);
    EXPECT_EQ(stored.rfind("$argon2id$", 0), 0u);
    EXPECT_EQ(stored.find("Admin#1234"), std::string::npos);
    EXPECT_NE(sys.auth.login("admin", "Admin#1234"), nullptr);
}

// ======================================================================= data integrity (NFR-05)
TEST(Database, ConstraintsAreEnforced) {
    World w;
    EXPECT_THROW(data::Statement(w.db, "INSERT INTO assignment(title,deadline,max_mark,course_code,created_by) VALUES('x','2026-01-01 00:00:00',0,'SCI101',1)").run(), ConstraintViolation);
    EXPECT_THROW(data::Statement(w.db, "INSERT INTO enrollment(student_id,course_code) VALUES(9999,'SCI101')").run(), ConstraintViolation);   // FK on
    EXPECT_THROW(data::Statement(w.db, "INSERT INTO mark(submission_id,raw_mark,state) VALUES(1,-1,'PROVISIONAL')").run(), ConstraintViolation);
    EXPECT_THROW(data::Statement(w.db, "INSERT INTO audit_entry(entity_type,entity_id,new_value,actor_id,reason) VALUES('Mark',1,'x',1,'   ')").run(), ConstraintViolation);
    EXPECT_THROW(data::Statement(w.db, "INSERT INTO \"user\"(username,password_hash,role) VALUES('x','h','ROOT')").run(), ConstraintViolation);
}
TEST(Database, CommittedDataSurvivesRestart) {
    const std::string path = "persist_test.db";
    std::remove(path.c_str());
    {
        data::Database db(path);
        db.initialiseSchema();
        FixedClock clock(DateTime::fromParts(2026, 11, 1));
        test::FakeHasher hasher;
        System sys(db, clock, hasher);
        sys.accounts.createFirstAdministrator("admin", "Admin#1234");
        auto admin = sys.users.findByUsername("admin");
        sys.catalog.createDepartment(*admin, "Science");
    }
    {
        data::Database db(path);
        db.initialiseSchema();  // idempotent: must not wipe data
        FixedClock clock(DateTime::fromParts(2026, 11, 2));
        test::FakeHasher hasher;
        System sys(db, clock, hasher);
        EXPECT_EQ(sys.users.count(), 1);
        ASSERT_EQ(sys.departments.list().size(), 1u);
        EXPECT_EQ(sys.departments.list()[0].name(), "Science");
    }
    for (const char* ext : {"", "-wal", "-shm"}) std::remove((path + ext).c_str());
}
TEST(Database, RolledBackTransactionLeavesNoTrace) {
    World w;
    {
        auto tx = w.sys.tx.begin();
        w.sys.departments.add("Temp");
    }  // no commit
    for (const auto& d : w.sys.departments.list()) EXPECT_NE(d.name(), "Temp");
    auto outer = w.sys.tx.begin();
    EXPECT_THROW(w.sys.tx.begin(), DatabaseError);   // nested transactions are refused
}

// ======================================================================= performance (NFR-01)
TEST(Performance, CommonOperationsUnderTwoSecondsWith800Students) {
    World w;
    const std::string hash = SodiumPasswordHasher().hash("Passw0rd!");   // one real Argon2id hash reused for seeding
    {
        auto tx = w.sys.tx.begin();
        for (int i = 0; i < 800; ++i) {
            NewUserSpec s; s.role = UserRole::Student; s.username = "student" + std::to_string(i);
            s.password = "Passw0rd!"; s.studentNo = "N" + std::to_string(1000 + i);
            UserRecord r; r.username = s.username; r.passwordHash = hash; r.role = UserRole::Student; r.studentNo = s.studentNo;
            const int id = w.sys.users.add(*UserFactory::hydrate(r));
            w.sys.enrollments.enrol(id, "SCI101");
        }
        tx->commit();
    }
    SodiumPasswordHasher real;
    AuthService auth(w.sys.users, real);
    auto time = [](auto&& fn) {
        auto t0 = std::chrono::steady_clock::now();
        fn();
        return std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
    };
    EXPECT_LT(time([&] { EXPECT_NO_THROW(auth.login("student799", "Passw0rd!")); }), 2.0);
    auto s = w.sys.users.findByUsername("student400");
    EXPECT_LT(time([&] { w.sys.submissionService.submit(*s, w.assignmentId, "f.pdf"); }), 2.0);
    EXPECT_LT(time([&] { EXPECT_EQ(w.sys.marking.submissionsFor(*w.lecturer, w.assignmentId).size(), 1u); }), 2.0);
    EXPECT_LT(time([&] { EXPECT_EQ(w.sys.assignmentService.studentsOfCourse(*w.lecturer, "SCI101").size(), 802u); }), 2.0);
    EXPECT_LT(time([&] { w.sys.accounts.listUsers(*w.admin); }), 2.0);
}

// ======================================================================= console (robustness)
namespace {
std::string runApp(World& w, const std::string& input) {
    std::istringstream in(input);
    std::ostringstream out;
    Console con(in, out, 120, false);
    Application app(w.sys, con);
    app.run();
    return out.str();
}
}  // namespace

TEST(Console, BadLoginAndGarbageInputDoNotCrash) {
    World w;
    const std::string out = runApp(w, "abc\n7\n1\nalice\nwrong\n1\nalice\nPassw0rd!\n99\nxyz\n2\n0\n0\n");
    EXPECT_NE(out.find("Invalid username or password."), std::string::npos);
    EXPECT_NE(out.find("Welcome, alice (Student)."), std::string::npos);
    EXPECT_NE(out.find("Goodbye."), std::string::npos);
}
TEST(Console, EndOfInputExitsCleanly) {
    World w;
    EXPECT_NO_THROW(runApp(w, "1\nalice\nPassw0rd!\n"));   // input ends inside the student menu
}
TEST(Console, StudentCannotReachOtherRolesFunctions) {
    World w;
    const std::string out = runApp(w, "1\nalice\nPassw0rd!\n0\n0\n");
    EXPECT_EQ(out.find("Moderate a mark"), std::string::npos);
    EXPECT_EQ(out.find("Manage user accounts"), std::string::npos);
    EXPECT_NE(out.find("Submit or resubmit an assignment"), std::string::npos);
}
TEST(Console, FirstRunCreatesAdministrator) {
    data::Database db(":memory:"); db.initialiseSchema();
    FixedClock clock(DateTime::fromParts(2026, 11, 1)); test::FakeHasher h; System sys(db, clock, h);
    std::istringstream in("root\nshort\nshort\nroot\nStrongPass1\nStrongPass1\n0\n");
    std::ostringstream out; Console con(in, out, 120, false);
    Application(sys, con).run();
    EXPECT_EQ(sys.users.count(), 1);
    EXPECT_NE(out.str().find("Password must be at least 8 characters."), std::string::npos);
}
