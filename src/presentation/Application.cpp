#include "presentation/Application.h"

#include "common/Exceptions.h"
#include "presentation/RoleMenu.h"

namespace lms {

void Application::firstRunSetup() {
    con_.heading("First-time setup");
    con_.println("No accounts exist yet. Create the system administrator account.");
    for (;;) {
        try {
            const std::string name = con_.prompt("Administrator username");
            const std::string pw = con_.promptPassword("Password (min 8 characters)");
            if (pw != con_.promptPassword("Repeat password")) throw ValidationError("The passwords do not match.");
            sys_.accounts.createFirstAdministrator(name, pw);
            con_.println("Administrator account created. Please log in.");
            return;
        } catch (const LmsError& e) {
            con_.error(e.what());
        }
    }
}

void Application::sessionFor(const User& user) {
    auto menu = makeRoleMenu(user, sys_, con_);
    menu->run(user);
}

void Application::run() {
    try {
        if (sys_.users.count() == 0) firstRunSetup();
        for (;;) {
            con_.heading("Moderated Learning Management System");
            con_.println("  1. Log in");
            con_.println("  0. Exit");
            try {
                if (con_.promptInt("Choose", 0, 1) == 0) break;
                const std::string name = con_.prompt("Username");
                const std::string pw = con_.promptPassword("Password");
                auto user = sys_.auth.login(name, pw);
                con_.println("Welcome, " + user->username() + " (" + user->roleLabel() + ").");
                sessionFor(*user);
            } catch (const LmsError& e) {
                con_.error(e.what());  // login failures, database errors: report and continue
            }
        }
    } catch (const InputClosed&) {
        // keyboard input ended: leave quietly
    }
    con_.println("Goodbye.");
}

}  // namespace lms
