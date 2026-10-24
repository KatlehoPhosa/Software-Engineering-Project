#include "common/Validation.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <cerrno>

#include "common/Exceptions.h"

namespace lms::validation {

std::string trim(const std::string& s) {
    auto b = s.begin();
    auto e = s.end();
    while (b != e && std::isspace(static_cast<unsigned char>(*b))) ++b;
    while (e != b && std::isspace(static_cast<unsigned char>(*(e - 1)))) --e;
    return std::string(b, e);
}

bool isBlank(const std::string& s) { return trim(s).empty(); }

std::string requireNonBlank(const std::string& value, const std::string& field, std::size_t maxLen) {
    std::string t = trim(value);
    if (t.empty()) throw ValidationError(field + " must not be blank.");
    if (t.size() > maxLen) throw ValidationError(field + " is too long (maximum " + std::to_string(maxLen) + " characters).");
    if (std::any_of(t.begin(), t.end(), [](unsigned char c) { return std::iscntrl(c) != 0; }))
        throw ValidationError(field + " contains invalid control characters.");
    return t;
}

std::string requireUsername(const std::string& value) {
    std::string t = trim(value);
    if (t.size() < 3 || t.size() > 32) throw ValidationError("Username must be 3-32 characters.");
    const bool ok = std::all_of(t.begin(), t.end(), [](unsigned char c) {
        return std::isalnum(c) || c == '.' || c == '_' || c == '-';
    });
    if (!ok) throw ValidationError("Username may contain only letters, digits, '.', '_' and '-'.");
    return t;
}

void requirePassword(const std::string& password, const std::string& username) {
    if (password.size() < 8) throw ValidationError("Password must be at least 8 characters.");
    if (password.size() > 128) throw ValidationError("Password must be at most 128 characters.");
    auto lower = [](std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return s;
    };
    if (lower(password) == lower(username)) throw ValidationError("Password must not be the same as the username.");
}

std::string requireCourseCode(const std::string& value) {
    std::string t = trim(value);
    if (t.size() < 2 || t.size() > 12) throw ValidationError("Course code must be 2-12 letters/digits.");
    for (char& c : t) {
        unsigned char u = static_cast<unsigned char>(c);
        if (!std::isalnum(u)) throw ValidationError("Course code may contain only letters and digits.");
        c = static_cast<char>(std::toupper(u));
    }
    return t;
}

int parseInt(const std::string& text, const std::string& field, int min, int max) {
    std::string t = trim(text);
    if (t.empty()) throw ValidationError(field + " is required.");
    errno = 0;
    char* end = nullptr;
    long v = std::strtol(t.c_str(), &end, 10);
    if (errno != 0 || end == t.c_str() || *end != '\0')
        throw ValidationError(field + " must be a whole number.");
    if (v < min || v > max)
        throw ValidationError(field + " must be between " + std::to_string(min) + " and " + std::to_string(max) + ".");
    return static_cast<int>(v);
}

double parseDouble(const std::string& text, const std::string& field, double min, double max) {
    std::string t = trim(text);
    if (t.empty()) throw ValidationError(field + " is required.");
    errno = 0;
    char* end = nullptr;
    double v = std::strtod(t.c_str(), &end);
    if (errno != 0 || end == t.c_str() || *end != '\0' || !std::isfinite(v))
        throw ValidationError(field + " must be a number.");
    if (v < min || v > max) throw ValidationError(field + " is out of the allowed range.");
    return v;
}

}  // namespace lms::validation
