"""fast-trigger: High-performance seismic event detection"""

__version__ = "0.1.0"

import numpy as np

from fast_trigger._fast_trigger_impl import (
    compute_sta_lta as _compute_sta_lta_impl,
)
from fast_trigger._fast_trigger_impl import (
    compute_sta_lta_batch as _compute_sta_lta_batch_impl,
)


def compute_sta_lta(data, *, sta_len, lta_len):
    """STA/LTA ratio for a single trace. Inputs with NaN/Inf may produce non-finite output."""
    arr = np.asarray(data, dtype=np.float64, order="C")
    if arr.ndim != 1:
        raise ValueError("Expected 1D array")
    return _compute_sta_lta_impl(arr, sta_len=sta_len, lta_len=lta_len)


def compute_sta_lta_batch(data, *, sta_len, lta_len):
    """STA/LTA ratio for multiple traces. Uses OpenMP; calling from many threads may over-subscribe. Inputs with NaN/Inf may produce non-finite output."""
    arr = np.asarray(data, dtype=np.float64, order="C")
    if arr.ndim != 2:
        raise ValueError("Expected 2D array")
    return _compute_sta_lta_batch_impl(arr, sta_len=sta_len, lta_len=lta_len)


__all__ = ["__version__", "compute_sta_lta", "compute_sta_lta_batch"]
