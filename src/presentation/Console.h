#pragma once
#include <iosfwd>
#include <string>
#include <vector>

#include "common/DateTime.h"

namespace lms {

/// Thrown when standard input ends (Ctrl-D / closed pipe). Not an LmsError: it ends the program cleanly.
class InputClosed {};

/// All keyboard/screen interaction. Streams are injected so menus can be tested with scripted input.
/// Times are stored in UTC but typed and shown in school local time (fixed UTC offset; Lesotho = +120 min).
class Console {
public:
    Console(std::istream& in, std::ostream& out, int utcOffsetMinutes = 120, bool hidePasswords = false)
        : in_(in), out_(out), offsetMinutes_(utcOffsetMinutes), hidePasswords_(hidePasswords) {}

    void print(const std::string& s);
    void println(const std::string& s = "");
    void heading(const std::string& s);
    void error(const std::string& s);

    std::string prompt(const std::string& label);                      ///< one line, trimmed
    std::string promptPassword(const std::string& label);              ///< not echoed on a real terminal
    int promptInt(const std::string& label, int min, int max);         ///< re-asks until valid
    double promptDouble(const std::string& label, double min, double max);
    DateTime promptDateTime(const std::string& label);                 ///< local time 'YYYY-MM-DD HH:MM'
    bool confirm(const std::string& label);                            ///< y/n
    /// Shows numbered options plus "0. <backLabel>"; returns 1..n or 0 for back.
    int choose(const std::string& title, const std::vector<std::string>& options,
               const std::string& backLabel = "Back");

    std::string local(DateTime utc) const;                              ///< UTC -> 'YYYY-MM-DD HH:MM'
    DateTime fromLocal(DateTime localWallClock) const { return localWallClockToUtc(localWallClock); }
    static std::string money(double v);                                 ///< 2-decimal number

private:
    DateTime localWallClockToUtc(DateTime wall) const { return wall.addSeconds(-static_cast<std::int64_t>(offsetMinutes_) * 60); }
    std::string readLine();

    std::istream& in_;
    std::ostream& out_;
    int offsetMinutes_;
    bool hidePasswords_;
};

}  // namespace lms
