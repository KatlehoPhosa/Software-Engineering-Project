#include "service/AccountService.h"

#include "common/Exceptions.h"
#include "common/Validation.h"
#include "service/AccessControl.h"

namespace lms {

int AccountService::createFirstAdministrator(const std::string& username, const std::string& password) {
    auto tx = tx_.begin();
    if (users_.count() != 0) throw AuthorizationError("First-run setup is only available on an empty system.");
    NewUserSpec spec;
    spec.role = UserRole::Admin;
    spec.username = username;
    spec.password = password;
    auto user = factory_.create(spec);
    const int id = users_.add(*user);
    tx->commit();
    return id;
}

int AccountService::createUser(const User& actor, const NewUserSpec& spec) {
    requireAction(actor, Action::ManageUsers);
    auto user = factory_.create(spec);  // validates fields and hashes the password
    auto tx = tx_.begin();
    if (users_.findByUsername(user->username())) throw BusinessRuleError("That username is already in use.");
    if (spec.role == UserRole::HoD && !depts_.findById(spec.departmentId))
        throw NotFoundError("Department not found.");
    int id = 0;
    try {
        id = users_.add(*user);
    } catch (const ConstraintViolation&) {
        throw BusinessRuleError(
            "Could not create the account: the student/staff number is already used, or the department already has a head.");
    }
    tx->commit();
    return id;
}

std::vector<UserRecord> AccountService::listUsers(const User& actor) {
    requireAction(actor, Action::ManageUsers);
    return users_.listAll();
}

void AccountService::resetPassword(const User& actor, int userId, const std::string& newPassword) {
    requireAction(actor, Action::ManageUsers);
    auto target = users_.findById(userId);
    if (!target) throw NotFoundError("User not found.");
    validation::requirePassword(newPassword, target->username());
    users_.updatePasswordHash(userId, hasher_.hash(newPassword));
}

void AccountService::renameUser(const User& actor, int userId, const std::string& newUsername) {
    requireAction(actor, Action::ManageUsers);
    const std::string name = validation::requireUsername(newUsername);
    if (!users_.findById(userId)) throw NotFoundError("User not found.");
    auto clash = users_.findByUsername(name);
    if (clash && clash->id() != userId) throw BusinessRuleError("That username is already in use.");
    users_.updateUsername(userId, name);
}

void AccountService::deleteUser(const User& actor, int userId) {
    requireAction(actor, Action::ManageUsers);
    if (actor.id() == userId) throw BusinessRuleError("You cannot delete your own account.");
    auto target = users_.findById(userId);
    if (!target) throw NotFoundError("User not found.");
    if (target->role() == UserRole::Admin && users_.countByRole(UserRole::Admin) <= 1)
        throw BusinessRuleError("The last administrator cannot be deleted.");
    auto tx = tx_.begin();
    try {
        users_.remove(userId);
    } catch (const ConstraintViolation&) {
        throw BusinessRuleError("This user has assignments, submissions, marks or audit records and cannot be deleted.");
    }
    tx->commit();
}

}  // namespace lms
