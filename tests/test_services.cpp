#include "TestSupport.h"

using namespace lms;
using lms::test::World;

// ======================================================================= submission (FR-08, C2, C3, C4)
TEST(Submit, OnTimeThenResubmitBeforeDeadline) {
    World w;
    auto r1 = w.sys.submissionService.submit(*w.alice, w.assignmentId, "v1.pdf");
    EXPECT_EQ(r1.state, SubmissionState::Submitted);
    EXPECT_FALSE(r1.wasResubmission);
    w.clock.set(World::deadline().addSeconds(-1));
    auto r2 = w.sys.submissionService.submit(*w.alice, w.assignmentId, "v2.pdf");
    EXPECT_TRUE(r2.wasResubmission);
    EXPECT_EQ(r2.state, SubmissionState::Resubmitted);
    EXPECT_EQ(r1.submissionId, r2.submissionId);                         // one row, latest file is active
    EXPECT_EQ(w.sys.submissions.findById(r1.submissionId)->fileRef(), "v2.pdf");
}
TEST(Submit, LateWorkIsAcceptedNotRejected) {
    World w;
    w.clock.set(World::deadline().addDays(2).addSeconds(-10));
    auto r = w.sys.submissionService.submit(*w.alice, w.assignmentId, "late.pdf");
    EXPECT_EQ(r.state, SubmissionState::Late);
    EXPECT_EQ(r.daysLate, 2);
    w.clock.set(World::deadline().addDays(10));                          // even far too late is stored
    EXPECT_EQ(w.sys.submissionService.submit(*w.bob, w.assignmentId, "x.pdf").state, SubmissionState::Late);
}
TEST(Submit, ResubmissionAfterDeadlineRefused) {
    World w;
    w.sys.submissionService.submit(*w.alice, w.assignmentId, "v1.pdf");
    w.clock.set(World::deadline().addSeconds(1));
    EXPECT_THROW(w.sys.submissionService.submit(*w.alice, w.assignmentId, "v2.pdf"), BusinessRuleError);
    EXPECT_EQ(w.sys.submissions.find(w.alice->id(), w.assignmentId)->fileRef(), "v1.pdf");
}
TEST(Submit, ExtensionMovesTheDeadlineForThatStudentOnly) {
    World w;
    w.sys.assignmentService.grantExtension(*w.lecturer, w.assignmentId, w.alice->id(), 2, "family emergency");
    w.clock.set(World::deadline().addDays(1));
    EXPECT_EQ(w.sys.submissionService.submit(*w.alice, w.assignmentId, "a.pdf").state, SubmissionState::Submitted);
    EXPECT_EQ(w.sys.submissionService.submit(*w.bob, w.assignmentId, "b.pdf").state, SubmissionState::Late);
}
TEST(Submit, ValidationAndOwnership) {
    World w;
    EXPECT_THROW(w.sys.submissionService.submit(*w.alice, w.assignmentId, "   "), ValidationError);
    EXPECT_THROW(w.sys.submissionService.submit(*w.alice, 9999, "a.pdf"), NotFoundError);
    auto outsider = w.make(UserRole::Student, "carol", "S3", 0);        // not enrolled
    EXPECT_THROW(w.sys.submissionService.submit(*outsider, w.assignmentId, "a.pdf"), AuthorizationError);
    EXPECT_THROW(w.sys.submissionService.submit(*w.lecturer, w.assignmentId, "a.pdf"), AuthorizationError);
}

