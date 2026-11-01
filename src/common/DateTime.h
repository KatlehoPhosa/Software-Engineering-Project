#pragma once
#include <cstdint>
#include <string>

namespace lms {

/// A UTC instant with one-second resolution.
/// Stored in the database as TEXT 'YYYY-MM-DD HH:MM:SS' (data dictionary).
/// Implemented without timegm()/gmtime() so behaviour is identical on every platform.
class DateTime {
public:
    static constexpr std::int64_t kSecondsPerDay = 86400;

    DateTime() = default;
    static DateTime fromEpochSeconds(std::int64_t s) { return DateTime(s); }
    static DateTime fromParts(int year, int month, int day, int hour = 0, int minute = 0, int second = 0);

    /// Strict database form 'YYYY-MM-DD HH:MM:SS'. Throws ValidationError.
    static DateTime parse(const std::string& text);
    /// Console form: 'YYYY-MM-DD HH:MM' or 'YYYY-MM-DD HH:MM:SS'. Throws ValidationError.
    static DateTime parseFlexible(const std::string& text);

    std::string toString() const;  ///< 'YYYY-MM-DD HH:MM:SS'
    std::int64_t epochSeconds() const { return secs_; }

    DateTime addSeconds(std::int64_t s) const { return DateTime(secs_ + s); }
    DateTime addDays(int d) const { return DateTime(secs_ + static_cast<std::int64_t>(d) * kSecondsPerDay); }

    friend bool operator==(DateTime a, DateTime b) { return a.secs_ == b.secs_; }
    friend bool operator!=(DateTime a, DateTime b) { return a.secs_ != b.secs_; }
    friend bool operator<(DateTime a, DateTime b) { return a.secs_ < b.secs_; }
    friend bool operator<=(DateTime a, DateTime b) { return a.secs_ <= b.secs_; }
    friend bool operator>(DateTime a, DateTime b) { return a.secs_ > b.secs_; }
    friend bool operator>=(DateTime a, DateTime b) { return a.secs_ >= b.secs_; }

private:
    explicit DateTime(std::int64_t s) : secs_(s) {}
    std::int64_t secs_ = 0;
};

}  // namespace lms
