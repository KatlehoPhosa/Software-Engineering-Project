#include "domain/PenaltyStrategy.h"

#include <algorithm>
#include <cmath>
#include <sstream>

#include "common/Exceptions.h"

namespace lms {
namespace {

void checkArgs(double rawMark, int daysLate) {
    if (!(rawMark >= 0.0) || !std::isfinite(rawMark)) throw ValidationError("Raw mark must be a non-negative number.");
    if (daysLate < 0) throw ValidationError("Days late must not be negative.");
}

double round2(double v) { return std::round(v * 100.0) / 100.0; }

}  // namespace

DailyPercentPenaltyStrategy::DailyPercentPenaltyStrategy(double percentPerDay, int maxDaysLate)
    : percentPerDay_(percentPerDay), maxDaysLate_(maxDaysLate) {
    if (percentPerDay < 0.0 || percentPerDay > 100.0 || maxDaysLate < 0)
        throw ValidationError("Invalid penalty parameters.");
}

double DailyPercentPenaltyStrategy::apply(double rawMark, int daysLate) const {
    checkArgs(rawMark, daysLate);
    if (daysLate == 0) return round2(rawMark);
    if (daysLate > maxDaysLate_) return 0.0;
    const double keep = std::max(0.0, 1.0 - (percentPerDay_ * daysLate) / 100.0);
    return round2(rawMark * keep);
}

std::string DailyPercentPenaltyStrategy::describe() const {
    std::ostringstream o;
    o << percentPerDay_ << "% per day late for up to " << maxDaysLate_ << " days, then zero";
    return o.str();
}

FixedPenaltyStrategy::FixedPenaltyStrategy(double percent, int maxDaysLate)
    : percent_(percent), maxDaysLate_(maxDaysLate) {
    if (percent < 0.0 || percent > 100.0 || maxDaysLate < 0) throw ValidationError("Invalid penalty parameters.");
}

double FixedPenaltyStrategy::apply(double rawMark, int daysLate) const {
    checkArgs(rawMark, daysLate);
    if (daysLate == 0) return round2(rawMark);
    if (daysLate > maxDaysLate_) return 0.0;
    return round2(rawMark * (1.0 - percent_ / 100.0));
}

std::string FixedPenaltyStrategy::describe() const {
    std::ostringstream o;
    o << "flat " << percent_ << "% off if late (up to " << maxDaysLate_ << " days), then zero";
    return o.str();
}

double LatePenaltyToZeroStrategy::apply(double rawMark, int daysLate) const {
    checkArgs(rawMark, daysLate);
    return daysLate == 0 ? round2(rawMark) : 0.0;
}

std::string LatePenaltyToZeroStrategy::describe() const { return "zero for any late work"; }

}  // namespace lms
