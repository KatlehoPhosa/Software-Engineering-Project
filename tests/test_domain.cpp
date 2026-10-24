#include "TestSupport.h"
#include "common/Validation.h"
#include "domain/Entities.h"

using namespace lms;

// ---------------------------------------------------------------- DateTime
TEST(DateTime, RoundTripsAndOrdersAsText) {
    auto t = DateTime::parse("2026-02-28 23:59:59");
    EXPECT_EQ(t.toString(), "2026-02-28 23:59:59");
    EXPECT_EQ(t.addSeconds(1).toString(), "2026-03-01 00:00:00");
    EXPECT_EQ(DateTime::parse("2028-02-29 00:00:00").addDays(1).toString(), "2028-03-01 00:00:00");  // leap year
}
TEST(DateTime, RejectsInvalid) {
    for (const char* bad : {"", "2026-13-01 00:00:00", "2026-02-30 00:00:00", "2026-01-01 24:00:00",
                            "2026-01-01", "garbage", "2026-01-01 10:00:00x"})
        EXPECT_THROW(DateTime::parse(bad), ValidationError) << bad;
    EXPECT_NO_THROW(DateTime::parseFlexible("2026-05-01 09:30"));
    EXPECT_THROW(DateTime::parseFlexible("2026-05-01 9:30"), ValidationError);
}

// ---------------------------------------------------------------- late days (boundary value analysis)
TEST(LateDays, Boundaries) {
    const DateTime d = DateTime::fromParts(2026, 11, 10, 12);
    EXPECT_EQ(lateDays(d.addSeconds(-1), d), 0);
    EXPECT_EQ(lateDays(d, d), 0);                    // exactly at the deadline is on time
    EXPECT_EQ(lateDays(d.addSeconds(1), d), 1);
    EXPECT_EQ(lateDays(d.addDays(1), d), 1);
    EXPECT_EQ(lateDays(d.addDays(1).addSeconds(1), d), 2);
    EXPECT_EQ(lateDays(d.addDays(3), d), 3);
    EXPECT_EQ(lateDays(d.addDays(3).addSeconds(1), d), 4);
}

// ---------------------------------------------------------------- penalty strategies
TEST(Penalty, DailyPercentMatchesHodRule) {
    DailyPercentPenaltyStrategy s;  // 10 % per day, 3 days, then zero
    EXPECT_DOUBLE_EQ(s.apply(20, 0), 20.0);
    EXPECT_DOUBLE_EQ(s.apply(20, 1), 18.0);
    EXPECT_DOUBLE_EQ(s.apply(20, 2), 16.0);
    EXPECT_DOUBLE_EQ(s.apply(20, 3), 14.0);
    EXPECT_DOUBLE_EQ(s.apply(20, 4), 0.0);
    EXPECT_DOUBLE_EQ(s.apply(0, 2), 0.0);
    EXPECT_DOUBLE_EQ(s.apply(17.5, 1), 15.75);
}
TEST(Penalty, ContractHoldsForEveryStrategy) {  // Liskov: result within [0, raw], 0 days unchanged
    std::vector<std::unique_ptr<IPenaltyStrategy>> all;
    all.push_back(std::make_unique<DailyPercentPenaltyStrategy>());
    all.push_back(std::make_unique<FixedPenaltyStrategy>(25));
    all.push_back(std::make_unique<LatePenaltyToZeroStrategy>());
    for (auto& s : all) {
        for (double raw : {0.0, 1.0, 13.37, 20.0})
            for (int d = 0; d <= 6; ++d) {
                const double r = s->apply(raw, d);
                EXPECT_GE(r, 0.0); EXPECT_LE(r, raw);
                if (d == 0) {
                    EXPECT_DOUBLE_EQ(r, raw);
                }
            }
        EXPECT_THROW(s->apply(-1, 0), ValidationError);
        EXPECT_THROW(s->apply(5, -1), ValidationError);
        EXPECT_FALSE(s->describe().empty());
    }
    EXPECT_DOUBLE_EQ(FixedPenaltyStrategy(25).apply(20, 3), 15.0);
    EXPECT_DOUBLE_EQ(LatePenaltyToZeroStrategy().apply(20, 1), 0.0);
    EXPECT_THROW(DailyPercentPenaltyStrategy(150, 3), ValidationError);
}

