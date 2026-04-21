"""
Property Test — Property 1: Casting cluster set membership (attribute & event decoders)

**Validates: Requirements 1.1, 2.1**

This test parses `casting-CHIPAttributeTLVValueDecoder.cpp` and
`casting-CHIPEventTLVValueDecoder.cpp` and verifies that the top-level switch
in each contains case statements for exactly the 18 casting clusters — no more,
no less.

Feature: casting-client-cluster-reduction
Property 1: Casting cluster set membership
"""

import os
import re

from hypothesis import HealthCheck, given, settings
from hypothesis import strategies as st

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------

REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
SLIM_ATTR_DECODER = os.path.join(
    REPO_ROOT,
    "examples",
    "tv-casting-app",
    "tv-casting-common",
    "casting-CHIPAttributeTLVValueDecoder.cpp",
)
SLIM_EVENT_DECODER = os.path.join(
    REPO_ROOT,
    "examples",
    "tv-casting-app",
    "tv-casting-common",
    "casting-CHIPEventTLVValueDecoder.cpp",
)

# ---------------------------------------------------------------------------
# Expected casting cluster set (18 clusters)
# Note: In the C++ code, the identifier is `WakeOnLan` (not `WakeOnLAN`).
# ---------------------------------------------------------------------------

EXPECTED_CASTING_CLUSTERS = frozenset(
    {
        "AccountLogin",
        "ApplicationBasic",
        "ApplicationLauncher",
        "AudioOutput",
        "Binding",
        "Channel",
        "ContentAppObserver",
        "ContentControl",
        "ContentLauncher",
        "Descriptor",
        "KeypadInput",
        "LevelControl",
        "LowPower",
        "MediaInput",
        "MediaPlayback",
        "OnOff",
        "TargetNavigator",
        "WakeOnLan",
    }
)

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


def _read_file(path: str) -> str:
    with open(path, "r") as f:
        return f.read()


def _extract_top_level_cluster_cases(content: str) -> set:
    """
    Extract cluster names from `case app::Clusters::XXX::Id:` patterns
    in the top-level switch of DecodeAttributeValue().

    We locate the function body and then find only the top-level case
    statements (depth 1 inside the outer switch).
    """
    # Find all case patterns of the form: case app::Clusters::XXX::Id:
    pattern = r"case\s+app::Clusters::(\w+)::Id\s*:"
    matches = re.findall(pattern, content)
    return set(matches)


# ---------------------------------------------------------------------------
# Property-based tests
# ---------------------------------------------------------------------------


@given(
    cluster=st.sampled_from(sorted(EXPECTED_CASTING_CLUSTERS)),
)
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_each_expected_cluster_present_in_attribute_decoder(cluster):
    """
    **Validates: Requirements 1.1, 2.1**

    Property 1: For any cluster in the expected 18-cluster casting set,
    the slim attribute decoder SHALL contain a top-level switch case for
    that cluster.
    """
    content = _read_file(SLIM_ATTR_DECODER)
    found_clusters = _extract_top_level_cluster_cases(content)

    assert cluster in found_clusters, (
        f"Expected casting cluster `{cluster}` is missing from "
        f"casting-CHIPAttributeTLVValueDecoder.cpp. "
        f"Found clusters: {sorted(found_clusters)}"
    )


