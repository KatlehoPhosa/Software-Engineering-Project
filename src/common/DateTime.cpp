#include "common/DateTime.h"

#include <cstdio>

#include "common/Exceptions.h"

namespace lms {
namespace {

// Howard Hinnant's civil-date algorithms (public domain).
std::int64_t daysFromCivil(int y, unsigned m, unsigned d) {
    y -= m <= 2;
    const std::int64_t era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(y - era * 400);
    const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + static_cast<std::int64_t>(doe) - 719468;
}

void civilFromDays(std::int64_t z, int& y, unsigned& m, unsigned& d) {
    z += 719468;
    const std::int64_t era = (z >= 0 ? z : z - 146096) / 146097;
    const unsigned doe = static_cast<unsigned>(z - era * 146097);
    const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    const std::int64_t yy = static_cast<std::int64_t>(yoe) + era * 400;
    const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    const unsigned mp = (5 * doy + 2) / 153;
    d = doy - (153 * mp + 2) / 5 + 1;
    m = mp < 10 ? mp + 3 : mp - 9;
    y = static_cast<int>(yy + (m <= 2));
}

bool isLeap(int y) { return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0; }

int daysInMonth(int y, int m) {
    static const int kDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    return (m == 2 && isLeap(y)) ? 29 : kDays[m - 1];
}

}  // namespace

DateTime DateTime::fromParts(int year, int month, int day, int hour, int minute, int second) {
    if (year < 1970 || year > 9999 || month < 1 || month > 12 || day < 1 ||
        day > daysInMonth(year, month) || hour < 0 || hour > 23 || minute < 0 || minute > 59 ||
        second < 0 || second > 59) {
        throw ValidationError("Invalid date/time value.");
    }
    const std::int64_t days = daysFromCivil(year, static_cast<unsigned>(month), static_cast<unsigned>(day));
    return DateTime(days * kSecondsPerDay + hour * 3600 + minute * 60 + second);
}

DateTime DateTime::parse(const std::string& text) {
    int y, mo, d, h, mi, s;
    char tail;
    if (std::sscanf(text.c_str(), "%4d-%2d-%2d %2d:%2d:%2d%c", &y, &mo, &d, &h, &mi, &s, &tail) != 6 ||
        text.size() != 19) {
        throw ValidationError("Date/time must look like YYYY-MM-DD HH:MM:SS.");
    }
    return fromParts(y, mo, d, h, mi, s);
}

DateTime DateTime::parseFlexible(const std::string& text) {
    if (text.size() == 19) return parse(text);
    int y, mo, d, h, mi;
    char tail;
    if (text.size() != 16 ||
        std::sscanf(text.c_str(), "%4d-%2d-%2d %2d:%2d%c", &y, &mo, &d, &h, &mi, &tail) != 5) {
        throw ValidationError("Date/time must look like YYYY-MM-DD HH:MM.");
    }
    return fromParts(y, mo, d, h, mi, 0);
}

std::string DateTime::toString() const {
    std::int64_t days = secs_ / kSecondsPerDay;
    std::int64_t rem = secs_ % kSecondsPerDay;
    if (rem < 0) {
        rem += kSecondsPerDay;
        --days;
    }
    int y;
    unsigned m, d;
    civilFromDays(days, y, m, d);
    char buf[32];
    std::snprintf(buf, sizeof buf, "%04d-%02u-%02u %02d:%02d:%02d", y, m, d,
                  static_cast<int>(rem / 3600), static_cast<int>((rem % 3600) / 60),
                  static_cast<int>(rem % 60));
    return buf;
}

}  // namespace lms
