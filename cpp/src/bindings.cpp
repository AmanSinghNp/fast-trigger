#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>
#include <nanobind/stl/vector.h>

#include "fast_trigger/sta_lta.hpp"

namespace nb = nanobind;
using namespace nb::literals;

namespace {

inline void validate_windows(size_t sta_len, size_t lta_len) {
    if (sta_len == 0 || lta_len == 0) {
        throw nb::value_error("Window lengths must be > 0");
    }
    if (sta_len >= lta_len) {
        throw nb::value_error("STA length must be < LTA length");
    }
}

}  // namespace

NB_MODULE(_fast_trigger_impl, m) {
    m.doc() = "Fast STA/LTA implementation in C++23";

    // 1D version - FIXED memory ownership
    m.def("compute_sta_lta",
        [](nb::ndarray<const double, nb::shape<-1>, nb::c_contig> data,
           size_t sta_len,
           size_t lta_len) -> nb::ndarray<nb::numpy, double> {
            if (data.shape(0) == 0) {
                throw nb::value_error("Input array cannot be empty");
            }
            validate_windows(sta_len, lta_len);

            std::span<const double> input_span(data.data(), data.shape(0));
            std::vector<double> result;
            {
                nb::gil_scoped_release release;
                result = fast_trigger::compute_sta_lta(input_span, sta_len, lta_len);
            }

            // Own result in heap-allocated vector; ndarray points into it, capsule deletes vector
            auto* vec_ptr = new std::vector<double>(std::move(result));
            size_t shape[1] = {vec_ptr->size()};
            double* data_ptr = vec_ptr->data();
            return nb::ndarray<nb::numpy, double>(
                data_ptr,
                1,
                shape,
                nb::capsule(vec_ptr, [](void* p) noexcept {
                    delete static_cast<std::vector<double>*>(p);
                })
            );
        },
        "data"_a, "sta_len"_a, "lta_len"_a,
        "Compute STA/LTA ratio for 1D seismic trace"
    );

    // 2D batch version - FIXED memory ownership
    m.def("compute_sta_lta_batch",
        [](nb::ndarray<const double, nb::shape<-1, -1>, nb::c_contig> data,
           size_t sta_len,
           size_t lta_len) -> nb::ndarray<nb::numpy, double> {

            size_t n_traces = data.shape(0);
            size_t n_samples = data.shape(1);
            if (n_traces == 0 || n_samples == 0) {
                throw nb::value_error("Batch dimensions must be > 0");
            }
            validate_windows(sta_len, lta_len);

            std::vector<double> result;
            {
                nb::gil_scoped_release release;
                result = fast_trigger::compute_sta_lta_batch(
                    data.data(), n_traces, n_samples, sta_len, lta_len
                );
            }

            // Own result in heap-allocated vector; ndarray points into it, capsule deletes vector
            auto* vec_ptr = new std::vector<double>(std::move(result));
            size_t shape[2] = {n_traces, n_samples};
            double* data_ptr = vec_ptr->data();
            return nb::ndarray<nb::numpy, double>(
                data_ptr,
                2,
                shape,
                nb::capsule(vec_ptr, [](void* p) noexcept {
                    delete static_cast<std::vector<double>*>(p);
                })
            );
        },
        "data"_a, "sta_len"_a, "lta_len"_a,
        "Compute STA/LTA for 2D batch of traces"
    );
}
