# fast-trigger

High-performance seismic event detection using C++23.

## Project Status

- **Maturity:** Alpha (`0.1.0`)
- **Audience:** Science/Research
- **Python:** 3.12+
- **Platform support:** Linux, macOS, Windows

## Installation

Install from PyPI (recommended):

```bash
pip install fast-trigger
```

Install from a local source checkout:

```bash
pip install .
```

## Quickstart

```python
import numpy as np
import fast_trigger

data = np.random.randn(1000)
sta_lta = fast_trigger.compute_sta_lta(data, sta_len=10, lta_len=100)

batch = np.vstack([data, data * 0.5])
batch_sta_lta = fast_trigger.compute_sta_lta_batch(batch, sta_len=10, lta_len=100)

assert sta_lta.shape == data.shape
assert batch_sta_lta.shape == batch.shape
assert np.all(np.isfinite(sta_lta))
assert np.all(np.isfinite(batch_sta_lta))
```

## Development

Install editable mode with development and test dependencies:

```bash
pip install -e ".[dev,test,benchmark]"
```

Run local checks:

```bash
ruff check src tests
ruff format --check src tests
mypy
pytest -q tests
```

## API Behavior Notes

- `compute_sta_lta(data, sta_len=..., lta_len=...)` expects a **1D** numeric array.
- `compute_sta_lta_batch(data, sta_len=..., lta_len=...)` expects a **2D** numeric array with shape `(n_traces, n_samples)`.
- Window rules: `sta_len > 0`, `lta_len > 0`, and `sta_len < lta_len`.
- Invalid dimensions or invalid window lengths raise `ValueError` from the Python API.

## Robustness Testing

Run property-based tests (Hypothesis):

```bash
pytest -q tests/test_hypothesis.py
```

Run the full local test suite:

```bash
pytest -q tests
```

## Build options

- **Strict floating-point:** On non-Windows builds, Release mode uses `-ffast-math` by default for speed. For strict IEEE behavior, set `-DFAST_TRIGGER_STRICT_FLOAT=ON` when configuring CMake.
- **Portable vs native CPU tuning:** Release builds are portable by default (no `-march=native`). For local, host-specific tuning, set `-DFAST_TRIGGER_ENABLE_NATIVE_OPT=ON`.
- **OpenMP and Python threads:** `compute_sta_lta_batch` uses OpenMP internally. Calling it from many Python threads at once can over-subscribe the CPU. Prefer limiting concurrency (e.g. one batch call at a time from a thread pool, or set `OMP_NUM_THREADS` in the environment).

## Sanitizer Build (Linux GCC/Clang)

The CMake option `FAST_TRIGGER_ENABLE_SANITIZERS=ON` enables AddressSanitizer and UndefinedBehaviorSanitizer instrumentation.

Note: The commands below use POSIX-style line continuation and environment variables.

```bash
cmake -S . -B build-sanitizers \
  -DFAST_TRIGGER_ENABLE_SANITIZERS=ON \
  -DBUILD_TESTING=ON \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build-sanitizers --parallel
ctest --test-dir build-sanitizers --output-on-failure
```

Recommended runtime options:

```bash
ASAN_OPTIONS=detect_leaks=1:abort_on_error=1:strict_string_checks=1 \
UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 \
pytest -q tests
```

## Links

- [Changelog](CHANGELOG.md)
- [Security policy](SECURITY.md)
- [Repository](https://github.com/AmanSinghNp/fast-trigger)
- [Issues](https://github.com/AmanSinghNp/fast-trigger/issues)
