#include "presentation/Console.h"

#include <iomanip>
#include <iostream>
#include <sstream>

#include "common/Exceptions.h"
#include "common/Validation.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

namespace lms {

void Console::print(const std::string& s) { out_ << s << std::flush; }
void Console::println(const std::string& s) { out_ << s << '\n' << std::flush; }
void Console::heading(const std::string& s) {
    out_ << "\n=== " << s << " ===\n" << std::flush;
}
void Console::error(const std::string& s) { out_ << "  ! " << s << '\n' << std::flush; }

std::string Console::readLine() {
    std::string line;
    if (!std::getline(in_, line)) throw InputClosed{};
    if (!line.empty() && line.back() == '\r') line.pop_back();
    return line;
}

std::string Console::prompt(const std::string& label) {
    print(label + ": ");
    return validation::trim(readLine());
}

std::string Console::promptPassword(const std::string& label) {
    print(label + ": ");
    if (!hidePasswords_) return readLine();
#ifdef _WIN32
    HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(h, &mode);
    SetConsoleMode(h, mode & ~ENABLE_ECHO_INPUT);
    std::string s;
    try { s = readLine(); } catch (...) { SetConsoleMode(h, mode); throw; }
    SetConsoleMode(h, mode);
#else
    termios oldt{};
    const bool tty = isatty(STDIN_FILENO) && tcgetattr(STDIN_FILENO, &oldt) == 0;
    if (tty) {
        termios newt = oldt;
        newt.c_lflag &= ~static_cast<tcflag_t>(ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    }
    std::string s;
    try { s = readLine(); } catch (...) { if (tty) tcsetattr(STDIN_FILENO, TCSANOW, &oldt); throw; }
    if (tty) tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif
    println();
    return s;
}

int Console::promptInt(const std::string& label, int min, int max) {
    for (;;) {
        try {
            return validation::parseInt(prompt(label), label, min, max);
        } catch (const ValidationError& e) {
            error(e.what());
        }
    }
}

double Console::promptDouble(const std::string& label, double min, double max) {
    for (;;) {
        try {
            return validation::parseDouble(prompt(label), label, min, max);
        } catch (const ValidationError& e) {
            error(e.what());
        }
    }
}

DateTime Console::promptDateTime(const std::string& label) {
    for (;;) {
        try {
            return fromLocal(DateTime::parseFlexible(prompt(label + " (YYYY-MM-DD HH:MM)")));
        } catch (const ValidationError& e) {
            error(e.what());
        }
    }
}

bool Console::confirm(const std::string& label) {
    for (;;) {
        std::string a = prompt(label + " (y/n)");
        if (a == "y" || a == "Y" || a == "yes") return true;
        if (a == "n" || a == "N" || a == "no") return false;
        error("Please answer y or n.");
    }
}

int Console::choose(const std::string& title, const std::vector<std::string>& options, const std::string& backLabel) {
    heading(title);
    for (std::size_t i = 0; i < options.size(); ++i) out_ << "  " << (i + 1) << ". " << options[i] << '\n';
    out_ << "  0. " << backLabel << '\n' << std::flush;
    return promptInt("Choose", 0, static_cast<int>(options.size()));
}

std::string Console::local(DateTime utc) const {
    return utc.addSeconds(static_cast<std::int64_t>(offsetMinutes_) * 60).toString().substr(0, 16);
}

std::string Console::money(double v) {
    std::ostringstream o;
    o << std::fixed << std::setprecision(2) << v;
    return o.str();
}

}  // namespace lms
