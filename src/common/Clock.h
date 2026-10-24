#pragma once
#include <chrono>

#include "common/DateTime.h"

namespace lms {

/// Time source abstraction: lets tests fix "now" to exercise deadline boundaries.
class IClock {
public:
    virtual ~IClock() = default;
    virtual DateTime now() const = 0;
};

class SystemClock final : public IClock {
public:
    DateTime now() const override {
        using namespace std::chrono;
        return DateTime::fromEpochSeconds(
            duration_cast<seconds>(system_clock::now().time_since_epoch()).count());
    }
};

class FixedClock final : public IClock {
public:
    explicit FixedClock(DateTime t) : t_(t) {}
    DateTime now() const override { return t_; }
    void set(DateTime t) { t_ = t; }
    void advanceSeconds(std::int64_t s) { t_ = t_.addSeconds(s); }

private:
    DateTime t_;
};

}  // namespace lms