// ======================================================================= extensions
TEST(Extension, Rules) {
    World w;
    auto& s = w.sys.assignmentService;
    EXPECT_THROW(s.grantExtension(*w.lecturer, w.assignmentId, w.alice->id(), 0, "r"), ValidationError);
    EXPECT_THROW(s.grantExtension(*w.lecturer, w.assignmentId, w.alice->id(), 31, "r"), ValidationError);
    EXPECT_THROW(s.grantExtension(*w.lecturer, w.assignmentId, w.alice->id(), 2, "  "), ValidationError);
    EXPECT_THROW(s.grantExtension(*w.otherLecturer, w.assignmentId, w.alice->id(), 2, "r"), AuthorizationError);
    auto outsider = w.make(UserRole::Student, "carol", "S3", 0);
    EXPECT_THROW(s.grantExtension(*w.lecturer, w.assignmentId, outsider->id(), 2, "r"), BusinessRuleError);
    EXPECT_NO_THROW(s.grantExtension(*w.lecturer, w.assignmentId, w.alice->id(), 2, "illness"));
    EXPECT_THROW(s.grantExtension(*w.lecturer, w.assignmentId, w.alice->id(), 3, "again"), BusinessRuleError);
}
TEST(Extension, ClassWideAppliesToEveryEnrolledStudentAndAppearsInPortal) {
    World w;
    EXPECT_GT(w.sys.assignmentService.grantClassExtension(*w.lecturer, w.assignmentId, 3, "class circumstances"), 0);

    for (const User* student : {w.alice.get(), w.bob.get()}) {
        auto rows = w.sys.portal.myAssignments(*student);
        ASSERT_EQ(rows.size(), 1u);
        EXPECT_EQ(rows[0].extraDays, 3);
        EXPECT_GT(rows[0].effectiveDeadline(), rows[0].assignment.deadline());
    }

    w.clock.set(World::deadline().addDays(1));
    EXPECT_EQ(w.sys.submissionService.submit(*w.alice, w.assignmentId, "a.pdf").state, SubmissionState::Submitted);
    EXPECT_EQ(w.sys.submissionService.submit(*w.bob, w.assignmentId, "b.pdf").state, SubmissionState::Submitted);
}

TEST(Extension, ClassWideRequestIsRejectedIfAnyStudentAlreadyHasAnExtension) {
    World w;
    w.sys.assignmentService.grantExtension(*w.lecturer, w.assignmentId, w.alice->id(), 2, "individual request");
    EXPECT_THROW(w.sys.assignmentService.grantClassExtension(*w.lecturer, w.assignmentId, 3, "class circumstances"), BusinessRuleError);

    w.clock.set(World::deadline().addDays(1));
    EXPECT_EQ(w.sys.submissionService.submit(*w.bob, w.assignmentId, "b.pdf").state, SubmissionState::Late);
}

TEST(Assignment, CreationRules) {
    World w;
    auto& s = w.sys.assignmentService;
    const DateTime future = w.clock.now().addDays(3);
    EXPECT_THROW(s.createAssignment(*w.lecturer, "SCI101", "  ", future, 10), ValidationError);
    EXPECT_THROW(s.createAssignment(*w.lecturer, "SCI101", "T", future, 0), ValidationError);
    EXPECT_THROW(s.createAssignment(*w.lecturer, "SCI101", "T", w.clock.now(), 10), ValidationError);  // not in the future
    EXPECT_THROW(s.createAssignment(*w.otherLecturer, "SCI101", "T", future, 10), AuthorizationError);
    EXPECT_THROW(s.createAssignment(*w.lecturer, "NOPE1", "T", future, 10), NotFoundError);
    EXPECT_GT(s.createAssignment(*w.lecturer, "SCI101", "T", future, 10), 0);
}

