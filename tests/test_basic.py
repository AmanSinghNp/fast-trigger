import numpy as np
import pytest

from fast_trigger import compute_sta_lta, compute_sta_lta_batch


def test_simple_spike():
    """Test detection of a simple spike"""
    data = np.zeros(1000)
    data[500] = 10.0  # Single spike

    result = compute_sta_lta(data, sta_len=10, lta_len=100)

    assert result.shape == data.shape
    assert np.max(result) > 1.0  # Spike should create high ratio
    assert result[0] < result[500]  # Ratio higher at spike


def test_input_validation():
    """Test error handling"""
    with pytest.raises((ValueError, RuntimeError)):
        compute_sta_lta(np.array([]), sta_len=10, lta_len=100)


def test_minimum_valid_window_lengths():
    data = np.array([1.0, 2.0], dtype=np.float64)
    result = compute_sta_lta(data, sta_len=1, lta_len=2)
    assert result.shape == data.shape
    assert np.all(np.isfinite(result))
    assert np.all(result >= 0.0)


def test_batch_validation_and_shape():
    data = np.array([[1.0, 2.0], [3.0, 4.0]], dtype=np.float64)
    result = compute_sta_lta_batch(data, sta_len=1, lta_len=2)
    assert result.shape == data.shape
    assert np.all(np.isfinite(result))

    with pytest.raises((ValueError, RuntimeError)):
        compute_sta_lta_batch(np.empty((0, 2), dtype=np.float64), sta_len=1, lta_len=2)


def test_compute_sta_lta_rejects_non_1d_input():
    data = np.array([[1.0, 2.0], [3.0, 4.0]], dtype=np.float64)
    with pytest.raises(ValueError, match="Expected 1D array"):
        compute_sta_lta(data, sta_len=1, lta_len=2)


def test_compute_sta_lta_batch_rejects_non_2d_input():
    data = np.array([1.0, 2.0, 3.0], dtype=np.float64)
    with pytest.raises(ValueError, match="Expected 2D array"):
        compute_sta_lta_batch(data, sta_len=1, lta_len=2)
