# Static Code Analysis Report – LMS Project

## 1. Introduction

Static code analysis was performed during Phase 6 of the LMS Software Engineering Project to identify potential programming errors, coding style issues, performance concerns and portability problems.

## 2. Analysis Tool

| Item | Details |
|---|---|
| Tool | Cppcheck |
| Version | 2.21.0 |
| Programming language | C++ |
| C++ standard | C++17 |
| Files analysed | 25 source files |
| Analysis categories | Warning, Style, Performance, Portability |

## 3. Analysis Configuration

The following command was used:

```bash
cppcheck --enable=warning,style,performance,portability --std=c++17 --suppress=missingIncludeSystem -I src src
```

The analysis covered the project's source code in the `src` directory.

## 4. Initial Findings

The initial analysis identified three style warnings in `src/domain/Entities.cpp`.

The warnings involved parameter names shadowing existing getter methods:

- `fileRef`
- `rawMark`
- `feedback`

These parameters were renamed to avoid shadowing:

- `fileRef` → `fileReference`
- `rawMark` → `rawMarkValue`
- `feedback` → `feedbackText`

The changes improved code readability without changing application behaviour.

## 5. Final Analysis Results

After correcting the identified issues, Cppcheck was executed again.

| Metric | Result |
|---|---:|
| Source files checked | 25 |
| Remaining warnings | 0 |
| Final analysis status | Successful |

The final analysis reported no warnings in the selected warning, style, performance and portability categories.

## 6. Integration with Continuous Integration

Cppcheck was integrated into the GitHub Actions workflow located at:

`.github/workflows/ci.yml`

The workflow runs static analysis automatically after building the application and executing the automated tests.

The command used in the workflow is:

```bash
cppcheck --enable=warning,style,performance,portability --std=c++17 --suppress=missingIncludeSystem -I src --error-exitcode=1 src
```

The `--error-exitcode=1` option ensures that detected Cppcheck findings cause the analysis step to fail.

The latest GitHub Actions workflow run completed successfully.

## 7. Conclusion

Static code analysis was successfully performed using Cppcheck 2.21.0.

Three initial style warnings were identified and corrected. A subsequent analysis reported zero warnings across the 25 checked source files.

Integrating Cppcheck into GitHub Actions also ensures that static analysis is performed automatically during future development.

This provides evidence of the project's commitment to code quality, maintainability and continuous integration.
