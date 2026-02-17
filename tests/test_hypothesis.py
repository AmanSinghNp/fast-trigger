import numpy as np
import pytest
from hypothesis import HealthCheck, given, settings
from hypothesis import strategies as st
from hypothesis.extra import numpy as hnp

from fast_trigger import compute_sta_lta, compute_sta_lta_batch


def _finite_f64():
    return st.floats(
        min_value=-1e6,
        max_value=1e6,
        allow_nan=False,
        allow_infinity=False,
        width=64,
    )


@st.composite
def _valid_trace_and_windows(draw):
    n_samples = draw(st.integers(min_value=2, max_value=512))
    data = draw(hnp.arrays(np.float64, n_samples, elements=_finite_f64()))
    lta_len = draw(st.integers(min_value=2, max_value=n_samples))
    sta_len = draw(st.integers(min_value=1, max_value=lta_len - 1))
    return data, sta_len, lta_len


@st.composite
def _valid_batch_and_windows(draw):
    n_traces = draw(st.integers(min_value=1, max_value=16))
    n_samples = draw(st.integers(min_value=2, max_value=256))
    data = draw(
        hnp.arrays(
            np.float64,
            (n_traces, n_samples),
            elements=_finite_f64(),
        )
    )
    lta_len = draw(st.integers(min_value=2, max_value=n_samples))
    sta_len = draw(st.integers(min_value=1, max_value=lta_len - 1))
    return data, sta_len, lta_len


@settings(
    max_examples=20,
    deadline=None,
    suppress_health_check=[HealthCheck.too_slow],
)
@given(_valid_trace_and_windows())
def test_hypothesis_sta_lta_shape_and_finite(pair):
    data, sta_len, lta_len = pair
    result = compute_sta_lta(data, sta_len=sta_len, lta_len=lta_len)
    assert result.shape == data.shape
    assert np.all(np.isfinite(result))
    assert np.all(result >= 0.0)


@settings(
    max_examples=20,
    deadline=None,
    suppress_health_check=[HealthCheck.too_slow],
)
@given(_valid_batch_and_windows())
def test_hypothesis_sta_lta_batch_shape_and_finite(triple):
    data, sta_len, lta_len = triple
    result = compute_sta_lta_batch(data, sta_len=sta_len, lta_len=lta_len)
    assert result.shape == data.shape
    assert np.all(np.isfinite(result))
    assert np.all(result >= 0.0)


@settings(
    max_examples=20,
    deadline=None,
    suppress_health_check=[HealthCheck.too_slow],
)
@given(
    hnp.arrays(
        np.float64,
        hnp.array_shapes(min_dims=1, max_dims=1, min_side=2, max_side=256),
        elements=st.one_of(
            _finite_f64(),
            st.sampled_from([np.nan, np.inf, -np.inf]),
        ),
    )
)
def test_hypothesis_sta_lta_handles_nan_inf_without_crash(data):
    sta_len = 1
    lta_len = data.shape[0]
    result = compute_sta_lta(data, sta_len=sta_len, lta_len=lta_len)
    assert result.shape == data.shape


def test_hypothesis_invalid_inputs_raise():
    with pytest.raises((ValueError, RuntimeError)):
        compute_sta_lta(np.array([], dtype=np.float64), sta_len=1, lta_len=2)

    with pytest.raises((ValueError, RuntimeError)):
        compute_sta_lta_batch(np.empty((0, 8), dtype=np.float64), sta_len=1, lta_len=2)

    with pytest.raises((ValueError, RuntimeError)):
        compute_sta_lta_batch(np.empty((4, 0), dtype=np.float64), sta_len=1, lta_len=2)
