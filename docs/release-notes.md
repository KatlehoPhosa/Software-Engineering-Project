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
