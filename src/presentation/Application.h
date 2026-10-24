#pragma once
#include "System.h"
#include "presentation/Console.h"

namespace lms {

/// Top-level loop: first-run setup, login, role menu, repeat.
class Application {
public:
    Application(System& sys, Console& con) : sys_(sys), con_(con) {}
    void run();

private:
    void firstRunSetup();
    void sessionFor(const User& user);

    System& sys_;
    Console& con_;
};

}  // namespace lms
