#pragma once
#include <string>

namespace lms {

/// Strategy pattern for late-work penalties (HoD rule, conflict C2).
///
/// Contract shared by every implementation (Liskov): for rawMark >= 0 and daysLate >= 0
/// the result is in [0, rawMark]; daysLate == 0 returns rawMark unchanged; the result is
/// rounded to two decimal places. Negative arguments throw ValidationError.
class IPenaltyStrategy {
public:
    virtual ~IPenaltyStrategy() = default;
    virtual double apply(double rawMark, int daysLate) const = 0;
    virtual std::string describe() const = 0;
};

/// School policy: lose `percentPerDay` % for each day late, for up to `maxDaysLate`
/// days; later than that the mark is zero. Defaults: 10 % per day, 3 days.
class DailyPercentPenaltyStrategy final : public IPenaltyStrategy {
public:
    explicit DailyPercentPenaltyStrategy(double percentPerDay = 10.0, int maxDaysLate = 3);
    double apply(double rawMark, int daysLate) const override;
    std::string describe() const override;

private:
    double percentPerDay_;
    int maxDaysLate_;
};

/// Alternative policy: one flat percentage off if late at all, up to `maxDaysLate`, then zero.
class FixedPenaltyStrategy final : public IPenaltyStrategy {
public:
    explicit FixedPenaltyStrategy(double percent, int maxDaysLate = 3);
    double apply(double rawMark, int daysLate) const override;
    std::string describe() const override;

private:
    double percent_;
    int maxDaysLate_;
};

/// Alternative policy: any late work receives zero.
class LatePenaltyToZeroStrategy final : public IPenaltyStrategy {
public:
    double apply(double rawMark, int daysLate) const override;
    std::string describe() const override;
};

}  // namespace lms
