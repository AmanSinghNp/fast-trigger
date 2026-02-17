cmake_minimum_required(VERSION 3.31)
project(fast_trigger LANGUAGES CXX)

option(FAST_TRIGGER_ENABLE_CPP26 "Build with C++26 (experimental)" OFF)
option(FAST_TRIGGER_ENABLE_SANITIZERS "Enable ASan/UBSan instrumentation (Linux GCC/Clang)" OFF)
option(FAST_TRIGGER_STRICT_FLOAT "Use strict IEEE floating-point (no -ffast-math)" OFF)
option(FAST_TRIGGER_ENABLE_NATIVE_OPT "Enable CPU-specific tuning flags (e.g. -march=native)" OFF)

# C++23 baseline with optional C++26 mode.
if(FAST_TRIGGER_ENABLE_CPP26)
    set(CMAKE_CXX_STANDARD 26)
else()
    set(CMAKE_CXX_STANDARD 23)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# CRITICAL: Force Release mode on Windows (avoids debug/release mismatch)
if(WIN32 AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Find Python BEFORE nanobind (required for FetchContent method)
find_package(Python REQUIRED COMPONENTS Interpreter Development.Module)
find_package(Python QUIET COMPONENTS Development.SABIModule)

# Fetch nanobind using FetchContent (official method for Windows)
include(FetchContent)
set(FAST_TRIGGER_NANOBIND_GIT_TAG "44ad9a9e5729abda24ef8dc9d76233d801e651e9" CACHE STRING "nanobind git tag/commit")
set(FAST_TRIGGER_MDSPAN_GIT_TAG "96b13fbc66c1d53b3738246e54212ea3098213a9" CACHE STRING "kokkos/mdspan git tag/commit")
set(FAST_TRIGGER_GTEST_GIT_TAG "77f6bd3e75a2e7d3c67a024893417de42f58c418" CACHE STRING "googletest git tag/commit")

FetchContent_Declare(
    nanobind
    GIT_REPOSITORY https://github.com/wjakob/nanobind.git
    GIT_TAG ${FAST_TRIGGER_NANOBIND_GIT_TAG}
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(nanobind)

# Header-only mdspan fallback used when std::mdspan is unavailable.
FetchContent_Declare(
    kokkos_mdspan
    GIT_REPOSITORY https://github.com/kokkos/mdspan.git
    GIT_TAG ${FAST_TRIGGER_MDSPAN_GIT_TAG}
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(kokkos_mdspan)

if(FAST_TRIGGER_MDSPAN_GIT_TAG STREQUAL "stable")
    message(WARNING "FAST_TRIGGER_MDSPAN_GIT_TAG is floating ('stable'); reproducibility is reduced.")
endif()

find_package(OpenMP QUIET)

set(FAST_TRIGGER_SANITIZER_FLAGS "")
if(FAST_TRIGGER_ENABLE_SANITIZERS)
    if(MSVC)
        message(WARNING "FAST_TRIGGER_ENABLE_SANITIZERS is not supported on MSVC; option ignored.")
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        list(APPEND FAST_TRIGGER_SANITIZER_FLAGS
            -fsanitize=address,undefined
            -fno-omit-frame-pointer
        )
    else()
        message(WARNING "FAST_TRIGGER_ENABLE_SANITIZERS requested on unsupported compiler: ${CMAKE_CXX_COMPILER_ID}")
    endif()
endif()

function(fast_trigger_enable_sanitizers target_name)
    if(FAST_TRIGGER_SANITIZER_FLAGS)
        target_compile_options(${target_name} PRIVATE ${FAST_TRIGGER_SANITIZER_FLAGS})
        target_link_options(${target_name} PRIVATE ${FAST_TRIGGER_SANITIZER_FLAGS})
    endif()
endfunction()

# Core C++ library
add_library(fast_trigger_core STATIC
    cpp/src/sta_lta.cpp
)
fast_trigger_enable_sanitizers(fast_trigger_core)

target_include_directories(fast_trigger_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/cpp/include
    ${kokkos_mdspan_SOURCE_DIR}/include
)

if(FAST_TRIGGER_ENABLE_CPP26)
    target_compile_definitions(fast_trigger_core PUBLIC FAST_TRIGGER_ENABLE_CPP26=1)
endif()

# Windows-specific: Position Independent Code for static lib
set_target_properties(fast_trigger_core PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

# Optimizations (fixed for Windows slow build times)
if(MSVC)
    # Use /O2 instead of /Os to avoid 3-hour build times on Windows
    target_compile_options(fast_trigger_core PRIVATE /O2 /GL)
    # Link-Time Code Generation
    set_target_properties(fast_trigger_core PROPERTIES 
        INTERPROCEDURAL_OPTIMIZATION TRUE
    )
else()
    set(FAST_TRIGGER_ARCH_FLAGS "")
    if(FAST_TRIGGER_ENABLE_NATIVE_OPT)
        list(APPEND FAST_TRIGGER_ARCH_FLAGS -march=native)
    endif()

    if(FAST_TRIGGER_STRICT_FLOAT)
        target_compile_options(fast_trigger_core PRIVATE -O3 ${FAST_TRIGGER_ARCH_FLAGS})
    else()
        target_compile_options(fast_trigger_core PRIVATE
            -O3 ${FAST_TRIGGER_ARCH_FLAGS} -ffast-math
        )
    endif()
endif()

if(OpenMP_CXX_FOUND)
    target_link_libraries(fast_trigger_core PUBLIC OpenMP::OpenMP_CXX)
    target_compile_definitions(fast_trigger_core PUBLIC FAST_TRIGGER_HAS_OPENMP=1)
endif()

# Python bindings module
set(FAST_TRIGGER_NANOBIND_MODULE_FLAGS NB_STATIC)
if(Python_VERSION VERSION_GREATER_EQUAL "3.12" AND TARGET Python::SABIModule)
    list(APPEND FAST_TRIGGER_NANOBIND_MODULE_FLAGS STABLE_ABI)
endif()

nanobind_add_module(
    _fast_trigger_impl
    ${FAST_TRIGGER_NANOBIND_MODULE_FLAGS}  # Static nanobind + optional stable ABI
    cpp/src/bindings.cpp
)
fast_trigger_enable_sanitizers(_fast_trigger_impl)

target_link_libraries(_fast_trigger_impl PRIVATE fast_trigger_core)

# Install to Python package directory
install(TARGETS _fast_trigger_impl 
    LIBRARY DESTINATION fast_trigger
    RUNTIME DESTINATION fast_trigger  # For Windows .pyd files
)

include(CTest)
if(BUILD_TESTING)
    enable_testing()

    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG ${FAST_TRIGGER_GTEST_GIT_TAG}
    )
    FetchContent_MakeAvailable(googletest)

    if(FAST_TRIGGER_GTEST_GIT_TAG STREQUAL "main")
        message(WARNING "FAST_TRIGGER_GTEST_GIT_TAG is floating ('main'); reproducibility is reduced.")
    endif()

    add_executable(fast_trigger_core_tests
        tests/cpp/sta_lta_test.cpp
    )
    fast_trigger_enable_sanitizers(fast_trigger_core_tests)
    target_link_libraries(fast_trigger_core_tests PRIVATE
        fast_trigger_core
        GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(fast_trigger_core_tests)
endif()
