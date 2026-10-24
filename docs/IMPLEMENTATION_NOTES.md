# Implementation Notes (Phase 4)

## 1. Design-to-code map

| Phase 3 element | Where it is implemented |
|---|---|
| Layered architecture (Presentation / Service / Domain / Data Access) | `src/presentation`, `src/service`, `src/domain`, `src/data` |
| Abstract `User`, `Student`, `Lecturer`, `HoD`, `Admin` (polymorphism) | `src/domain/User.h/.cpp` - each role overrides `role()`, `allowedActions()`, `toRecord()` |
| Strategy: penalty rules (`IPenaltyStrategy`, `FixedPenaltyStrategy`, `LatePenaltyToZeroStrategy`) | `src/domain/PenaltyStrategy.h/.cpp`; default `DailyPercentPenaltyStrategy` (10 % per day, 3 days, then 0) |
| Repository (`IUserRepository`, `IMarkRepository`, `IAuditRepository`, ... split per aggregate) | `src/data/Repositories.h`; SQLite versions in `src/data/sqlite/` |
| Factory: `UserFactory` (validates, hashes, builds role objects) | `src/domain/UserFactory.h/.cpp` |
| Observer: `SubmissionObserver` -> audit + student notification | `src/service/MarkEvents.h`, `AuditService`, `StudentNotifier` |
| `Assignment.getEffectiveDeadline()` | `src/domain/Entities.h` |
| `Mark.applyPenalty()` | `src/domain/Entities.cpp` |
| Submission state machine | `Submission::createNew/resubmit`, `Mark::moderate/release`, `lifecycleOf()` in `Entities.cpp` |
| 14 tables, constraints, indexes | `schema/schema.sql` (identical to Phase 3 section 3.4.2), embedded at build time |
| Rules "checked outside the table definitions" (data dictionary) | subtype/role match: `UserFactory` + `UserRepository::add`; `raw_mark <= max_mark`: `MarkingService`; students see only RELEASED: query + `StudentPortalService`; audit append-only: `IAuditRepository` has no update/delete |

## 2. Decisions where the documents left room (please confirm, then record in the SRS)

