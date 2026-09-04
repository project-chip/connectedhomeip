"""
Property Test — Property 5: Generation script TLV decoder extraction

Feature: consolidate-overrides-and-generation-script

**Validates: Requirements 9.5, 9.6, 11.5**

For any valid subset of cluster names from the full TLV decoder files,
the generation script SHALL produce slim decoder files that contain
exactly the case blocks for the specified clusters, plus a default case,
and no case blocks for clusters not in the subset.
"""

import os
import re
import sys
import tempfile

from hypothesis import HealthCheck, given, settings
from hypothesis import strategies as st

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------

REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
FULL_ATTR_DECODER = os.path.join(
    REPO_ROOT, "src", "controller", "java", "zap-generated",
    "CHIPAttributeTLVValueDecoder.cpp",
)
FULL_EVENT_DECODER = os.path.join(
    REPO_ROOT, "src", "controller", "java", "zap-generated",
    "CHIPEventTLVValueDecoder.cpp",
)

# Import the generation script
sys.path.insert(0, os.path.join(REPO_ROOT, "examples", "tv-casting-app", "tv-casting-common"))
from generate_slim_overrides import generate_tlv_decoder  # noqa: E402

# ---------------------------------------------------------------------------
# Discover all cluster names from the full source
# ---------------------------------------------------------------------------


def _discover_tlv_cluster_names(source_path):
    """Extract unique cluster names from case app::Clusters::X::Id: lines."""
    names = set()
    with open(source_path, "r") as f:
        for line in f:
            m = re.search(r'case\s+app::Clusters::(\w+)::Id:', line)
            if m:
                names.add(m.group(1))
    return sorted(names)


ALL_ATTR_CLUSTERS = _discover_tlv_cluster_names(FULL_ATTR_DECODER)
ALL_EVENT_CLUSTERS = _discover_tlv_cluster_names(FULL_EVENT_DECODER)

# ---------------------------------------------------------------------------
# Strategies
# ---------------------------------------------------------------------------

attr_cluster_subsets = st.lists(
    st.sampled_from(ALL_ATTR_CLUSTERS),
    min_size=1,
    max_size=min(len(ALL_ATTR_CLUSTERS), 30),
    unique=True,
)

event_cluster_subsets = st.lists(
    st.sampled_from(ALL_EVENT_CLUSTERS),
    min_size=1,
    max_size=min(len(ALL_EVENT_CLUSTERS), 30),
    unique=True,
)

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


def _extract_case_clusters(text):
    """Extract cluster names from case app::Clusters::X::Id: lines."""
    clusters = set()
    for m in re.finditer(r'case\s+app::Clusters::(\w+)::Id:', text):
        clusters.add(m.group(1))
    return clusters


# ---------------------------------------------------------------------------
# Property tests — Attribute decoder
# ---------------------------------------------------------------------------


@given(subset=attr_cluster_subsets)
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.too_slow],
    deadline=None,
)
def test_attr_decoder_contains_only_requested_case_blocks(subset):
    """
    **Validates: Requirements 9.5, 11.5**

    For any subset of cluster names, the generated attribute TLV decoder
    SHALL contain case blocks only for those clusters.
    """
    with tempfile.TemporaryDirectory() as tmpdir:
        output_path = os.path.join(tmpdir, "CHIPAttributeTLVValueDecoder-override.cpp")
        generate_tlv_decoder(FULL_ATTR_DECODER, subset, output_path, "attribute")

        with open(output_path, "r") as f:
            content = f.read()

        case_clusters = _extract_case_clusters(content)
        requested = set(subset)

        # All requested clusters should have case blocks
        missing = requested - case_clusters
        assert not missing, (
            f"Missing case blocks for requested clusters: {missing}"
        )

        # No extra clusters should have case blocks
        extra = case_clusters - requested
        assert not extra, (
            f"Case blocks found for clusters NOT in the subset: {extra}"
        )


@given(subset=attr_cluster_subsets)
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.too_slow],
    deadline=None,
)
def test_attr_decoder_has_default_case(subset):
    """
    **Validates: Requirements 9.5, 11.5**

    For any subset, the generated attribute decoder SHALL contain a
    default: case with CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB.
    """
    with tempfile.TemporaryDirectory() as tmpdir:
        output_path = os.path.join(tmpdir, "CHIPAttributeTLVValueDecoder-override.cpp")
        generate_tlv_decoder(FULL_ATTR_DECODER, subset, output_path, "attribute")

        with open(output_path, "r") as f:
            content = f.read()

        assert "default:" in content, "Output must contain a default: case"
        assert "CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB" in content, (
            "Output must contain CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB in default case"
        )


# ---------------------------------------------------------------------------
# Property tests — Event decoder
# ---------------------------------------------------------------------------


@given(subset=event_cluster_subsets)
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.too_slow],
    deadline=None,
)
def test_event_decoder_contains_only_requested_case_blocks(subset):
    """
    **Validates: Requirements 9.6, 11.5**

    For any subset of cluster names, the generated event TLV decoder
    SHALL contain case blocks only for those clusters.
    """
    with tempfile.TemporaryDirectory() as tmpdir:
        output_path = os.path.join(tmpdir, "CHIPEventTLVValueDecoder-override.cpp")
        generate_tlv_decoder(FULL_EVENT_DECODER, subset, output_path, "event")

        with open(output_path, "r") as f:
            content = f.read()

        case_clusters = _extract_case_clusters(content)
        requested = set(subset)

        missing = requested - case_clusters
        assert not missing, (
            f"Missing case blocks for requested clusters: {missing}"
        )

        extra = case_clusters - requested
        assert not extra, (
            f"Case blocks found for clusters NOT in the subset: {extra}"
        )


@given(subset=event_cluster_subsets)
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.too_slow],
    deadline=None,
)
def test_event_decoder_has_default_case(subset):
    """
    **Validates: Requirements 9.6, 11.5**

    For any subset, the generated event decoder SHALL contain a
    default: case with CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB.
    """
    with tempfile.TemporaryDirectory() as tmpdir:
        output_path = os.path.join(tmpdir, "CHIPEventTLVValueDecoder-override.cpp")
        generate_tlv_decoder(FULL_EVENT_DECODER, subset, output_path, "event")

        with open(output_path, "r") as f:
            content = f.read()

        assert "default:" in content, "Output must contain a default: case"
        assert "CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB" in content, (
            "Output must contain CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB in default case"
        )
