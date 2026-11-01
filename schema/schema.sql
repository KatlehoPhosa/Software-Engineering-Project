-- =====================================================
-- LEARNING MANAGEMENT SYSTEM (LMS)
-- SQLite schema matching the data dictionary
-- =====================================================
PRAGMA foreign_keys = ON;

-- 1. USER
CREATE TABLE IF NOT EXISTS "user" (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT NOT NULL COLLATE NOCASE UNIQUE,
    password_hash TEXT NOT NULL,
    role TEXT NOT NULL
        CHECK (role IN ('STUDENT', 'LECTURER', 'HOD', 'ADMIN'))
);

-- 2. DEPARTMENT
CREATE TABLE IF NOT EXISTS department (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL UNIQUE
);

-- 3. STUDENT
CREATE TABLE IF NOT EXISTS student (
    user_id INTEGER PRIMARY KEY,
    student_no TEXT NOT NULL UNIQUE,
    FOREIGN KEY (user_id) REFERENCES "user"(id) ON DELETE CASCADE
);

-- 4. LECTURER
CREATE TABLE IF NOT EXISTS lecturer (
    user_id INTEGER PRIMARY KEY,
    staff_no TEXT NOT NULL UNIQUE,
    FOREIGN KEY (user_id) REFERENCES "user"(id) ON DELETE CASCADE
);

-- 5. HOD
CREATE TABLE IF NOT EXISTS hod (
    user_id INTEGER PRIMARY KEY,
    department_id INTEGER NOT NULL UNIQUE,
    FOREIGN KEY (user_id) REFERENCES "user"(id) ON DELETE CASCADE,
    FOREIGN KEY (department_id) REFERENCES department(id)
);

-- 6. ADMIN
CREATE TABLE IF NOT EXISTS admin (
    user_id INTEGER PRIMARY KEY,
    FOREIGN KEY (user_id) REFERENCES "user"(id) ON DELETE CASCADE
);

-- 7. COURSE
CREATE TABLE IF NOT EXISTS course (
    code TEXT PRIMARY KEY,
    title TEXT NOT NULL,
    dept_id INTEGER NOT NULL,
    FOREIGN KEY (dept_id) REFERENCES department(id) ON DELETE RESTRICT
);

-- 8. COURSE_LECTURER
CREATE TABLE IF NOT EXISTS course_lecturer (
    lecturer_id INTEGER NOT NULL,
    course_code TEXT NOT NULL,
    PRIMARY KEY (lecturer_id, course_code),
    FOREIGN KEY (lecturer_id) REFERENCES lecturer(user_id) ON DELETE CASCADE,
    FOREIGN KEY (course_code) REFERENCES course(code) ON DELETE CASCADE
);

-- 9. ENROLLMENT
CREATE TABLE IF NOT EXISTS enrollment (
    student_id INTEGER NOT NULL,
    course_code TEXT NOT NULL,
    PRIMARY KEY (student_id, course_code),
    FOREIGN KEY (student_id) REFERENCES student(user_id) ON DELETE CASCADE,
    FOREIGN KEY (course_code) REFERENCES course(code) ON DELETE CASCADE
);

-- 10. ASSIGNMENT
CREATE TABLE IF NOT EXISTS assignment (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    title TEXT NOT NULL CHECK (length(trim(title)) > 0),
    deadline TEXT NOT NULL,
    max_mark REAL NOT NULL CHECK (max_mark > 0),
    course_code TEXT NOT NULL,
    created_by INTEGER NOT NULL,
    FOREIGN KEY (course_code) REFERENCES course(code),
    FOREIGN KEY (created_by) REFERENCES lecturer(user_id)
);

-- 11. EXTENSION
CREATE TABLE IF NOT EXISTS extension (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    student_id INTEGER NOT NULL,
    assignment_id INTEGER NOT NULL,
    extra_days INTEGER NOT NULL CHECK (extra_days > 0),
    granted_by INTEGER NOT NULL,
    reason TEXT NOT NULL CHECK (length(trim(reason)) > 0),
    UNIQUE (student_id, assignment_id),
    FOREIGN KEY (student_id) REFERENCES student(user_id),
    FOREIGN KEY (assignment_id) REFERENCES assignment(id) ON DELETE CASCADE,
    FOREIGN KEY (granted_by) REFERENCES lecturer(user_id)
);

-- 12. SUBMISSION
CREATE TABLE IF NOT EXISTS submission (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    student_id INTEGER NOT NULL,
    assignment_id INTEGER NOT NULL,
    submitted_at TEXT NOT NULL,
    file_ref TEXT NOT NULL,
    state TEXT NOT NULL
        CHECK (state IN ('SUBMITTED', 'LATE', 'RESUBMITTED', 'MARKED')),
    UNIQUE (student_id, assignment_id),
    FOREIGN KEY (student_id) REFERENCES student(user_id),
    FOREIGN KEY (assignment_id) REFERENCES assignment(id)
);

-- 13. MARK
CREATE TABLE IF NOT EXISTS mark (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    submission_id INTEGER NOT NULL UNIQUE,
    raw_mark REAL NOT NULL CHECK (raw_mark >= 0),
    final_mark REAL CHECK (final_mark >= 0),
    feedback TEXT,
    state TEXT NOT NULL DEFAULT 'PROVISIONAL'
        CHECK (state IN ('PROVISIONAL', 'MODERATED', 'RELEASED')),
    FOREIGN KEY (submission_id) REFERENCES submission(id)
);

-- 14. AUDIT_ENTRY
CREATE TABLE IF NOT EXISTS audit_entry (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    entity_type TEXT NOT NULL,
    entity_id INTEGER NOT NULL,
    old_value TEXT,
    new_value TEXT NOT NULL,
    actor_id INTEGER NOT NULL,
    timestamp TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
    reason TEXT NOT NULL CHECK (length(trim(reason)) > 0),
    FOREIGN KEY (actor_id) REFERENCES "user"(id)
);

-- =====================================================
-- INDEXES FOR COMMON LOOKUPS
-- =====================================================
CREATE INDEX IF NOT EXISTS idx_course_department ON course(dept_id);
CREATE INDEX IF NOT EXISTS idx_assignment_course ON assignment(course_code);
CREATE INDEX IF NOT EXISTS idx_submission_student ON submission(student_id);
CREATE INDEX IF NOT EXISTS idx_submission_assignment ON submission(assignment_id);
CREATE INDEX IF NOT EXISTS idx_extension_student_assignment ON extension(student_id, assignment_id);
CREATE INDEX IF NOT EXISTS idx_audit_actor_timestamp ON audit_entry(actor_id, timestamp);
