#pragma once
#include <span>
#include <vector>
#include <cstddef>
#include <stdexcept>

namespace fast_trigger {

/**
 * Compute recursive STA/LTA for a single trace
 *
 * @param data Input seismic trace (length n)
 * @param sta_len Short-term average window length
 * @param lta_len Long-term average window length
 * @return STA/LTA ratio time series
 */
std::vector<double> compute_sta_lta(
    std::span<const double> data,
    size_t sta_len,
    size_t lta_len
);

/**
 * Batch process multiple traces (2D array)
 *
 * @param data Pointer to contiguous 2D array (n_traces × n_samples)
 * @param n_traces Number of traces
 * @param n_samples Samples per trace
 * @param sta_len STA window
 * @param lta_len LTA window
 * @return Flattened output array (n_traces × n_samples)
 */
std::vector<double> compute_sta_lta_batch(
    const double* data,
    size_t n_traces,
    size_t n_samples,
    size_t sta_len,
    size_t lta_len
);

} // namespace fast_trigger
