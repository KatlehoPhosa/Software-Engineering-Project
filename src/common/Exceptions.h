#pragma once
#include <stdexcept>
#include <string>

namespace lms {

/// Base class of every error the application raises on purpose.
/// The presentation layer catches LmsError, shows what() to the user and carries on.
class LmsError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

/// Input is malformed or violates a field rule (blank title, mark out of range, bad date...).
class ValidationError : public LmsError {
public:
    using LmsError::LmsError;
};

/// Username/password not accepted.
class AuthenticationError : public LmsError {
public:
    using LmsError::LmsError;
};

/// The signed-in user's role (or ownership of the record) does not allow the action.
class AuthorizationError : public LmsError {
public:
    using LmsError::LmsError;
};

/// A referenced record does not exist.
class NotFoundError : public LmsError {
public:
    using LmsError::LmsError;
};

/// A business rule forbids the action (resubmission after deadline, remarking a moderated mark...).
class BusinessRuleError : public LmsError {
public:
    using LmsError::LmsError;
};

/// SQLite reported a failure that is not a user-correctable constraint problem.
class DatabaseError : public LmsError {
public:
    using LmsError::LmsError;
};

/// A UNIQUE / FOREIGN KEY / CHECK constraint rejected the statement.
class ConstraintViolation : public DatabaseError {
public:
    using DatabaseError::DatabaseError;
};

}  // namespace lms
