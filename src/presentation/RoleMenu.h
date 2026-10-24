#pragma once
#include <memory>

#include "System.h"
#include "presentation/Console.h"

namespace lms {

/// One console menu per role. The menu entries come from User::allowedActions(), so the
/// menu and the access-control rules can never disagree (FR-02, Principal's "simple" requirement).
class IRoleMenu {
public:
    virtual ~IRoleMenu() = default;
    virtual void run(const User& user) = 0;
};

/// Template method: shows the menu, dispatches one action, reports errors without crashing.
class RoleMenuBase : public IRoleMenu {
public:
    RoleMenuBase(System& sys, Console& con) : sys_(sys), con_(con) {}
    void run(const User& user) override;

protected:
    virtual void dispatch(const User& user, Action action) = 0;
    virtual void onLogin(const User&) {}
    void changePassword(const User& user);

    System& sys_;
    Console& con_;
};

/// Factory Method: picks the menu that matches the user's role.
std::unique_ptr<IRoleMenu> makeRoleMenu(const User& user, System& sys, Console& con);

}  // namespace lms
