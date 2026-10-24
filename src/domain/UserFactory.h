#pragma once
#include <memory>
#include <string>

#include "domain/User.h"
#include "security/IPasswordHasher.h"

namespace lms {

/// Input needed to create a new account.
struct NewUserSpec {
    UserRole role = UserRole::Student;
    std::string username;
    std::string password;
    std::string studentNo;   ///< required for STUDENT
    std::string staffNo;     ///< required for LECTURER
    int departmentId = 0;    ///< required for HOD
};

/// Factory pattern: the single place that turns role data into the right User subclass.
///  * create()  - validates a brand-new account and hashes the password (id is assigned by the DB);
///  * hydrate() - rebuilds a User from a stored record (no validation, no hashing).
class UserFactory {
public:
    explicit UserFactory(const IPasswordHasher& hasher) : hasher_(hasher) {}

    std::unique_ptr<User> create(const NewUserSpec& spec) const;
    static std::unique_ptr<User> hydrate(const UserRecord& record);

private:
    const IPasswordHasher& hasher_;
};

}  // namespace lms
