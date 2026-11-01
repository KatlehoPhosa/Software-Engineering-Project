# Release Notes
**Moderated Learning Management System (LMS)**

## Version 1.0.0

**Release type:** Initial release
**Application:** Moderated Learning Management System
**Platform:** Windows and Linux
**Language:** C++17
**Database:** SQLite

## 1. Release Overview

Version 1.0.0 is the initial release of the Moderated Learning Management System, developed to support academic administration and assessment activities in a secondary school.

The application operates offline and provides role-based access for administrators, Heads of Department, lecturers and students.

## 2. Main Features

### User and Access Management
- User accounts for four roles: Administrator, Head of Department, Lecturer and Student.
- Role-based access control.
- Password hashing using libsodium and Argon2id.
- Password change and administrator password reset functionality.

### Academic Administration
- Department creation and management.
- Course creation and management.
- Lecturer assignment to courses.
- Student enrolment in courses.

### Assignment Management
- Assignment creation by lecturers.
- Assignment deadlines and maximum marks.
- Student assignment listing.
- Submission and resubmission support.
- Deadline extensions granted by lecturers.

### Assessment and Moderation
- Lecturer marking of submissions.
- Written feedback for students.
- Automatic late-submission penalty handling.
- HoD review and moderation of provisional marks.
- Individual or bulk release of moderated marks.
- Students can view only released marks.

### Audit and Security
- Audit history of mark changes.
- Recording of previous and updated mark values.
- Reasons for mark changes.
- Prepared SQL statements to protect database operations.
- Local SQLite database storage.

## 3. Technical Information

- C++17 application.
- CMake-based build configuration.
- SQLite database.
- libsodium Argon2id password hashing.
- GoogleTest automated test suite.
- Cppcheck static analysis.
- GitHub Actions continuous integration.

## 4. Testing and Quality Assurance

The project has been tested using its automated GoogleTest suite.

The Phase 6 test evidence recorded:
- 43 out of 43 GoogleTests passing.
- Successful CMake build.
- Successful CTest execution.
- Cppcheck analysis of 25 source files with zero remaining warnings after corrections.
- Successful GitHub Actions CI execution.

These results reflect the recorded Phase 6 verification; they do not guarantee that every possible deployment environment has been tested.

## 5. Deployment Information

The application can be built and run on supported Windows and Linux environments.

Refer to:
- `installation-guide.md` for installation and setup.
- `user-manuals.md` for role-specific instructions.

The LMS uses a local SQLite database and does not require an internet connection during normal operation.

## 6. Known Limitations

- The application uses a console-based interface rather than a graphical web interface.
- Users must have access to the computer or environment where the LMS is installed.
- Database backups and account administration must be handled by authorised school personnel.
- The application does not provide a central online server or remote access service.

## 7. Upgrade and Installation Notes

For a fresh installation, follow the installation guide.

For an existing installation:
- Back up the SQLite database before replacing application files.
- Ensure the required compiler/runtime dependencies are available.
- Build the application using the supplied CMake configuration.
- Verify that the application starts and can access its database.

## 8. Release Identification

**Version:** 1.0.0
**Release tag:** `v1.0` (required submission tag)
**Project repository:** https://github.com/KatlehoPhosa/Software-Engineering-Project

---

**End of Release Notes**

## Version 1.1.0

**Release type:** Perfective maintenance release
**Change request:** CR1 – Class-wide assignment deadline extensions

### 1. Release Overview

Version 1.1.0 enhances deadline-extension functionality. Lecturers can now grant an extension to an individual student or to an entire class.

### 2. Changes and Improvements

#### Class-wide Deadline Extensions
- Lecturers can choose between an individual extension and a whole-class extension.
- A whole-class extension applies to all students currently enrolled in the assignment's course.
- The system creates an extension record for each enrolled student.
- The operation is all-or-nothing: it is rejected if the class has no enrolled students or if any student already has an extension for that assignment.
- Existing extension rules, including the 1–30 day limit and mandatory reason, remain in force.

#### Student Assignment List
- Students see their effective submission deadline.
- When an extension applies, the assignment list also displays the number of extra days and the original deadline.
- Submission and resubmission continue to use the effective deadline.

### 3. Database and Compatibility

- No database schema migration is required.
- The existing extension records and constraints are reused.
- Existing individual extension functionality is retained.

### 4. Regression Testing

The Phase 8 regression test run recorded:
- 45 out of 45 GoogleTests passing.
- 0 test failures.
- Successful CMake build.
- Successful CTest execution.
- Two new tests covering class-wide extension behaviour and rejection when an existing individual extension is present.

### 5. Upgrade Notes

Back up the existing SQLite database before replacing application files. Build the v1.1 application using the installation guide. No database migration is required.

### 6. Release Identification

**Version:** 1.1.0
**Release tag:** `v1.1`
**Change request:** CR1 – Class-wide assignment deadline extensions

---

**End of Version 1.1.0 Release Notes**
