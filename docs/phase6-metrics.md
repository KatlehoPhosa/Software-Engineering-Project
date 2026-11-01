# Phase 6 – Configuration Management and Project Metrics

## 1. Overview

This document records the configuration management activities, code metrics, static analysis results, automated testing and release management completed during Phase 6 of the LMS Software Engineering Project.

## 2. Configuration Management

The project uses Git for version control and GitHub for remote repository hosting.

- **Repository:** `KatlehoPhosa/Software-Engineering-Project`
- **Working branch:** `phase6-configuration-management`
- **Release tag:** `v1.0.0`
- **Branch synchronisation:** Local branch successfully pushed to GitHub.
- **Working tree:** Clean after committing and pushing changes.

Meaningful commits were used to record configuration management and implementation changes.

## 3. Automated Build and Testing

A GitHub Actions workflow was created at `.github/workflows/ci.yml`.

The workflow automatically:
1. Checks out the repository.
2. Installs required dependencies.
3. Configures the project using CMake.
4. Builds the LMS application.
5. Runs GoogleTest through CTest.

**Verification results:**

- GoogleTests passed: 43/43.
- Test success rate: 100%.
- Local CTest execution time: approximately 1.86 seconds.
- GitHub Actions workflow runs verified: 2 successful runs.

## 4. Static Code Analysis

Cppcheck version 2.21.0 was used to analyse the C++ source code.

Analysis command:

```bash
cppcheck --enable=warning,style,performance,portability --std=c++17 --suppress=missingIncludeSystem -I src src
```

Initial analysis identified three style warnings involving parameter names shadowing class getter methods.

The affected parameters were renamed without changing application behaviour.

A subsequent analysis checked 25 source files and reported:

- Warnings: 0
- Final analysis status: Successful

## 5. Source Code Metrics

CLOC version 2.10 was used to measure the source code.

Command:

```bash
cloc src tests
```

### Results

| Metric | Result |
|---|---:|
| Text files analysed | 63 |
| Unique files | 62 |
| Files ignored | 3 |
| C++ files | 28 |
| C/C++ header files | 34 |
| Code lines | 3,947 |
| Comment lines | 181 |
| Blank lines | 652 |

### Language breakdown

| Language | Files | Blank | Comments | Code |
|---|---:|---:|---:|---:|
| C++ | 28 | 360 | 35 | 2,686 |
| C/C++ Header | 34 | 292 | 146 | 1,261 |
| **Total** | **62** | **652** | **181** | **3,947** |

## 6. Test Coverage

The Phase 5 coverage analysis estimated business logic coverage at approximately 84.83%.

This estimate was calculated from approximately 688 executed lines out of 811 executable lines, based on rounded gcov summaries. It should therefore be treated as an estimate rather than an exact coverage measurement.

## 7. Release Management

An annotated Git tag was created and successfully pushed to the remote repository.

- **Tag:** `v1.0.0`
- **Description:** LMS Phase 6 release
- **Publication:** Successfully pushed to GitHub.

The tag identifies the specific project commit associated with this release.

## 8. Conclusion

Phase 6 configuration management activities included Git branching, meaningful commits, remote repository synchronisation, automated build and test integration, static code analysis, source code metrics and release tagging.

The LMS project successfully passed all 43 GoogleTests locally, and two GitHub Actions workflow runs completed successfully. The final Cppcheck analysis reported zero warnings across 25 source files.

These results provide evidence of the project's configuration management, testing and code quality activities.