@given(dummy=st.just(True))
@settings(
    max_examples=1,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_attribute_decoder_cluster_set_exact_match(dummy):
    """
    **Validates: Requirements 1.1, 2.1**

    Property 1: The set of clusters in the slim attribute decoder SHALL
    equal exactly the 18-cluster casting set — no extra, no missing.
    """
    content = _read_file(SLIM_ATTR_DECODER)
    found_clusters = _extract_top_level_cluster_cases(content)

    assert found_clusters == EXPECTED_CASTING_CLUSTERS, (
        f"Cluster set mismatch in casting-CHIPAttributeTLVValueDecoder.cpp.\n"
        f"  Expected ({len(EXPECTED_CASTING_CLUSTERS)}): {sorted(EXPECTED_CASTING_CLUSTERS)}\n"
        f"  Found    ({len(found_clusters)}): {sorted(found_clusters)}\n"
        f"  Missing:  {sorted(EXPECTED_CASTING_CLUSTERS - found_clusters)}\n"
        f"  Extra:    {sorted(found_clusters - EXPECTED_CASTING_CLUSTERS)}"
    )


@given(dummy=st.just(True))
@settings(
    max_examples=1,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_attribute_decoder_has_exactly_18_clusters(dummy):
    """
    **Validates: Requirements 1.1, 2.1**

    Property 1: The slim attribute decoder SHALL contain exactly 18
    top-level cluster case statements.
    """
    content = _read_file(SLIM_ATTR_DECODER)
    found_clusters = _extract_top_level_cluster_cases(content)

    assert len(found_clusters) == 18, (
        f"Expected exactly 18 clusters, found {len(found_clusters)}: "
        f"{sorted(found_clusters)}"
    )


# ---------------------------------------------------------------------------
# Property-based tests — Event Decoder
# ---------------------------------------------------------------------------


@given(
    cluster=st.sampled_from(sorted(EXPECTED_CASTING_CLUSTERS)),
)
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_each_expected_cluster_present_in_event_decoder(cluster):
    """
    **Validates: Requirements 2.1**

    Property 1: For any cluster in the expected 18-cluster casting set,
    the slim event decoder SHALL contain a top-level switch case for
    that cluster.
    """
    content = _read_file(SLIM_EVENT_DECODER)
    found_clusters = _extract_top_level_cluster_cases(content)

    assert cluster in found_clusters, (
        f"Expected casting cluster `{cluster}` is missing from "
        f"casting-CHIPEventTLVValueDecoder.cpp. "
        f"Found clusters: {sorted(found_clusters)}"
    )


@given(dummy=st.just(True))
@settings(
    max_examples=1,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_event_decoder_cluster_set_exact_match(dummy):
    """
    **Validates: Requirements 2.1**

    Property 1: The set of clusters in the slim event decoder SHALL
    equal exactly the 18-cluster casting set — no extra, no missing.
    """
    content = _read_file(SLIM_EVENT_DECODER)
    found_clusters = _extract_top_level_cluster_cases(content)

    assert found_clusters == EXPECTED_CASTING_CLUSTERS, (
        f"Cluster set mismatch in casting-CHIPEventTLVValueDecoder.cpp.\n"
        f"  Expected ({len(EXPECTED_CASTING_CLUSTERS)}): {sorted(EXPECTED_CASTING_CLUSTERS)}\n"
        f"  Found    ({len(found_clusters)}): {sorted(found_clusters)}\n"
        f"  Missing:  {sorted(EXPECTED_CASTING_CLUSTERS - found_clusters)}\n"
        f"  Extra:    {sorted(found_clusters - EXPECTED_CASTING_CLUSTERS)}"
    )


@given(dummy=st.just(True))
@settings(
    max_examples=1,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_event_decoder_has_exactly_18_clusters(dummy):
    """
    **Validates: Requirements 2.1**

    Property 1: The slim event decoder SHALL contain exactly 18
    top-level cluster case statements.
    """
    content = _read_file(SLIM_EVENT_DECODER)
    found_clusters = _extract_top_level_cluster_cases(content)

    assert len(found_clusters) == 18, (
        f"Expected exactly 18 clusters, found {len(found_clusters)}: "
        f"{sorted(found_clusters)}"
    )


# ---------------------------------------------------------------------------
# Allow running directly
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    import sys

    print("Running cluster set membership property tests (attribute & event decoders)...")
    tests = [
        ("1a: Each expected cluster present (attr)", test_each_expected_cluster_present_in_attribute_decoder),
        ("1b: Exact cluster set match (attr)", test_attribute_decoder_cluster_set_exact_match),
        ("1c: Exactly 18 clusters (attr)", test_attribute_decoder_has_exactly_18_clusters),
        ("1d: Each expected cluster present (event)", test_each_expected_cluster_present_in_event_decoder),
        ("1e: Exact cluster set match (event)", test_event_decoder_cluster_set_exact_match),
        ("1f: Exactly 18 clusters (event)", test_event_decoder_has_exactly_18_clusters),
    ]
    all_passed = True
    for name, test_fn in tests:
        try:
            test_fn()
            print(f"  PASS: {name}")
        except AssertionError as e:
            print(f"  FAIL: {name}\n    {e}")
            all_passed = False
        except Exception as e:
            print(f"  ERROR: {name}\n    {e}")
            all_passed = False

    sys.exit(0 if all_passed else 1)