// ---------------------------------------------------------------- submission / mark life cycle
TEST(SubmissionLifecycle, OnTimeLateResubmit) {
    const DateTime dl = DateTime::fromParts(2026, 11, 10, 12);
    auto onTime = Submission::createNew(1, 1, dl, dl, "a");
    EXPECT_EQ(onTime.state(), SubmissionState::Submitted);
    auto late = Submission::createNew(1, 1, dl.addSeconds(1), dl, "a");
    EXPECT_EQ(late.state(), SubmissionState::Late);

    onTime.resubmit(dl.addSeconds(-5), dl, "b");
    EXPECT_EQ(onTime.state(), SubmissionState::Resubmitted);
    EXPECT_EQ(onTime.fileRef(), "b");
    EXPECT_THROW(late.resubmit(dl.addSeconds(2), dl, "c"), BusinessRuleError);
    onTime.markAsMarked();
    EXPECT_THROW(onTime.resubmit(dl.addSeconds(-1), dl, "d"), BusinessRuleError);
}
TEST(MarkLifecycle, OnlyLegalTransitions) {
    Mark m = Mark::provisional(1, 15, "ok");
    EXPECT_THROW(m.moderate(), BusinessRuleError);   // no final mark yet
    m.applyPenalty(DailyPercentPenaltyStrategy(), 1);
    EXPECT_DOUBLE_EQ(*m.finalMark(), 13.5);
    EXPECT_THROW(m.release(), BusinessRuleError);    // must be moderated first
    m.moderate();
    EXPECT_THROW(m.moderate(), BusinessRuleError);
    EXPECT_THROW(m.remark(10, "x"), BusinessRuleError);
    EXPECT_THROW(m.adjustFinal(5), BusinessRuleError);
    m.release();
    EXPECT_EQ(m.state(), MarkState::Released);
    EXPECT_THROW(m.release(), BusinessRuleError);
}
TEST(MarkLifecycle, CombinedStages) {
    Submission s(1, 1, 1, DateTime(), "f", SubmissionState::Late);
    EXPECT_EQ(lifecycleOf(nullptr, nullptr), LifecycleStage::NotSubmitted);
    EXPECT_EQ(lifecycleOf(&s, nullptr), LifecycleStage::Late);
    Mark m = Mark::provisional(1, 5, "f");
    EXPECT_EQ(lifecycleOf(&s, &m), LifecycleStage::Marked);
    m.applyPenalty(LatePenaltyToZeroStrategy(), 0); m.moderate();
    EXPECT_EQ(lifecycleOf(&s, &m), LifecycleStage::Moderated);
    m.release();
    EXPECT_EQ(lifecycleOf(&s, &m), LifecycleStage::Released);
}

// ---------------------------------------------------------------- validation
TEST(Validation, Rules) {
    EXPECT_THROW(validation::requireNonBlank("   ", "x"), ValidationError);
    EXPECT_EQ(validation::requireNonBlank("  hi ", "x"), "hi");
    EXPECT_THROW(validation::requireUsername("a b"), ValidationError);
    EXPECT_THROW(validation::requireUsername("ab"), ValidationError);
    EXPECT_THROW(validation::requirePassword("short", "u"), ValidationError);
    EXPECT_THROW(validation::requirePassword("SameName1", "samename1"), ValidationError);
    EXPECT_EQ(validation::requireCourseCode("sci101"), "SCI101");
    EXPECT_THROW(validation::requireCourseCode("sci 101"), ValidationError);
    EXPECT_EQ(validation::parseInt("42", "n", 0, 100), 42);
    EXPECT_THROW(validation::parseInt("4x", "n", 0, 100), ValidationError);
    EXPECT_THROW(validation::parseInt("101", "n", 0, 100), ValidationError);
    EXPECT_THROW(validation::parseDouble("nan", "n", 0, 1), ValidationError);
    EXPECT_DOUBLE_EQ(validation::parseDouble("2.5", "n", 0, 10), 2.5);
}
