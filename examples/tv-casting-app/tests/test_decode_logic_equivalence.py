"""
Property Test — Property 4: Decode logic equivalence for casting clusters
(attribute decoder + event decoder)

**Validates: Requirements 5.1, 5.2**

For each of the 18 casting clusters, extract the switch-case body from the
slim decoder and the full zap-generated decoder, then assert the bodies are
identical after whitespace normalization.

Feature: casting-client-cluster-reduction
Property 4: Decode logic equivalence for casting clusters
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

FULL_ATTR_DECODER = os.path.join(
    REPO_ROOT,
    "src",
    "controller",
    "java",
    "zap-generated",
    "CHIPAttributeTLVValueDecoder.cpp",
)

SLIM_EVENT_DECODER = os.path.join(
    REPO_ROOT,
    "examples",
    "tv-casting-app",
    "tv-casting-common",
    "casting-CHIPEventTLVValueDecoder.cpp",
)

FULL_EVENT_DECODER = os.path.join(
    REPO_ROOT,
    "src",
    "controller",
    "java",
    "zap-generated",
    "CHIPEventTLVValueDecoder.cpp",
)

# ---------------------------------------------------------------------------
# Expected casting cluster set (18 clusters)
# Note: C++ identifier is `WakeOnLan` (not `WakeOnLAN`).
# ---------------------------------------------------------------------------

CASTING_CLUSTERS = sorted([
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
])


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


def _read_file(path: str) -> str:
    with open(path, "r") as f:
        return f.read()


def _extract_case_block(content: str, cluster_name: str) -> str:
    """
    Extract the full case block for a given cluster using brace-matching.

    Finds `case app::Clusters::<cluster_name>::Id: {` and then uses a
    brace-depth counter to locate the matching closing `}`.  Returns the
    text *between* (and including) the opening and closing braces of the
    case block.
    """
    # Build the pattern to find the start of the case block
    pattern = r"case\s+app::Clusters::" + re.escape(cluster_name) + r"::Id\s*:\s*\{"
    match = re.search(pattern, content)
    if match is None:
        raise ValueError(
            f"Could not find case block for cluster '{cluster_name}'"
        )

    # Start brace-matching from the opening '{' of the case block
    open_brace_pos = match.end() - 1  # position of the '{'
    depth = 0
    i = open_brace_pos
    while i < len(content):
        ch = content[i]
        if ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                # Found the matching closing brace
                # Return everything between (exclusive of outer braces)
                return content[open_brace_pos + 1: i]
        # Skip string literals to avoid counting braces inside strings
        elif ch == '"':
            i += 1
            while i < len(content) and content[i] != '"':
                if content[i] == "\\":
                    i += 1  # skip escaped character
                i += 1
        # Skip single-line comments
        elif ch == "/" and i + 1 < len(content) and content[i + 1] == "/":
            i += 2
            while i < len(content) and content[i] != "\n":
                i += 1
        # Skip multi-line comments
        elif ch == "/" and i + 1 < len(content) and content[i + 1] == "*":
            i += 2
            while i < len(content) and not (content[i] == "*" and i + 1 < len(content) and content[i + 1] == "/"):
                i += 1
            i += 1  # skip past the '/'
        i += 1

    raise ValueError(
        f"Unbalanced braces for cluster '{cluster_name}' — "
        f"reached end of file without finding matching '}}'"
    )


def _normalize_whitespace(text: str) -> str:
    """
    Collapse all runs of whitespace (spaces, tabs, newlines) into a single
    space and strip leading/trailing whitespace.
    """
    return re.sub(r"\s+", " ", text).strip()


# ---------------------------------------------------------------------------
# Pre-load file contents (read once, reused across hypothesis examples)
# ---------------------------------------------------------------------------

_slim_content = None
_full_content = None


def _get_slim_content() -> str:
    global _slim_content
    if _slim_content is None:
        _slim_content = _read_file(SLIM_ATTR_DECODER)
    return _slim_content


def _get_full_content() -> str:
    global _full_content
    if _full_content is None:
        _full_content = _read_file(FULL_ATTR_DECODER)
    return _full_content


_slim_event_content = None
_full_event_content = None


def _get_slim_event_content() -> str:
    global _slim_event_content
    if _slim_event_content is None:
        _slim_event_content = _read_file(SLIM_EVENT_DECODER)
    return _slim_event_content


def _get_full_event_content() -> str:
    global _full_event_content
    if _full_event_content is None:
        _full_event_content = _read_file(FULL_EVENT_DECODER)
    return _full_event_content


# ---------------------------------------------------------------------------
# Property-based tests
# ---------------------------------------------------------------------------


@given(cluster=st.sampled_from(CASTING_CLUSTERS))
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_attribute_decode_logic_equivalence(cluster):
    """
    **Validates: Requirements 5.1, 5.2**

    Property 4: For each cluster in the Casting_Cluster_Set, the switch-case
    body in the slim attribute decoder SHALL be identical (modulo whitespace
    normalization) to the corresponding switch-case body in the full
    zap-generated CHIPAttributeTLVValueDecoder.cpp.
    """
    slim_content = _get_slim_content()
    full_content = _get_full_content()

    slim_body = _extract_case_block(slim_content, cluster)
    full_body = _extract_case_block(full_content, cluster)

    slim_normalized = _normalize_whitespace(slim_body)
    full_normalized = _normalize_whitespace(full_body)

    assert slim_normalized == full_normalized, (
        f"Decode logic mismatch for cluster '{cluster}'.\n"
        f"Slim (first 200 chars): {slim_normalized[:200]}...\n"
        f"Full (first 200 chars): {full_normalized[:200]}..."
    )


# ---------------------------------------------------------------------------
# Property-based test — Event decoder
# ---------------------------------------------------------------------------


@given(cluster=st.sampled_from(CASTING_CLUSTERS))
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_event_decode_logic_equivalence(cluster):
    """
    **Validates: Requirements 5.2**

    Property 4: For each cluster in the Casting_Cluster_Set, the switch-case
    body in the slim event decoder SHALL be identical (modulo whitespace
    normalization) to the corresponding switch-case body in the full
    zap-generated CHIPEventTLVValueDecoder.cpp.

    If a cluster is not present in the full event decoder (no events defined),
    it must also be absent from the slim event decoder — or both must have the
    same empty-event pattern.
    """
    slim_content = _get_slim_event_content()
    full_content = _get_full_event_content()

    slim_body = _extract_case_block(slim_content, cluster)
    full_body = _extract_case_block(full_content, cluster)

    slim_normalized = _normalize_whitespace(slim_body)
    full_normalized = _normalize_whitespace(full_body)

    assert slim_normalized == full_normalized, (
        f"Event decode logic mismatch for cluster '{cluster}'.\n"
        f"Slim (first 200 chars): {slim_normalized[:200]}...\n"
        f"Full (first 200 chars): {full_normalized[:200]}..."
    )


# ---------------------------------------------------------------------------
# Allow running directly
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    import sys

    print("Running decode logic equivalence property tests...")

    failures = 0

    print("  Attribute decoder...")
    try:
        test_attribute_decode_logic_equivalence()
        print("    PASS: Property 4 — Decode logic equivalence (attribute)")
    except AssertionError as e:
        print(f"    FAIL: Property 4 (attribute)\n      {e}")
        failures += 1
    except Exception as e:
        print(f"    ERROR: Property 4 (attribute)\n      {e}")
        failures += 1

    print("  Event decoder...")
    try:
        test_event_decode_logic_equivalence()
        print("    PASS: Property 4 — Decode logic equivalence (event)")
    except AssertionError as e:
        print(f"    FAIL: Property 4 (event)\n      {e}")
        failures += 1
    except Exception as e:
        print(f"    ERROR: Property 4 (event)\n      {e}")
        failures += 1

    sys.exit(1 if failures else 0)
