#include "fast_trigger/sta_lta.hpp"

#include <gtest/gtest.h>

#include <cmath>
#include <limits>
#include <span>
#include <vector>

namespace {

TEST(StaLta, RejectsEmptyInput) {
    const std::vector<double> input;
    EXPECT_THROW(fast_trigger::compute_sta_lta(std::span<const double>(input), 4, 20),
                 std::invalid_argument);
}

TEST(StaLta, RejectsInvalidWindowLengths) {
    const std::vector<double> input{1.0, 2.0, 3.0, 4.0, 5.0};

    EXPECT_THROW(fast_trigger::compute_sta_lta(std::span<const double>(input), 0, 10),
                 std::invalid_argument);
    EXPECT_THROW(fast_trigger::compute_sta_lta(std::span<const double>(input), 4, 4),
                 std::invalid_argument);
}

TEST(StaLta, ReturnsFiniteValuesWithExpectedShape) {
    const std::vector<double> input{0.2, -0.5, 1.0, 2.0, -1.0, 0.1, 0.3, -0.2};
    const auto result = fast_trigger::compute_sta_lta(std::span<const double>(input), 2, 4);

    ASSERT_EQ(result.size(), input.size());
    for (double value : result) {
        EXPECT_TRUE(std::isfinite(value));
        EXPECT_GE(value, 0.0);
    }
}

TEST(StaLta, SupportsMinimumValidWindowLengths) {
    const std::vector<double> input{1.0, 2.0};
    const auto result = fast_trigger::compute_sta_lta(std::span<const double>(input), 1, 2);
    ASSERT_EQ(result.size(), input.size());
    for (double value : result) {
        EXPECT_TRUE(std::isfinite(value));
        EXPECT_GE(value, 0.0);
    }
}

TEST(StaLta, HandlesNearZeroInputWithoutNaN) {
    const std::vector<double> input(64, 1e-12);
    const auto result = fast_trigger::compute_sta_lta(std::span<const double>(input), 2, 8);
    ASSERT_EQ(result.size(), input.size());
    for (double value : result) {
        EXPECT_TRUE(std::isfinite(value));
        EXPECT_GE(value, 0.0);
    }
}

TEST(StaLtaBatch, MatchesPerTraceScalarComputation) {
    const std::size_t n_traces = 3;
    const std::size_t n_samples = 8;
    const std::vector<double> batch{
        0.2, -0.4, 0.7,  1.4, -0.9, 0.2, 0.3, -0.1,
        1.0, 0.5,  -0.5, 0.2, 0.2,  0.1, 0.0, 0.6,
        2.0, 1.8,  1.7,  1.6, 1.5,  1.4, 1.3, 1.2};

    const auto batch_result =
        fast_trigger::compute_sta_lta_batch(batch.data(), n_traces, n_samples, 2, 5);

    ASSERT_EQ(batch_result.size(), n_traces * n_samples);

    for (std::size_t trace = 0; trace < n_traces; ++trace) {
        const std::span<const double> row(batch.data() + trace * n_samples, n_samples);
        const auto scalar_result = fast_trigger::compute_sta_lta(row, 2, 5);

        for (std::size_t sample = 0; sample < n_samples; ++sample) {
            const auto idx = trace * n_samples + sample;
            EXPECT_NEAR(batch_result[idx], scalar_result[sample], 1e-12);
        }
    }
}

TEST(StaLtaBatch, RejectsInvalidBatchMetadata) {
    const std::vector<double> batch{1.0, 2.0, 3.0, 4.0};

    EXPECT_THROW(fast_trigger::compute_sta_lta_batch(nullptr, 1, 4, 2, 3),
                 std::invalid_argument);
    EXPECT_THROW(fast_trigger::compute_sta_lta_batch(batch.data(), 0, 4, 2, 3),
                 std::invalid_argument);
    EXPECT_THROW(fast_trigger::compute_sta_lta_batch(batch.data(), 1, 0, 2, 3),
                 std::invalid_argument);
}

TEST(StaLtaBatch, RejectsOverflowingDimensions) {
    const std::vector<double> sentinel{1.0};
    const size_t huge = std::numeric_limits<size_t>::max();

    EXPECT_THROW(fast_trigger::compute_sta_lta_batch(sentinel.data(), huge, 2, 1, 2),
                 std::length_error);
    EXPECT_THROW(fast_trigger::compute_sta_lta_batch(sentinel.data(), 2, huge, 1, 2),
                 std::length_error);
}

}  // namespace
