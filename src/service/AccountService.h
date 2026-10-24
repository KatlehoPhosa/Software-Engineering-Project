#pragma once
#include <vector>

#include "data/Repositories.h"
#include "domain/UserFactory.h"

namespace lms {

/// FR-03: administrator manages user accounts.
class AccountService {
public:
    AccountService(IUserRepository& users, IDepartmentRepository& depts, ITransactionManager& tx,
                   const UserFactory& factory, const IPasswordHasher& hasher)
        : users_(users), depts_(depts), tx_(tx), factory_(factory), hasher_(hasher) {}

    /// First-run setup: only permitted while the user table is empty.
    int createFirstAdministrator(const std::string& username, const std::string& password);

    int createUser(const User& actor, const NewUserSpec& spec);
    std::vector<UserRecord> listUsers(const User& actor);
    void resetPassword(const User& actor, int userId, const std::string& newPassword);
    void renameUser(const User& actor, int userId, const std::string& newUsername);
    void deleteUser(const User& actor, int userId);

private:
    IUserRepository& users_;
    IDepartmentRepository& depts_;
    ITransactionManager& tx_;
    const UserFactory& factory_;
    const IPasswordHasher& hasher_;
};

}  // namespace lms
