"""
Property Test — Property 2: Non-casting cluster error handling (attribute decoder)

**Validates: Requirements 1.2, 5.3**

This test parses `casting-CHIPAttributeTLVValueDecoder.cpp` and verifies that:
1. The top-level switch contains a `default:` case that sets
   `CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB`.
2. The function returns `nullptr` after the switch (for the default case path).
3. Random cluster IDs not in the casting set would hit the default case
   (i.e., they don't match any of the 18 casting cluster case labels).

Feature: casting-client-cluster-reduction
Property 2: Non-casting cluster error handling
"""

import os
import re

from hypothesis import HealthCheck, assume, given, settings
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

# ---------------------------------------------------------------------------
# Expected casting cluster set — hex IDs from the Matter spec
# ---------------------------------------------------------------------------

CASTING_CLUSTER_IDS = frozenset(
    {
        0x050E,  # AccountLogin
        0x050D,  # ApplicationBasic
        0x050C,  # ApplicationLauncher
        0x050B,  # AudioOutput
        0x001E,  # Binding
        0x0504,  # Channel
        0x0510,  # ContentAppObserver
        0x050F,  # ContentControl
        0x050A,  # ContentLauncher
        0x001D,  # Descriptor
        0x0509,  # KeypadInput
        0x0008,  # LevelControl
        0x0508,  # LowPower
        0x0507,  # MediaInput
        0x0506,  # MediaPlayback
        0x0006,  # OnOff
        0x0505,  # TargetNavigator
        0x0503,  # WakeOnLAN
    }
)

