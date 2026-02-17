Remove-Item -Recurse -Force ".venv", ".mypy_cache", ".pytest_cache", ".ruff_cache", ".hypothesis" -ErrorAction SilentlyContinue
Remove-Item -Recurse -Force "build", "build-phase1", "build-phase3", "_skbuild", "dist" -ErrorAction SilentlyContinue
Remove-Item -Force "build_log*.txt", "*.log" -ErrorAction SilentlyContinue
