#include "service/AuthService.h"

#include "common/Exceptions.h"
#include "common/Validation.h"
#include "service/AccessControl.h"

namespace lms {

std::unique_ptr<User> AuthService::login(const std::string& username, const std::string& password) const {
    const std::string generic = "Invalid username or password.";
    if (validation::isBlank(username) || password.empty() || password.size() > 128) throw AuthenticationError(generic);

    auto user = users_.findByUsername(validation::trim(username));
    if (!user) {
        if (dummyHash_.empty()) dummyHash_ = hasher_.hash("not-a-real-password");
        hasher_.verify(password, dummyHash_);
        throw AuthenticationError(generic);
    }
    if (!hasher_.verify(password, user->passwordHash())) throw AuthenticationError(generic);
    return user;
}

void AuthService::changeOwnPassword(const User& actor, const std::string& oldPassword, const std::string& newPassword) {
    requireAction(actor, Action::ChangeOwnPassword);
    if (!hasher_.verify(oldPassword, actor.passwordHash())) throw AuthenticationError("Current password is incorrect.");
    validation::requirePassword(newPassword, actor.username());
    users_.updatePasswordHash(actor.id(), hasher_.hash(newPassword));
}

}  // namespace lms