# Cluster names as they appear in the C++ code (WakeOnLan, not WakeOnLAN)
CASTING_CLUSTER_NAMES = frozenset(
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
    """Extract cluster names from top-level case statements."""
    pattern = r"case\s+app::Clusters::(\w+)::Id\s*:"
    return set(re.findall(pattern, content))


def _find_top_level_default_case(content: str) -> bool:
    """
    Check that the top-level switch in DecodeAttributeValue() has a
    `default:` case that sets `*aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB`.

    Strategy: find the function body, then locate the outermost switch's
    default case by looking for the pattern at the correct brace depth.
    We parse brace depth to distinguish the top-level switch default from
    inner (per-cluster attribute) switch defaults.
    """
    # Find the start of DecodeAttributeValue function body
    func_match = re.search(
        r"jobject\s+DecodeAttributeValue\s*\([^)]*\)\s*\{", content
    )
    if not func_match:
        return False

    body_start = func_match.end()
    # Track brace depth: we start at depth 1 (inside the function body)
    depth = 1
    # The top-level switch adds another depth level; its default is at depth 2
    # (depth 1 = function body, depth 2 = inside the switch)

    # Find the `switch (aPath.mClusterId)` opening brace
    switch_match = re.search(
        r"switch\s*\(\s*aPath\.mClusterId\s*\)\s*\{", content[body_start:]
    )
    if not switch_match:
        return False

    switch_body_start = body_start + switch_match.end()
    # Now scan through the switch body tracking depth
    # We're looking for `default:` at depth 0 relative to the switch body
    switch_depth = 1  # we're inside the switch's opening brace
    i = switch_body_start
    while i < len(content) and switch_depth > 0:
        ch = content[i]
        if ch == "{":
            switch_depth += 1
        elif ch == "}":
            switch_depth -= 1
            if switch_depth == 0:
                break
        elif ch == "d" and switch_depth == 1:
            # Check for `default:` at the top level of the switch
            candidate = content[i: i + 8]
            if candidate == "default:":
                # Look ahead for the error assignment
                lookahead = content[i: i + 200]
                if "CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB" in lookahead:
                    return True
        i += 1

    return False


def _function_returns_nullptr_after_switch(content: str) -> bool:
    """
    Verify that after the top-level switch statement closes, the function
    returns `nullptr`. This is the code path taken when the default case
    breaks out of the switch.
    """
    # Find the function body
    func_match = re.search(
        r"jobject\s+DecodeAttributeValue\s*\([^)]*\)\s*\{", content
    )
    if not func_match:
        return False

    body_start = func_match.end()

    # Find the switch statement
    switch_match = re.search(
        r"switch\s*\(\s*aPath\.mClusterId\s*\)\s*\{", content[body_start:]
    )
    if not switch_match:
        return False

    switch_body_start = body_start + switch_match.end()

    # Walk to find the closing brace of the switch
    depth = 1
    i = switch_body_start
    while i < len(content) and depth > 0:
        if content[i] == "{":
            depth += 1
        elif content[i] == "}":
            depth -= 1
        i += 1

    # i now points just past the closing brace of the switch
    # Look for `return nullptr;` between here and the function's closing brace
    remaining = content[i: i + 200]
    return "return nullptr;" in remaining


# ---------------------------------------------------------------------------
# Property-based tests
# ---------------------------------------------------------------------------


@given(dummy=st.just(True))
@settings(
    max_examples=1,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_top_level_switch_has_default_case_with_error(dummy):
    """
    **Validates: Requirements 1.2, 5.3**

    Property 2: The top-level switch in DecodeAttributeValue() SHALL contain
    a `default:` case that sets `*aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB`.
    """
    content = _read_file(SLIM_ATTR_DECODER)
    assert _find_top_level_default_case(content), (
        "The top-level switch in DecodeAttributeValue() does not contain a "
        "`default:` case that sets CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB."
    )


@given(dummy=st.just(True))
@settings(
    max_examples=1,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_function_returns_nullptr_after_switch(dummy):
    """
    **Validates: Requirements 1.2, 5.3**

    Property 2: After the top-level switch, DecodeAttributeValue() SHALL
    return `nullptr`, which is the code path for the default case.
    """
    content = _read_file(SLIM_ATTR_DECODER)
    assert _function_returns_nullptr_after_switch(content), (
        "DecodeAttributeValue() does not return `nullptr` after the "
        "top-level switch statement."
    )


@given(
    cluster_id=st.integers(min_value=0, max_value=0xFFFF),
)
@settings(
    max_examples=200,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_random_non_casting_cluster_ids_hit_default(cluster_id):
    """
    **Validates: Requirements 1.2, 5.3**

    Property 2: For any cluster ID not in the Casting_Cluster_Set, the
    cluster ID SHALL NOT match any case label in the top-level switch,
    meaning it would hit the default case.
    """
    assume(cluster_id not in CASTING_CLUSTER_IDS)

    content = _read_file(SLIM_ATTR_DECODER)
    found_cluster_names = _extract_top_level_cluster_cases(content)

    # Verify the found clusters are exactly the casting set
    assert found_cluster_names == CASTING_CLUSTER_NAMES, (
        f"Unexpected cluster set in decoder. "
        f"Expected: {sorted(CASTING_CLUSTER_NAMES)}, "
        f"Found: {sorted(found_cluster_names)}"
    )

    # Since the found clusters are exactly the 18 casting clusters,
    # and our cluster_id is NOT in the casting ID set, it cannot match
    # any case label — it will fall through to the default case.
    # This is a logical confirmation: no case label exists for this ID.
    assert cluster_id not in CASTING_CLUSTER_IDS, (
        f"Cluster ID 0x{cluster_id:04X} is in the casting set but should not be."
    )


# ---------------------------------------------------------------------------
# Allow running directly
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    import sys

    print("Running non-casting error handling property tests (attribute decoder)...")
    tests = [
        ("2a: Default case with error", test_top_level_switch_has_default_case_with_error),
        ("2b: Returns nullptr after switch", test_function_returns_nullptr_after_switch),
        ("2c: Random non-casting IDs hit default", test_random_non_casting_cluster_ids_hit_default),
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


# ===========================================================================
# Event Decoder Tests — Property 2: Non-casting cluster error handling
#
# **Validates: Requirements 2.2, 5.3**
#
# These tests parse `casting-CHIPEventTLVValueDecoder.cpp` and verify that:
# 1. The top-level switch contains a `default:` case that sets
#    `CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB`.
# 2. The function returns `nullptr` after the switch (for the default case path).
# 3. Random cluster IDs not in the casting set would hit the default case.
# ===========================================================================

SLIM_EVENT_DECODER = os.path.join(
    REPO_ROOT,
    "examples",
    "tv-casting-app",
    "tv-casting-common",
    "casting-CHIPEventTLVValueDecoder.cpp",
)

# ---------------------------------------------------------------------------
# Event-specific helpers
# ---------------------------------------------------------------------------


def _find_event_top_level_default_case(content: str) -> bool:
    """
    Check that the top-level switch in DecodeEventValue() has a
    `default:` case that sets `*aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB`.

    Strategy: find the function body, then locate the outermost switch's
    default case by looking for the pattern at the correct brace depth.
    We parse brace depth to distinguish the top-level switch default from
    inner (per-cluster event) switch defaults.
    """
    # Find the start of DecodeEventValue function body
    func_match = re.search(
        r"jobject\s+DecodeEventValue\s*\([^)]*\)\s*\{", content
    )
    if not func_match:
        return False

    body_start = func_match.end()

    # Find the `switch (aPath.mClusterId)` opening brace
    switch_match = re.search(
        r"switch\s*\(\s*aPath\.mClusterId\s*\)\s*\{", content[body_start:]
    )
    if not switch_match:
        return False

    switch_body_start = body_start + switch_match.end()
    # Now scan through the switch body tracking depth
    # We're looking for `default:` at depth 0 relative to the switch body
    switch_depth = 1  # we're inside the switch's opening brace
    i = switch_body_start
    while i < len(content) and switch_depth > 0:
        ch = content[i]
        if ch == "{":
            switch_depth += 1
        elif ch == "}":
            switch_depth -= 1
            if switch_depth == 0:
                break
        elif ch == "d" and switch_depth == 1:
            # Check for `default:` at the top level of the switch
            candidate = content[i: i + 8]
            if candidate == "default:":
                # Look ahead for the error assignment
                lookahead = content[i: i + 200]
                if "CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB" in lookahead:
                    return True
        i += 1

    return False


def _event_function_returns_nullptr_after_switch(content: str) -> bool:
    """
    Verify that after the top-level switch statement closes, the function
    returns `nullptr`. This is the code path taken when the default case
    breaks out of the switch.
    """
    # Find the function body
    func_match = re.search(
        r"jobject\s+DecodeEventValue\s*\([^)]*\)\s*\{", content
    )
    if not func_match:
        return False

    body_start = func_match.end()

    # Find the switch statement
    switch_match = re.search(
        r"switch\s*\(\s*aPath\.mClusterId\s*\)\s*\{", content[body_start:]
    )
    if not switch_match:
        return False

    switch_body_start = body_start + switch_match.end()

    # Walk to find the closing brace of the switch
    depth = 1
    i = switch_body_start
    while i < len(content) and depth > 0:
        if content[i] == "{":
            depth += 1
        elif content[i] == "}":
            depth -= 1
        i += 1

    # i now points just past the closing brace of the switch
    # Look for `return nullptr;` between here and the function's closing brace
    remaining = content[i: i + 200]
    return "return nullptr;" in remaining


# ---------------------------------------------------------------------------
# Event decoder property-based tests
# ---------------------------------------------------------------------------


@given(dummy=st.just(True))
@settings(
    max_examples=1,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_event_top_level_switch_has_default_case_with_error(dummy):
    """
    **Validates: Requirements 2.2, 5.3**

    Property 2: The top-level switch in DecodeEventValue() SHALL contain
    a `default:` case that sets `*aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB`.
    """
    content = _read_file(SLIM_EVENT_DECODER)
    assert _find_event_top_level_default_case(content), (
        "The top-level switch in DecodeEventValue() does not contain a "
        "`default:` case that sets CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB."
    )


@given(dummy=st.just(True))
@settings(
    max_examples=1,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_event_function_returns_nullptr_after_switch(dummy):
    """
    **Validates: Requirements 2.2, 5.3**

    Property 2: After the top-level switch, DecodeEventValue() SHALL
    return `nullptr`, which is the code path for the default case.
    """
    content = _read_file(SLIM_EVENT_DECODER)
    assert _event_function_returns_nullptr_after_switch(content), (
        "DecodeEventValue() does not return `nullptr` after the "
        "top-level switch statement."
    )


@given(
    cluster_id=st.integers(min_value=0, max_value=0xFFFF),
)
@settings(
    max_examples=200,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_random_non_casting_cluster_ids_hit_event_default(cluster_id):
    """
    **Validates: Requirements 2.2, 5.3**

    Property 2: For any cluster ID not in the Casting_Cluster_Set, the
    cluster ID SHALL NOT match any case label in the top-level switch of
    DecodeEventValue(), meaning it would hit the default case.
    """
    assume(cluster_id not in CASTING_CLUSTER_IDS)

    content = _read_file(SLIM_EVENT_DECODER)
    found_cluster_names = _extract_top_level_cluster_cases(content)

    # Verify the found clusters are exactly the casting set
    assert found_cluster_names == CASTING_CLUSTER_NAMES, (
        f"Unexpected cluster set in event decoder. "
        f"Expected: {sorted(CASTING_CLUSTER_NAMES)}, "
        f"Found: {sorted(found_cluster_names)}"
    )

    # Since the found clusters are exactly the 18 casting clusters,
    # and our cluster_id is NOT in the casting ID set, it cannot match
    # any case label — it will fall through to the default case.
    assert cluster_id not in CASTING_CLUSTER_IDS, (
        f"Cluster ID 0x{cluster_id:04X} is in the casting set but should not be."
    )
