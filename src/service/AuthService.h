#pragma once
#include <memory>

#include "data/Repositories.h"
#include "security/IPasswordHasher.h"

namespace lms {

class AuthService {
public:
    AuthService(IUserRepository& users, const IPasswordHasher& hasher) : users_(users), hasher_(hasher) {}

    /// FR-01. The same generic error is used for "no such user" and "wrong password" so the
    /// system does not reveal which usernames exist, and a dummy hash is checked for unknown
    /// users so the response time does not reveal it either.
    std::unique_ptr<User> login(const std::string& username, const std::string& password) const;

    void changeOwnPassword(const User& actor, const std::string& oldPassword, const std::string& newPassword);

private:
    IUserRepository& users_;
    const IPasswordHasher& hasher_;
    mutable std::string dummyHash_;
};

}  // namespace lms