// ======================================================================= marking (FR-09)
TEST(Marking, AppliesLatePenaltyUsingEffectiveDeadline) {
    World w;
    int markId = w.submitAndMark(*w.alice, World::deadline().addDays(2).addSeconds(-60), 20);  // 2 days late
    EXPECT_DOUBLE_EQ(*w.sys.marks.findById(markId)->finalMark(), 16.0);
    EXPECT_EQ(w.sys.submissions.find(w.alice->id(), w.assignmentId)->state(), SubmissionState::Marked);

    w.sys.assignmentService.grantExtension(*w.lecturer, w.assignmentId, w.bob->id(), 2, "illness");
    int bobMark = w.submitAndMark(*w.bob, World::deadline().addDays(2).addSeconds(-60), 20);   // on time for Bob
    EXPECT_DOUBLE_EQ(*w.sys.marks.findById(bobMark)->finalMark(), 20.0);
}
TEST(Marking, ThreeDaysLateKeepsSeventyPercentFourGivesZero) {
    World w;
    EXPECT_DOUBLE_EQ(*w.sys.marks.findById(w.submitAndMark(*w.alice, World::deadline().addDays(3), 20))->finalMark(), 14.0);
    EXPECT_DOUBLE_EQ(*w.sys.marks.findById(w.submitAndMark(*w.bob, World::deadline().addDays(3).addSeconds(1), 20))->finalMark(), 0.0);
}
TEST(Marking, InputAndOwnershipChecks) {
    World w;
    auto sub = w.sys.submissionService.submit(*w.alice, w.assignmentId, "a.pdf");
    auto& m = w.sys.marking;
    EXPECT_THROW(m.markSubmission(*w.lecturer, sub.submissionId, 21, "fb", ""), ValidationError);   // above max
    EXPECT_THROW(m.markSubmission(*w.lecturer, sub.submissionId, -1, "fb", ""), ValidationError);
    EXPECT_THROW(m.markSubmission(*w.lecturer, sub.submissionId, 10, "  ", ""), ValidationError);   // feedback required
    EXPECT_THROW(m.markSubmission(*w.otherLecturer, sub.submissionId, 10, "fb", ""), AuthorizationError);
    EXPECT_THROW(m.markSubmission(*w.lecturer, 12345, 10, "fb", ""), NotFoundError);
    EXPECT_THROW(m.markSubmission(*w.alice, sub.submissionId, 10, "fb", ""), AuthorizationError);
    EXPECT_NO_THROW(m.markSubmission(*w.lecturer, sub.submissionId, 20, "fb", ""));                 // max is allowed
    EXPECT_THROW(w.sys.submissionService.submit(*w.alice, w.assignmentId, "again.pdf"), BusinessRuleError);  // marked
}
TEST(Marking, RemarkNeedsReasonAndIsAudited) {
    World w;
    auto sub = w.sys.submissionService.submit(*w.alice, w.assignmentId, "a.pdf");
    auto first = w.sys.marking.markSubmission(*w.lecturer, sub.submissionId, 10, "fb", "");
    EXPECT_THROW(w.sys.marking.markSubmission(*w.lecturer, sub.submissionId, 12, "fb2", ""), ValidationError);
    w.clock.advanceSeconds(60);
    w.sys.marking.markSubmission(*w.lecturer, sub.submissionId, 12, "fb2", "added missed page");
    auto log = w.sys.auditRepo.listForEntity("Mark", first.markId);
    ASSERT_EQ(log.size(), 2u);
    EXPECT_FALSE(log[0].oldValue().has_value());
    EXPECT_EQ(log[0].reason(), "Initial marking");
    EXPECT_EQ(*log[1].oldValue(), "raw=10.00;final=10.00;state=PROVISIONAL");
    EXPECT_EQ(log[1].newValue(), "raw=12.00;final=12.00;state=PROVISIONAL");
    EXPECT_EQ(log[1].actorId(), w.lecturer->id());
    EXPECT_EQ(log[1].reason(), "added missed page");
    EXPECT_GT(log[1].timestamp(), log[0].timestamp());
}

