import numpy as np
import pytest

import fast_trigger


def test_sta_len_equals_lta_len():
    """Test that sta_len == lta_len raises ValueError"""
    data = np.ones(100)
    with pytest.raises(ValueError, match="STA length must be < LTA length"):
        fast_trigger.compute_sta_lta(data, sta_len=10, lta_len=10)

def test_sta_len_greater_than_lta_len():
    """Test that sta_len > lta_len raises ValueError"""
    data = np.ones(100)
    with pytest.raises(ValueError, match="STA length must be < LTA length"):
        fast_trigger.compute_sta_lta(data, sta_len=11, lta_len=10)

def test_window_exceeds_data_length():
    """Test that window length > data length raises ValueError"""
    data = np.ones(10)
    with pytest.raises(ValueError, match="Window lengths must not exceed trace length|Input array cannot be empty"):
        # The C++ code checks `sta_len > n` specifically
        fast_trigger.compute_sta_lta(data, sta_len=5, lta_len=20)

def test_zero_length_windows():
    """Test zero length windows"""
    data = np.ones(100)
    with pytest.raises(ValueError, match="Window lengths must be > 0"):
        fast_trigger.compute_sta_lta(data, sta_len=0, lta_len=10)

def test_nan_input_stability():
    """Test that NaN inputs don't crash the extension (result might be NaN, but no segfault)"""
    data = np.array([1.0, np.nan, 2.0], dtype=np.float64)
    # This should verify it doesn't crash. 
    # Logic: abs(nan) is nan. sta/lta updates will propagate nan.
    # We just want to ensure no crash.
    result = fast_trigger.compute_sta_lta(data, sta_len=1, lta_len=2)
    # Result might be NaN or 0.0 depending on internal checks (lta > 1e-10 is False for NaN)
    # The key is that it doesn't crash and returns valid storage
    assert result.shape == data.shape
    assert np.all(np.isfinite(result)) or np.any(np.isnan(result))

def test_inf_input_stability():
    """Test that Inf inputs don't crash"""
    data = np.array([1.0, np.inf, 2.0], dtype=np.float64)
    result = fast_trigger.compute_sta_lta(data, sta_len=1, lta_len=2)
    # Result will likely be NaN or Inf, just check it returns
    assert result.shape == data.shape

def test_batch_exception_propagation():
    """Test that exceptions in batch threads propagate safely"""
    # If one trace is invalid, does it crash?
    # Actually batch mode assumes consistent n_samples.
    # But if we pass valid shapes but invalid windows:
    data = np.ones((2, 10))
    with pytest.raises(ValueError, match="STA length must be < LTA length"):
        fast_trigger.compute_sta_lta_batch(data, sta_len=10, lta_len=10)
