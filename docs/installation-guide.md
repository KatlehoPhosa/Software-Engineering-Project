# LMS Installation Guide
**Moderated Learning Management System – Version 1.0.0**

## 1. Introduction

This guide explains how a school technician can install, build, configure and run the Moderated Learning Management System (LMS).

The LMS is a C++17 console application designed for secondary schools. It supports student enrolment, courses, assignments, submissions, marking, Head-of-Department moderation and audit logging.

The application works offline and stores its information in a local SQLite database.

## 2. System Requirements

### Hardware
- Desktop or laptop computer.
- At least 4 GB RAM recommended.
- At least 500 MB free disk space.
- Keyboard and monitor.

### Software
- Windows 10/11 or Ubuntu/Debian Linux.
- C++17-compatible compiler.
- CMake version 3.16 or later.
- SQLite 3 development libraries.
- libsodium 1.0.18 or later.
- Git (if installing from the source repository).

## 3. Installation on Windows

The following instructions use MSYS2 UCRT64.

### Step 1: Install MSYS2

1. Download MSYS2 from https://www.msys2.org/.
2. Install it using the installation wizard.
3. Open the **MSYS2 UCRT64** terminal.

### Step 2: Install dependencies

Update the package database and install the required tools:

```bash
pacman -Syu
```

If the terminal asks you to close, close it, reopen MSYS2 UCRT64 and continue.

Install the development packages:

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-make mingw-w64-ucrt-x86_64-pkgconf mingw-w64-ucrt-x86_64-sqlite3 mingw-w64-ucrt-x86_64-libsodium git
```

### Step 3: Obtain the project

Clone the school LMS repository:

```bash
git clone https://github.com/KatlehoPhosa/Software-Engineering-Project.git
```

Enter the project directory:

```bash
cd Software-Engineering-Project
```

Alternatively, if the school has supplied the project source code, open its extracted project directory.

### Step 4: Configure the project

Run:

```bash
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DLMS_BUILD_TESTS=OFF
```

CMake checks the compiler and required libraries and prepares the build files.

### Step 5: Build the application

Run:

```bash
cmake --build build --parallel
```

After a successful build, the executable is located at:

```text
build/lms.exe
```

### Step 6: Start the LMS

Run:

```bash
./build/lms.exe
```

On the first run, the application asks the technician to create the administrator account.

There are no default administrator passwords.

## 4. Installation on Ubuntu/Debian Linux

### Step 1: Update the system

```bash
sudo apt update
```

### Step 2: Install dependencies

```bash
sudo apt install build-essential cmake pkg-config libsqlite3-dev libsodium-dev git
```

### Step 3: Obtain the project

```bash
git clone https://github.com/KatlehoPhosa/Software-Engineering-Project.git
cd Software-Engineering-Project
```

### Step 4: Configure the project

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DLMS_BUILD_TESTS=OFF
```

### Step 5: Build the application

```bash
cmake --build build --parallel
```

### Step 6: Run the LMS

```bash
./build/lms
```

On first launch, create the administrator account when prompted.

## 5. Database Configuration

The LMS uses SQLite for local data storage.

By default, the application creates a database named:

```text
lms.db
```

The technician can specify a different database location:

```bash
./build/lms --db /path/to/school/lms.db
```

The `LMS_DB` environment variable can also specify the default database path.

The application creates the database when necessary.

### Database backup

1. Ask all users to exit the LMS.
2. Ensure the application has stopped.
3. Copy the database file to a secure backup location.
4. Store backups somewhere separate from the computer running the LMS.
5. Periodically test that a backup can be restored.

Do not copy a database while it is actively being modified. SQLite may use additional journal or WAL files during operation.

## 6. Demonstration Data

The technician can create demonstration data using:

```bash
./build/lms --seed-demo --db demo.db
```

Then start the demonstration database:

```bash
./build/lms --db demo.db
```

The demonstration accounts use the password:

```text
Demo#1234
```

Demonstration accounts include an administrator, Heads of Department, lecturers and students.

**Security warning:** Demonstration accounts and passwords must not be used for normal school operations. Use a separate demonstration database and protect or remove it before deployment.

## 7. Initial Administrator Setup

On the first launch with an empty database:

1. Start the LMS.
2. Follow the prompts to create the administrator account.
3. Choose a strong password.
4. Keep administrator credentials confidential.
5. Log in and configure the school's accounts and academic information.

There are no preconfigured default administrator credentials.

## 8. Running the Application

To start the application, use the executable generated during the build.

Windows:

```bash
./build/lms.exe
```

Linux:

```bash
./build/lms
```

The LMS is an offline console application. Users interact with it through the keyboard and the menus available to their assigned roles.

## 9. Troubleshooting

| Problem | Possible solution |
|---|---|
| CMake cannot find SQLite | Install the SQLite development package and rerun CMake. |
| CMake cannot find libsodium | Install libsodium and pkg-config development packages. |
| Compiler not found | Check that the compiler is installed and available in the terminal. |
| Build fails | Check the error output, dependencies and CMake configuration. |
| Application cannot start | Confirm that the build completed and the executable exists. |
| Database cannot be opened | Check the database path and directory permissions. |
| User cannot log in | Verify the username and password or ask the administrator for assistance. |

## 10. Maintenance and Security

The school technician should:

- Keep the operating system and development/runtime dependencies updated.
- Restrict access to the LMS database and project files.
- Maintain regular database backups.
- Protect administrator credentials.
- Avoid sharing demonstration credentials with real users.
- Check that backups can be restored.
- Ensure users exit the application before maintenance or backup operations.

## 11. Support

For technical problems, contact the school's designated LMS administrator or software maintenance team.

---

**Document:** LMS Installation Guide
**Application:** Moderated Learning Management System
**Version:** 1.0.0
