
# Project Roadmap: fast-trigger (2026 Modern Edition - Real World)

**A High-Performance C++23/26 Extension for Seismic Event Detection**

## 1. Executive Summary

**Goal:** Build a Python library that implements the Recursive STA/LTA algorithm using a C++23 backend (with experimental C++26 features).
**Modern Tech Stack:**

*   **Language:** C++23 Stable (Baseline). **C++26** is experimental opt-in via CMake 3.31+.
*   **Bindings:** `nanobind` (v2.x) with Stable ABI (Python 3.12+) and Free-Threaded options.
*   **Build System:** `scikit-build-core` + `CMake 3.31+` (Leveraging newer modules).
*   **CI/CD:** `cibuildwheel` v3.3.1+ on `ubuntu-24.04` and latest runners.
*   **Target Performance:** >50x speedup over pure Python, deterministic benchmarking via `asv`.

---

## 2. Phase 1: The C++23 Core Engine (with C++26 Opt-in)

*Focus: Modern features with real-world portability.*

### Task 1.0: Standard Library Feature Gating

*   **Action:** Create `fast_trigger_mdspan.h` compatibility layer.
*   **Strategy:**
    *   Prefer `std::mdspan` (C++23) when available (MSVC 19.40+, GCC 14+, Clang 18+).
    *   Fallback to vendor `kokkos/mdspan` implementation for broader compatibility.
    *   **C++26 Features:** Use only where compiler detection passes (e.g., `__cpp_constexpr >= 202306L`).
*   **CI Matrix:**
    *   **Primary:** C++23 mode (production-ready).
    *   **Experimental:** C++26 mode (opt-in via CMake flag).

### Task 1.2: The `StaLta` Kernel (Batch Processing)

*   **Action:** Implement the processing loop supporting **2D Inputs** `(n_traces, n_samples)`.
*   **Optimization:**
    *   **OpenMP 5.0/5.1:** Use `#pragma omp parallel for simd` (universally supported).
    *   **Architecture Dispatch:** Optional OpenMP 5.2 metadirectives.
    *   **Avoid:** OpenMP 6.0 features (too new for production).
    *   Use a macro for restricted pointers (e.g., `__restrict` extension) combined with `std::assume_aligned`.

### Task 1.3: GoogleTest Integration

*   **Action:** Use `FetchContent` to pull from `main` branch.
*   **Why:** GoogleTest v1.15.x tags are frozen; `main` contains critical C++23/26 fixes.

```cmake
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG main
)
```

### Phase 1 Acceptance Criteria

*   [ ] Builds with standard C++23 flags on Windows, Linux, and macOS.
*   [ ] Fallback logic correctly handles missing `<mdspan>` without user intervention.
*   [ ] Unit tests pass using **GoogleTest (main)**.

---

## 3. Phase 2: `nanobind` & The Build Backend

*Focus: The "Next-Gen" Python Binding Layer.*

### Task 2.1: `nanobind` Integration

*   **Action:** Use `FetchContent` for `nanobind` (v2.7.0+ or latest).
*   **Configuration:** Enable `STABLE_ABI` option for Python 3.12+ builds.

### Task 2.2: Packaging with `scikit-build-core`

*   **Action:** Use `pyproject.toml` with `[tool.scikit-build]` configuration.
*   **Settings:**
    *   `cmake.minimum-version = "3.31"` (To enable C++26 features natively).
    *   `sdist.include` patterns for reproducible source distributions.

### Phase 2 Acceptance Criteria

*   [ ] `pip install .` works reliably in clean environments.
*   [ ] `import fast_trigger` works on Python 3.12, 3.13, and **3.14 (Stable)**.
*   [ ] `nb::ndarray` enforces types safely.

---

## 4. Phase 3: Robustness & Sanitizers

*Focus: Catching UB and Leaks.*

### Task 3.1: Hypothesis Property Testing

*   **Action:** Fuzz C++ code from Python using `hypothesis` (NaNs, Infs, empty arrays).

### Task 3.2: Sanitizer CI Jobs

*   **Action:** Run tests under **AddressSanitizer (ASan)** and **UndefinedBehaviorSanitizer (UBSan)** on Linux.
*   **Action:** Use **LeakSanitizer** (LSan) to prove zero leaks in the C++ layer.
*   **C++26 Checks:** Add specific checks for enhanced `constexpr` and verified structured bindings if experimental mode is on.

### Phase 3 Acceptance Criteria

*   [ ] Sanitizer jobs pass with zero errors in CI.
*   [ ] Fuzzing producing valid/invalid inputs never crashes the interpreter.

---

## 5. Phase 4: CI/CD Pipeline (2026 Standards)

*Focus: Automation & Modern Wheel Standards.*

### Task 5.1: GitHub Actions with `cibuildwheel` (v3.x)

*   **Action:** Use `pypa/cibuildwheel@v3.3.1+`.
*   **Runners:** Use `ubuntu-24.04`, `macos-15`, `windows-2025` (if generally available).
*   **Matrix:**
    *   Standard CPython wheels: Python 3.12, 3.13, **3.14**.
    *   **Stable ABI** wheels: Python 3.12+ (single wheel).
    *   **Free-Threaded** wheels: Python 3.13t, 3.14t.
        *   *Note:* Ensure dependencies (e.g., NumPy) support nogil before building.

### Task 5.2: Automatic Publishing

*   **Action:** Configure "Trusted Publishing" (OIDC) for PyPI.

### Phase 4 Acceptance Criteria

*   [ ] CI builds all target wheel variants (standard, stable, free-threaded).
*   [ ] Import test step validates every built wheel.

---

## 6. Phase 5: Documentation & System Design

*Focus: The "Senior Engineer" Polish.*

### Task 6.1: Reproducible Benchmarking

*   **Action:** Use **Airspeed Velocity (`asv`)** or `pyperf` to define benchmarks.
*   **Scenarios:** Pure Python vs NumPy vs fast-trigger (1D) vs fast-trigger (2D Batch).
*   **Output:** JSON results and a rendered plot for the README.

### Task 6.2: Documentation Updates

*   **Action:** Add "Compatibility" section explaining `mdspan` fallback.
*   **Action:** Add "Performance" badge linking to reproducible benchmark reports.

### Task 6.3: Compatibility Matrix

*   **Action:** Document feature support in `README.md`.

| Compiler       | C++23 | C++26 (Exp) | std::mdspan | OpenMP |
|:---------------|:------|:------------|:------------|:-------|
| GCC 14         | ✅    | 🔶          | ✅          | 5.2    |
| GCC 15         | ✅    | ✅          | ✅          | 5.2+6.0|
| Clang 18       | ✅    | 🔶          | ✅          | 5.1    |
| Clang 21       | ✅    | ✅          | ✅          | 5.2    |
| MSVC 19.40+    | ✅    | 🔶          | ✅          | 5.2    |

🔶 = Partial support with feature detection.

### Phase 5 Acceptance Criteria

*   [ ] README accurately reflects compatibility constraints.
*   [ ] Benchmarks can be re-run by any contributor with a single command.