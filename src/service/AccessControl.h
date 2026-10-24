#pragma once
#include "common/Exceptions.h"
#include "domain/User.h"

namespace lms {

/// Central role check used by every service method (FR-02, NFR-04).
inline void requireAction(const User& actor, Action action) {
    if (!actor.can(action))
        throw AuthorizationError("Your role (" + actor.roleLabel() + ") is not allowed to do this.");
}

}  // namespace lms