// ======================================================================= moderation (FR-10, FR-11, C1)
TEST(Moderation, StudentsSeeNothingUntilReleased) {
    World w;
    int mid = w.submitAndMark(*w.alice, w.clock.now(), 15);
    EXPECT_TRUE(w.sys.portal.myReleasedMarks(*w.alice).empty());          // provisional
    w.sys.moderation.moderate(*w.hodSci, mid, std::nullopt, "");
    EXPECT_TRUE(w.sys.portal.myReleasedMarks(*w.alice).empty());          // moderated but not released
    w.sys.moderation.release(*w.hodSci, mid);
    auto rows = w.sys.portal.myReleasedMarks(*w.alice);
    ASSERT_EQ(rows.size(), 1u);
    EXPECT_DOUBLE_EQ(*rows[0].finalMark, 15.0);
    EXPECT_EQ(*rows[0].feedback, "feedback");
    EXPECT_TRUE(w.sys.portal.myReleasedMarks(*w.bob).empty());            // other students never see it
    EXPECT_EQ(w.sys.notifier.drain(w.alice->id()).size(), 1u);            // observer fired
    EXPECT_TRUE(w.sys.notifier.drain(w.alice->id()).empty());
}
TEST(Moderation, HodOnlyForOwnDepartment) {
    World w;
    int mid = w.submitAndMark(*w.alice, w.clock.now(), 15);
    EXPECT_THROW(w.sys.moderation.moderate(*w.hodHum, mid, std::nullopt, ""), AuthorizationError);
    EXPECT_THROW(w.sys.moderation.release(*w.hodHum, mid), AuthorizationError);
    EXPECT_TRUE(w.sys.moderation.reviewMarks(*w.hodHum).empty());
    EXPECT_EQ(w.sys.moderation.reviewMarks(*w.hodSci).size(), 1u);
    EXPECT_THROW(w.sys.audit.historyOfMark(*w.hodHum, mid), AuthorizationError);
    EXPECT_EQ(w.sys.audit.historyOfMark(*w.hodSci, mid).size(), 1u);
}
TEST(Moderation, AdjustmentNeedsReasonAndRangeAndIsAudited) {
    World w;
    int mid = w.submitAndMark(*w.alice, w.clock.now(), 15);
    EXPECT_THROW(w.sys.moderation.moderate(*w.hodSci, mid, 17.0, ""), ValidationError);
    EXPECT_THROW(w.sys.moderation.moderate(*w.hodSci, mid, 21.0, "why"), ValidationError);
    EXPECT_THROW(w.sys.moderation.moderate(*w.hodSci, mid, -1.0, "why"), ValidationError);
    w.sys.moderation.moderate(*w.hodSci, mid, 17.0, "consistent with rubric");
    auto m = *w.sys.marks.findById(mid);
    EXPECT_DOUBLE_EQ(*m.finalMark(), 17.0);
    EXPECT_DOUBLE_EQ(m.rawMark(), 15.0);                                  // lecturer's raw mark preserved
    auto log = w.sys.auditRepo.listForEntity("Mark", mid);
    ASSERT_EQ(log.size(), 2u);
    EXPECT_EQ(log[1].actorId(), w.hodSci->id());
    EXPECT_EQ(log[1].newValue(), "raw=15.00;final=17.00;state=MODERATED");
    EXPECT_THROW(w.sys.marking.markSubmission(*w.lecturer, m.submissionId(), 5, "fb", "change"), BusinessRuleError);
}
TEST(Moderation, ReleaseOrderAndBulkRelease) {
    World w;
    int a = w.submitAndMark(*w.alice, w.clock.now(), 15);
    int b = w.submitAndMark(*w.bob, w.clock.now(), 12);
    EXPECT_THROW(w.sys.moderation.release(*w.hodSci, a), BusinessRuleError);     // not moderated yet
    w.sys.moderation.moderate(*w.hodSci, a, std::nullopt, "");
    w.sys.moderation.moderate(*w.hodSci, b, std::nullopt, "");
    EXPECT_THROW(w.sys.moderation.moderate(*w.hodSci, a, std::nullopt, ""), BusinessRuleError);
    EXPECT_EQ(w.sys.moderation.releaseAllModerated(*w.hodSci), 2);
    EXPECT_EQ(w.sys.moderation.releaseAllModerated(*w.hodSci), 0);
    EXPECT_EQ(w.sys.auditRepo.listForEntity("Mark", a).size(), 3u);               // marked, moderated, released
}
TEST(Moderation, FailedAuditRollsTheWholeChangeBack) {  // atomicity: no mark change without a log entry
    struct Boom final : SubmissionObserver { void onMarkEvent(const MarkEvent&) override { throw DatabaseError("disk full"); } };
    World w;
    Boom boom;
    w.sys.events.attach(&boom);
    auto sub = w.sys.submissionService.submit(*w.alice, w.assignmentId, "a.pdf");
    EXPECT_THROW(w.sys.marking.markSubmission(*w.lecturer, sub.submissionId, 10, "fb", ""), DatabaseError);
    EXPECT_FALSE(w.sys.marks.findBySubmissionId(sub.submissionId).has_value());
    EXPECT_NE(w.sys.submissions.findById(sub.submissionId)->state(), SubmissionState::Marked);
    EXPECT_TRUE(w.sys.auditRepo.listRecent(10).empty());
}

