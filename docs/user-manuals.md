# LMS User Manuals
**Moderated Learning Management System – Version 1.0.0**

## 1. Introduction

This document explains how each type of user operates the Moderated Learning Management System (LMS).

The system supports four user roles:

1. Administrator
2. Head of Department (HoD)
3. Lecturer
4. Student

Each user receives a username, password and role-specific menu. Users can only access functions permitted by their assigned role.

## 2. Getting Started

### 2.1 Starting the application

Open a terminal in the LMS installation directory.

Windows:

```bash
./build/lms.exe
```

Linux:

```bash
./build/lms
```

### 2.2 Logging in

1. Start the LMS application.
2. Enter your assigned username when prompted.
3. Enter your password.
4. The system displays the menu for your assigned role.

If your login fails, check your username and password. Contact the administrator if you need your password reset.

### 2.3 Navigating menus

- Read the numbered menu options displayed on screen.
- Enter the number corresponding to the action you want.
- Follow the prompts and enter the requested information.
- Confirm important operations when prompted.
- Select the logout option to leave your account.

### 2.4 Changing your password

All four roles can change their own passwords.

1. Select **Change Own Password**.
2. Enter your current password.
3. Enter your new password (minimum 8 characters).
4. Enter the new password again.
5. The system confirms when the password has been changed.

## 3. Administrator User Manual

### 3.1 Role description

The administrator manages school accounts, departments, courses, lecturer assignments, student enrolments and the system audit log.

### 3.2 Managing user accounts

Select **Manage Users**.

Available operations:

- **List users:** Display registered accounts and their roles.
- **Create user:** Register a student, lecturer, Head of Department or administrator.
- **Reset a password:** Set a new password for an existing user.
- **Rename a user:** Change an existing username.
- **Delete a user:** Permanently remove an account after confirmation.

When creating a user:

1. Select the required role.
2. Enter the username.
3. Enter an initial password of at least 8 characters.
4. For students, enter the student number.
5. For lecturers, enter the staff number.
6. For Heads of Department, select the relevant department.
7. Confirm the account creation.

### 3.3 Managing departments

Select **Manage Departments**.

Available operations:

- List departments.
- Create a department.
- Rename a department.
- Delete a department.

Enter the department name when creating a department. When renaming or deleting, select the relevant department ID.

### 3.4 Managing courses

Select **Manage Courses**.

Available operations:

- List courses.
- Create a course.
- Change a course title.
- Delete a course.

When creating a course:

1. Enter the course code.
2. Enter the course title.
3. Select the department ID.

### 3.5 Assigning lecturers to courses

Select **Assign Lecturers**.

1. Choose **Add** or **Remove**.
2. Select the lecturer ID from the displayed list.
3. Select the course code.
4. The system confirms the operation.

### 3.6 Enrolling students

Select **Enrol Students**.

1. Choose **Add** or **Remove**.
2. Select the student ID.
3. Select the course code.
4. The system confirms the operation.

### 3.7 Viewing the audit log

Select **View Audit Log**.

1. Enter the number of recent entries to display (1–1000).
2. Review the entries, displayed newest first.

The audit log includes information about mark changes, the user responsible, timestamps, previous and new values, and reasons.

### 3.8 Administrator responsibilities

- Create and maintain user accounts.
- Assign users to the correct roles.
- Maintain departments and courses.
- Assign lecturers and enrol students correctly.
- Reset passwords when necessary.
- Protect administrator credentials.
- Monitor the audit log.

## 4. Head of Department (HoD) User Manual

### 4.1 Role description

The Head of Department reviews marks submitted by lecturers, moderates provisional marks and releases approved marks to students.

### 4.2 Reviewing department marks

Select **Review Department Marks**.

1. View the marks associated with your department.
2. Review raw marks, final marks, late-submission information and mark status.
3. Enter a mark number if you want to inspect its change history.
4. Review previous values, new values, timestamps and reasons for changes.

Enter `0` when prompted to skip viewing a mark's history.

### 4.3 Moderating marks

Select **Moderate Mark**.

1. View marks awaiting moderation.
2. Select the mark number to moderate.
3. Confirm whether you approve the mark exactly as it is.
4. If you do not approve it, enter the adjusted final mark and the reason for the adjustment.
5. The system confirms that the mark has been moderated.

A moderated mark is not automatically visible to students.

### 4.4 Releasing marks

Select **Release Marks**.

1. Review marks that have been moderated and are ready for release.
2. Confirm whether you want to release all listed marks.
3. If you choose not to release all, select an individual mark number.
4. The system confirms the number of marks released or confirms the individual release.

Students can only view marks after they have been released.

