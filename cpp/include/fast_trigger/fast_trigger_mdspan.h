#pragma once

#include <cstddef>

#if defined(__cpp_constexpr) && (__cpp_constexpr >= 202306L)
#define FAST_TRIGGER_HAS_CPP26_CONSTEXPR 1
#else
#define FAST_TRIGGER_HAS_CPP26_CONSTEXPR 0
#endif

#if defined(__has_include)
#if __has_include(<mdspan>) && defined(__cpp_lib_mdspan) && (__cpp_lib_mdspan >= 202207L)
#include <mdspan>
#define FAST_TRIGGER_MDSPAN_BACKEND_STD 1
namespace fast_trigger_mdspan_ns = std;
#elif __has_include(<experimental/mdspan>)
#include <experimental/mdspan>
#define FAST_TRIGGER_MDSPAN_BACKEND_STD 0
namespace fast_trigger_mdspan_ns = std::experimental;
#elif __has_include(<mdspan/mdspan.hpp>)
#include <mdspan/mdspan.hpp>
#define FAST_TRIGGER_MDSPAN_BACKEND_STD 0
namespace fast_trigger_mdspan_ns = Kokkos::Experimental;
#else
#error "No mdspan implementation found. Install standard library mdspan or kokkos/mdspan."
#endif
#else
#include <mdspan/mdspan.hpp>
#define FAST_TRIGGER_MDSPAN_BACKEND_STD 0
namespace fast_trigger_mdspan_ns = Kokkos::Experimental;
#endif

namespace fast_trigger::compat {

using index_type = std::size_t;
using matrix_extents =
    fast_trigger_mdspan_ns::extents<index_type,
                                    fast_trigger_mdspan_ns::dynamic_extent,
                                    fast_trigger_mdspan_ns::dynamic_extent>;

template <typename T>
using matrix_view = fast_trigger_mdspan_ns::mdspan<T, matrix_extents, fast_trigger_mdspan_ns::layout_right>;

}  // namespace fast_trigger::compat