// ======================================================================= student views
TEST(Portal, StudentSeesOnlyOwnCoursesAndAssignments) {
    World w;
    auto carol = w.make(UserRole::Student, "carol", "S3", 0);
    EXPECT_TRUE(w.sys.portal.myCourses(*carol).empty());
    EXPECT_TRUE(w.sys.portal.myAssignments(*carol).empty());
    auto list = w.sys.portal.myAssignments(*w.alice);
    ASSERT_EQ(list.size(), 1u);
    EXPECT_EQ(list[0].stage, LifecycleStage::NotSubmitted);
    w.sys.submissionService.submit(*w.alice, w.assignmentId, "a.pdf");
    EXPECT_EQ(w.sys.portal.myAssignments(*w.alice)[0].stage, LifecycleStage::Submitted);
}

// ======================================================================= access control (FR-02, NFR-04)
TEST(AccessControl, EveryRoleIsBlockedFromOtherRolesFunctions) {
    World w;
    NewUserSpec spec; spec.role = UserRole::Student; spec.username = "zed"; spec.password = "Passw0rd!"; spec.studentNo = "S9";
    const DateTime future = w.clock.now().addDays(5);
    struct Case { const char* who; const User* u; };
    for (const User* u : {w.alice.get(), w.lecturer.get(), w.hodSci.get()}) {
        EXPECT_THROW(w.sys.accounts.createUser(*u, spec), AuthorizationError) << u->username();
        EXPECT_THROW(w.sys.catalog.createDepartment(*u, "X"), AuthorizationError);
        EXPECT_THROW(w.sys.catalog.createCourse(*u, "ABC1", "t", 1), AuthorizationError);
        EXPECT_THROW(w.sys.catalog.enrolStudent(*u, w.alice->id(), "SCI101"), AuthorizationError);
        EXPECT_THROW(w.sys.catalog.assignLecturer(*u, w.lecturer->id(), "SCI101"), AuthorizationError);
        EXPECT_THROW(w.sys.audit.recent(*u, 10), AuthorizationError);
    }
    for (const User* u : {w.admin.get(), w.alice.get(), w.hodSci.get()}) {
        EXPECT_THROW(w.sys.assignmentService.createAssignment(*u, "SCI101", "T", future, 5), AuthorizationError);
        EXPECT_THROW(w.sys.assignmentService.grantExtension(*u, w.assignmentId, w.alice->id(), 1, "r"), AuthorizationError);
        EXPECT_THROW(w.sys.marking.markSubmission(*u, 1, 5, "fb", ""), AuthorizationError);
    }
    for (const User* u : {w.admin.get(), w.alice.get(), w.lecturer.get()}) {
        EXPECT_THROW(w.sys.moderation.reviewMarks(*u), AuthorizationError);
        EXPECT_THROW(w.sys.moderation.moderate(*u, 1, std::nullopt, ""), AuthorizationError);
        EXPECT_THROW(w.sys.moderation.release(*u, 1), AuthorizationError);
        EXPECT_THROW(w.sys.moderation.releaseAllModerated(*u), AuthorizationError);
    }
    for (const User* u : {w.admin.get(), w.lecturer.get(), w.hodSci.get()}) {
        EXPECT_THROW(w.sys.submissionService.submit(*u, w.assignmentId, "f"), AuthorizationError);
        EXPECT_THROW(w.sys.portal.myCourses(*u), AuthorizationError);
        EXPECT_THROW(w.sys.portal.myAssignments(*u), AuthorizationError);
        EXPECT_THROW(w.sys.portal.myReleasedMarks(*u), AuthorizationError);
    }
}
TEST(AccessControl, AllowedActionsComeFromTheRoleClass) {
    World w;
    EXPECT_TRUE(w.alice->can(Action::SubmitAssignment));
    EXPECT_FALSE(w.alice->can(Action::ModerateMark));
    EXPECT_TRUE(w.hodSci->can(Action::ReleaseMarks));
    EXPECT_FALSE(w.lecturer->can(Action::ReleaseMarks));
    EXPECT_TRUE(w.admin->can(Action::ManageUsers));
    for (const User* u : {w.admin.get(), w.alice.get(), w.lecturer.get(), w.hodSci.get()}) EXPECT_TRUE(u->can(Action::ChangeOwnPassword));
}

