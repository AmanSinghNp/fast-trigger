# Contributing to fast-trigger

Thanks for your interest in contributing.

## Development setup

1. Create and activate a Python virtual environment.
2. Install development dependencies:
   - `python -m pip install -e .[dev,test]`
3. Install pre-commit hooks:
   - `pre-commit install`

## Local checks

Run these before opening a pull request:

- `ruff check .`
- `ruff format --check .`
- `mypy`
- `pytest -q`

For C++ tests, configure and build with CMake, then run CTest.

To clear local generated artifacts and caches, run:

- `powershell -ExecutionPolicy Bypass -File scripts/cleanup-local.ps1`

## Pull request guidelines

- Keep changes focused and small.
- Add or update tests for behavioral changes.
- Update docs when user-facing behavior changes.
- Ensure CI is passing.

## Coding conventions

- Python code style and linting are managed by Ruff.
- C++ formatting is managed by clang-format.
- Prefer clear, readable code and explicit error handling.
