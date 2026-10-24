#pragma once
#include <iosfwd>

#include "System.h"

namespace lms {
/// Fills an empty database with a small demonstration data set (for the viva and manual testing).
/// Passwords are printed to the console; never use this on a real installation.
void seedDemoData(System& sys, const IClock& clock, std::ostream& out);
}  // namespace lms
