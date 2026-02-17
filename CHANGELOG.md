# Changelog

All notable changes to this project are documented in this file.

The format is based on Keep a Changelog, and this project follows Semantic Versioning.

## [Unreleased]

## [0.1.0] - 2026-02-15

### Added
- Initial release of `fast-trigger`.
- C++ STA/LTA core implementation with Python bindings via nanobind.
- Batch processing support for 2D seismic traces.
- CMake/scikit-build-core packaging baseline and initial tests.
- Property-based robustness tests with Hypothesis for 1D and 2D STA/LTA APIs.
- Sanitizer build option and Linux sanitizer workflow scaffolding.
- C++ regression tests for near-zero behavior, minimum valid windows, and overflow guards.
- Packaging improvements for stable ABI builds and source distribution inclusion rules.

### Changed
- Hardened core batch implementation against overflow and unsafe alignment assumptions.
- Added defensive argument validation in nanobind entry points while keeping API compatibility.
- Improved test discovery configuration to avoid collecting build/dependency trees.
