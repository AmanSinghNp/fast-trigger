#include "fast_trigger/sta_lta.hpp"
#include "fast_trigger/fast_trigger_mdspan.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>

namespace fast_trigger {

namespace {

#if defined(_MSC_VER)
#define FAST_TRIGGER_RESTRICT __restrict
#elif defined(__GNUC__) || defined(__clang__)
#define FAST_TRIGGER_RESTRICT __restrict__
#else
#define FAST_TRIGGER_RESTRICT
#endif

inline size_t checked_multiply(size_t lhs, size_t rhs) {
    if (lhs == 0 || rhs == 0) {
        return 0;
    }

    constexpr size_t max_size = std::numeric_limits<size_t>::max();
    if (lhs > (max_size / rhs)) {
        throw std::length_error("Batch dimensions are too large");
    }
    return lhs * rhs;
}

}  // namespace

std::vector<double> compute_sta_lta(
    std::span<const double> data,
    size_t sta_len,
    size_t lta_len
) {
    const size_t n = data.size();

    if (n == 0) {
        throw std::invalid_argument("Empty input array");
    }
    if (sta_len == 0 || lta_len == 0) {
        throw std::invalid_argument("Window lengths must be > 0");
    }
    if (sta_len >= lta_len) {
        throw std::invalid_argument("STA length must be < LTA length");
    }
    if (sta_len > n || lta_len > n) {
        throw std::invalid_argument("Window lengths must not exceed trace length");
    }

    std::vector<double> ratio(n, 0.0);

    // Recursive coefficients
    const double c_sta = 1.0 / sta_len;
    const double c_lta = 1.0 / lta_len;

    double sta = 0.0;
    double lta = 0.0;

    // Initialize with first sample
    const double first_val = std::abs(data[0]);
    sta = first_val;
    lta = first_val;

    for (size_t i = 0; i < n; ++i) {
        const double abs_val = std::abs(data[i]);

        // Recursive STA/LTA update
        sta = c_sta * abs_val + (1.0 - c_sta) * sta;
        lta = c_lta * abs_val + (1.0 - c_lta) * lta;

        // Avoid division by zero
        ratio[i] = (lta > 1e-10) ? (sta / lta) : 0.0;
    }

    return ratio;
}

std::vector<double> compute_sta_lta_batch(
    const double* data,
    size_t n_traces,
    size_t n_samples,
    size_t sta_len,
    size_t lta_len
) {
    if (data == nullptr) {
        throw std::invalid_argument("Input pointer must not be null");
    }
    if (n_traces == 0 || n_samples == 0) {
        throw std::invalid_argument("Batch dimensions must be > 0");
    }
    if (sta_len == 0 || lta_len == 0) {
        throw std::invalid_argument("Window lengths must be > 0");
    }
    if (sta_len >= lta_len) {
        throw std::invalid_argument("STA length must be < LTA length");
    }
    if (sta_len > n_samples || lta_len > n_samples) {
        throw std::invalid_argument("Window lengths must not exceed number of samples per trace");
    }

    const size_t total_elements = checked_multiply(n_traces, n_samples);
    if (total_elements > std::numeric_limits<std::ptrdiff_t>::max()) {
        throw std::length_error("Batch dimensions exceed supported parallel index range");
    }

    std::vector<double> output(total_elements);

    const auto* FAST_TRIGGER_RESTRICT input_data = data;
    auto* FAST_TRIGGER_RESTRICT output_data = output.data();

    compat::matrix_view<const double> input_view(input_data, n_traces, n_samples);
    compat::matrix_view<double> output_view(output_data, n_traces, n_samples);

#if defined(FAST_TRIGGER_HAS_OPENMP)
#if defined(_MSC_VER)
#pragma omp parallel for
#else
#pragma omp parallel for simd
#endif
#endif
    for (std::ptrdiff_t i = 0; i < static_cast<std::ptrdiff_t>(n_traces); ++i) {
        const size_t trace_idx = static_cast<size_t>(i);
        const double* trace_in = &input_view[trace_idx, 0];
        double* trace_out = &output_view[trace_idx, 0];

        std::span<const double> input_span(trace_in, n_samples);
        auto result = compute_sta_lta(input_span, sta_len, lta_len);

        std::copy(result.begin(), result.end(), trace_out);
    }

    return output;
}

} // namespace fast_trigger