1. **"10 % of the available mark per day"** - implemented as 10 % of the *raw mark* per day (so a mark of 20 is 18, 16, 14, then 0), because the Phase 3 contract is `apply(rawMark, daysLate)`. If the HoD meant 10 % of the *maximum* mark, change `DailyPercentPenaltyStrategy::apply` and add `maxMark` to the interface (one place, thanks to the Strategy pattern).
2. **Days late are rounded up**: 1 second late = 1 day late; exactly on the deadline = on time; 3 days + 1 second = 4 days = zero.
3. **More than 3 days late**: the work is still accepted and stored (student representative: "not rejected") but scores zero (HoD).
4. **Submission states vs the state diagram.** The approved schema keeps `submission.state` in {SUBMITTED, LATE, RESUBMITTED, MARKED} and `mark.state` in {PROVISIONAL, MODERATED, RELEASED}. The diagram's MODERATED and RELEASED states are therefore *derived* from the mark (`lifecycleOf`). The schema was not changed.
5. **Moderation** may approve a mark unchanged or adjust the *final* mark (reason mandatory, range 0..max). The lecturer's raw mark is never overwritten. A lecturer can re-mark only while the mark is PROVISIONAL (reason mandatory).
6. **Resubmission** is allowed only before the student's effective deadline and before marking; the row is updated in place (state RESUBMITTED), as in the data dictionary.
7. **Extensions**: 1-30 days, one per student per assignment (v1.0), reason mandatory, lecturer must teach the course and the student must be enrolled.
8. **Student assignment list shows the original deadline only** in v1.0. Lateness is still judged against the extended deadline, and the submit confirmation tells the student their real deadline. Showing extensions in the list is left for change request CR1 (Phase 8).
9. **Written feedback is mandatory** when marking (student representative's requirement; FR-09), although the column is nullable.
10. **Notifications** (`StudentNotifier`) are in memory for the running session: the approved schema has no notification table.
11. **Submission file** is stored as a reference (path); the file itself is not copied by the LMS. The console warns if the path does not exist.
12. Extra validation not in the SRS: username 3-32 chars `[A-Za-z0-9._-]`; password 8-128 chars and not equal to the username; deadline must be in the future when an assignment is created; the last administrator and your own account cannot be deleted.
13. Time: stored UTC; typed/shown in local time with a fixed offset (default +120 min, Lesotho has no daylight saving).

## 3. Inconsistencies found in the Phase 1-3 documents (worth fixing before the final report)

* Phase 2: FR-06 and FR-12 are identical (the brief needs at least 12 *distinct* functional requirements: e.g. split out "grant individual extension", "audit log of mark changes", "apply late penalty", "change own password"). Conflict row C6 has its columns copy-pasted and no real conflict text.
* Phase 2: there is no functional requirement for the **audit log**, **late penalty**, **extensions** or **release of marks** although the case study requires them; they are implemented, so add them to the SRS and traceability matrix.
* Phase 3 section 3.4.4: the mapping table names the audit table `AUDIT_LOG` (schema: `audit_entry`) and says "lecturer_id nullable" for Course (no such column). Section 3.4.2 is numbered twice (3.4.2 / 3.4.3).
* Phase 3 section 3.2 calls the strategy `LatePenaltyStrategy` and later `IPenaltyStrategy`; the code uses `IPenaltyStrategy`.

## 4. Draft traceability (functional requirements -> code -> tests)

| FR | Main code | Tests (tests/) |
|----|-----------|----------------|
| FR-01 login | `AuthService`, `SodiumPasswordHasher` | `Auth.*`, `SodiumHasher.*`, `Console.BadLoginAndGarbageInputDoNotCrash` |
| FR-02 role restriction | `User::allowedActions`, `requireAction`, role menus | `AccessControl.*`, `Console.StudentCannotReach...` |
| FR-03 manage accounts | `AccountService`, `UserFactory` | `Accounts.*`, `Console.FirstRunCreatesAdministrator` |
| FR-04 departments/courses | `CatalogService` | `Accounts.CannotDeleteUserWithRecords` (partial: add catalog CRUD tests in Phase 5) |
| FR-05 assign/enrol | `CatalogService`, `Sqlite{Teaching,Enrollment}Repository` | used by `World` fixture (add direct tests in Phase 5) |
| FR-06/12 create assignment | `AssignmentService::createAssignment` | `Assignment.CreationRules` |
| FR-07 student views | `StudentPortalService` | `Portal.*` |
| FR-08 submit/resubmit | `SubmissionService`, `Submission` | `Submit.*`, `SubmissionLifecycle.*`, `LateDays.*` |
| FR-09 mark + feedback | `MarkingService`, `Mark` | `Marking.*`, `Penalty.*` |
| FR-10 moderate/release | `ModerationService` | `Moderation.*` |
| FR-11 released marks only | `SqliteMarkRepository::listForStudent`, `StudentPortalService` | `Moderation.StudentsSeeNothingUntilReleased` |
| (extension) | `AssignmentService::grantExtension` | `Extension.Rules`, `Submit.Extension...` |
| (audit) | `AuditService`, `SqliteAuditRepository` | `Marking.RemarkNeedsReason...`, `Moderation.FailedAuditRollsTheWholeChangeBack` |
| NFR-01 performance | indexes, prepared statements | `Performance.*` (800 students, < 2 s) |
| NFR-02 offline | embedded SQLite, no network code | by construction (no socket code) |
| NFR-03 password security | Argon2id | `SodiumHasher.*` |
| NFR-04 access control | services | `AccessControl.*` |
| NFR-05 integrity/persistence | FKs on, transactions, WAL | `Database.*` |
| NFR-06 usability | role menus | (user test in Phase 5 acceptance testing) |

## 5. Measured results (this build)

* 43 automated tests, all passing; also clean under AddressSanitizer + UndefinedBehaviourSanitizer.
* Line coverage of business logic (`src/domain`, `src/service`, `src/common`): 86.8 %; whole `src/`: 71.1 %.
* `cppcheck` (warning, style, performance, portability): no findings.
* About 4,150 lines of source and 630 lines of tests.