// ======================================================================= accounts & auth (FR-01, FR-03, NFR-03)
TEST(Auth, LoginAndGenericFailure) {
    World w;
    EXPECT_EQ(w.sys.auth.login("alice", "Passw0rd!")->id(), w.alice->id());
    EXPECT_EQ(w.sys.auth.login("ALICE", "Passw0rd!")->id(), w.alice->id());   // usernames are case-insensitive
    for (auto [u, p] : std::vector<std::pair<std::string, std::string>>{{"alice", "wrong"}, {"nobody", "Passw0rd!"}, {"", "x"}, {"alice", ""}}) {
        try { w.sys.auth.login(u, p); FAIL(); }
        catch (const AuthenticationError& e) { EXPECT_STREQ(e.what(), "Invalid username or password."); }
    }
}
TEST(Auth, SqlInjectionAttemptsAreInert) {
    World w;
    EXPECT_THROW(w.sys.auth.login("' OR '1'='1", "x"), AuthenticationError);
    EXPECT_THROW(w.sys.auth.login("alice'--", "x"), AuthenticationError);
    const int id = w.sys.catalog.createDepartment(*w.admin, "Art'); DROP TABLE user;--");
    EXPECT_EQ(w.sys.departments.findById(id)->name(), "Art'); DROP TABLE user;--");
    EXPECT_EQ(w.sys.users.count(), 7);                                          // table intact
}
TEST(Auth, ChangePassword) {
    World w;
    EXPECT_THROW(w.sys.auth.changeOwnPassword(*w.alice, "bad", "NewPassw0rd"), AuthenticationError);
    EXPECT_THROW(w.sys.auth.changeOwnPassword(*w.alice, "Passw0rd!", "short"), ValidationError);
    w.sys.auth.changeOwnPassword(*w.alice, "Passw0rd!", "NewPassw0rd");
    EXPECT_THROW(w.sys.auth.login("alice", "Passw0rd!"), AuthenticationError);
    EXPECT_NO_THROW(w.sys.auth.login("alice", "NewPassw0rd"));
}
TEST(Accounts, RulesAndFriendlyErrors) {
    World w;
    NewUserSpec s; s.role = UserRole::Student; s.username = "Alice"; s.password = "Passw0rd!"; s.studentNo = "S77";
    EXPECT_THROW(w.sys.accounts.createUser(*w.admin, s), BusinessRuleError);   // duplicate username (case-insensitive)
    s.username = "dave"; s.studentNo = "S1";                                      // duplicate student number
    EXPECT_THROW(w.sys.accounts.createUser(*w.admin, s), BusinessRuleError);
    s.studentNo = " ";
    EXPECT_THROW(w.sys.accounts.createUser(*w.admin, s), ValidationError);
    NewUserSpec h; h.role = UserRole::HoD; h.username = "hod3"; h.password = "Passw0rd!"; h.departmentId = 1;
    EXPECT_THROW(w.sys.accounts.createUser(*w.admin, h), BusinessRuleError);   // Science already has a head
    h.departmentId = 999;
    EXPECT_THROW(w.sys.accounts.createUser(*w.admin, h), NotFoundError);
    EXPECT_THROW(w.sys.accounts.createFirstAdministrator("root", "Passw0rd!"), AuthorizationError);
    EXPECT_THROW(w.sys.accounts.deleteUser(*w.admin, w.admin->id()), BusinessRuleError);
    EXPECT_NO_THROW(w.sys.accounts.deleteUser(*w.admin, w.otherLecturer->id()));
}
TEST(Accounts, CannotDeleteUserWithRecords) {
    World w;
    w.submitAndMark(*w.alice, w.clock.now(), 10);
    EXPECT_THROW(w.sys.accounts.deleteUser(*w.admin, w.alice->id()), BusinessRuleError);
    EXPECT_THROW(w.sys.catalog.deleteCourse(*w.admin, "SCI101"), BusinessRuleError);
    EXPECT_THROW(w.sys.catalog.deleteDepartment(*w.admin, 1), BusinessRuleError);
}