### 4.5 HoD responsibilities

- Review marks submitted by lecturers.
- Check mark accuracy and consistency.
- Provide reasons when adjusting marks.
- Ensure marks are moderated before release.
- Release approved marks to students.
- Review mark-change history when necessary.

## 5. Lecturer User Manual

### 5.1 Role description

Lecturers manage their assigned courses, create assignments, grant student deadline extensions and mark student submissions.

### 5.2 Viewing teaching courses

Select **View Teaching Courses**.

The system displays your assigned courses and the assignments belonging to each course.

If you have not been assigned to a course, contact the administrator.

### 5.3 Creating assignments

Select **Create Assignment**.

1. Select one of your assigned courses.
2. Enter the assignment title.
3. Enter the deadline.
4. Enter the maximum mark (greater than zero and up to 1000).
5. The system creates the assignment and displays its assigned number.

Ensure the deadline and maximum mark are correct before completing the operation.

### 5.4 Granting deadline extensions

Select **Grant Extension**.

1. Select the relevant course.
2. Select the assignment number.
3. Review the enrolled students.
4. Enter the student's ID.
5. Enter the number of extra days (1–30).
6. Enter the reason for the extension.
7. The system confirms the extension.

Only grant extensions when authorised by school policy.

### 5.5 Marking student submissions

Select **Mark Submission**.

1. Select the relevant course.
2. Select the assignment number.
3. Review the available submissions.
4. Select the submission number.
5. Enter the raw mark before any late penalty.
6. Enter written feedback for the student.
7. If changing an existing mark, provide a reason.
8. The system calculates the final mark after applying any applicable late penalty.

The system confirms whether the mark was saved or updated.

**Important:** A lecturer's mark remains provisional until it has been moderated and released by the Head of Department. Students cannot see provisional marks.

### 5.6 Lecturer responsibilities

- Work only with assigned courses.
- Create accurate assignments and deadlines.
- Grant authorised extensions.
- Review student submissions carefully.
- Enter accurate marks and useful feedback.
- Provide reasons when changing existing marks.
- Understand that the HoD must moderate and release marks.

## 6. Student User Manual

### 6.1 Role description

Students use the LMS to view their courses, check assignments, submit work and view released marks and feedback.

### 6.2 Viewing courses

Select **View My Courses**.

The system displays the courses in which you are enrolled.

If no courses are displayed, contact the school administrator.

### 6.3 Viewing assignments

Select **View My Assignments**.

The system displays your assignments, including:

- Assignment number.
- Course code.
- Assignment title.
- Submission deadline.
- Maximum mark.
- Current submission status.

Check deadlines regularly.

### 6.4 Submitting an assignment

Select **Submit Assignment**.

1. Review your available assignments.
2. Enter the assignment number.
It also displays the effective deadline and indicates whether the work was submitted on time or late.

If you submit late, the system displays the number of days late and the applicable penalty rule.
**Important:** Ensure that the file path is correct and that your work is available on the computer.
Select **View Released Marks**.

The system displays marks that have been moderated and released by the Head of Department.

For each released mark, students can view:

- Course code.
- Late-submission information, where applicable.
- Lecturer feedback.

Marks that have not been released will not appear.

### 6.6 Student responsibilities

- Keep login credentials confidential.
- Check courses and assignments regularly.
- Submit work before deadlines.
- Verify the file path before submitting.
- Read lecturer feedback.
- Contact the lecturer or administrator if there are problems.

## 7. Common Problems and Assistance

| Problem | Recommended action |
|---|---|
| Login unsuccessful | Check your username and password. Contact the administrator if necessary. |
| No courses displayed | Ask the administrator to check course enrolment or lecturer assignment. |
| No assignments displayed | Confirm that you are enrolled in the correct course. |
| Submission file not found | Check the file path and confirm that the file exists. |
| Mark not visible | Wait for HoD moderation and release. |
| Cannot perform an operation | Confirm that your account has the correct role and permissions. |
| Password forgotten | Contact the administrator for a password reset. |

## 8. Security Guidelines

All users should:

- Keep passwords private.
- Never share login credentials.
- Log out after using the application.
- Use only their assigned accounts.
- Avoid accessing information belonging to other users.
- Report suspicious activity or unexpected errors to the administrator.

---

**Document:** LMS User Manuals
**Application:** Moderated Learning Management System
**Version:** 1.0.0
**Roles covered:** Administrator, Head of Department, Lecturer and Student- Assignment title.
- Final mark.
- Maximum mark.

### 6.5 Viewing released marks and feedback


You may resubmit until the effective deadline.


The system confirms whether the submission was saved or resubmitted.
