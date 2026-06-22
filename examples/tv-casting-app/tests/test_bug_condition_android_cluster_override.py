"""
Bug Condition Exploration Test — Property 1: Slim Cluster Override on Android

Validates: Requirements 1.1, 2.1

This test parses `examples/tv-casting-app/android/args.gni` and checks that
when `optimize_apk_size = true`, the build configuration:

  1. Sets `chip_cluster_objects_source_override` to the slim
     `casting-cluster-objects.cpp` path (so only ~36 casting-relevant clusters
     are compiled instead of all ~200+).
  2. Sets `matter_enable_tlv_decoder_api = false` (so the Java TLV decoder
     files that reference Decode() for every cluster are excluded, removing
     the link-time dependency that blocks the slim override).

EXPECTED on UNFIXED code: FAIL — the current `optimize_apk_size` block does
NOT set either flag, confirming the bug exists.
"""

import os
import re

from hypothesis import HealthCheck, given, settings
from hypothesis import strategies as st

# -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -
# Helpers — lightweight GNI parser
# -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -

REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
ANDROID_ARGS_GNI = os.path.join(
    REPO_ROOT, "examples", "tv-casting-app", "android", "args.gni"
)
SLIM_CLUSTER_FILE = os.path.join(
    REPO_ROOT,
    "examples",
    "tv-casting-app",
    "tv-casting-common",
    "casting-cluster-objects.cpp",
)


def _read_gni(path: str) -> str:
    """Return the text content of a .gni file."""
    with open(path, "r") as f:
        return f.read()


def _strip_comments(text: str) -> str:
    """Remove single-line # comments (but not inside strings)."""
    return re.sub(r"#[^\n]*", "", text)


def _extract_optimize_block(text: str) -> str:
    """
    Extract the body of the first `if (optimize_apk_size)
{
    ...
}` block.

    Uses a simple brace-depth counter — sufficient for the flat structure of
    args.gni files.
    """
    stripped = _strip_comments(text)
    match = re.search(r"if\s*\(\s*optimize_apk_size\s*\)", stripped)
    if not match:
        return ""

# Find the opening brace
    start = stripped.index("{", match.end())
    depth = 1
    pos = start + 1
    while pos < len(stripped) and depth > 0:
        if stripped[pos] == "{":
            depth += 1
        elif stripped[pos] == "}":
            depth -= 1
        pos += 1

    return stripped[start + 1: pos - 1]


def _extract_assignment(block: str, var_name: str) -> str | None:
    """
    Return the RHS of `var_name = <value>` inside *block*, or None if the
    variable is not assigned.  Handles both quoted strings and bare identifiers.
    """
    pattern = rf"{re.escape(var_name)}\s*=\s*(.+)"
    m = re.search(pattern, block)
    if not m:
        return None
    return m.group(1).strip().rstrip(";").strip()

# -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -
# Property - based test
# -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -

# We use Hypothesis to parameterise over a(trivially small) strategy so the
# test is formally a property - based test, but the real assertion is against the
# concrete file on disk.The strategy generates a boolean flag that is always
# True — representing `optimize_apk_size = true`.


@given(optimize_apk_size=st.just(True))
@settings(
    max_examples=1,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_android_optimized_build_uses_slim_cluster_override(optimize_apk_size: bool):
    """
    **Validates: Requirements 1.1, 2.1**

    Property 1 (Bug Condition): For an Android build with
    `optimize_apk_size = true`, the GN args SHALL set
    `chip_cluster_objects_source_override` to the slim
    `casting-cluster-objects.cpp` AND set
    `matter_enable_tlv_decoder_api = false`.
    """
    assert optimize_apk_size, "Test only applies when optimize_apk_size is true"

# -- - Parse the android args.gni -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -
    gni_text = _read_gni(ANDROID_ARGS_GNI)
    opt_block = _extract_optimize_block(gni_text)
    assert opt_block, (
        "Could not find `if (optimize_apk_size) { ... }` block in android/args.gni"
    )

# -- - Assert 1 : chip_cluster_objects_source_override is set -- -- -- -- -- -- -- --
    override_val = _extract_assignment(opt_block, "chip_cluster_objects_source_override")
    assert override_val is not None, (
        "COUNTEREXAMPLE: `chip_cluster_objects_source_override` is NOT set inside "
        "the `optimize_apk_size` block of android/args.gni.  "
        "The full generated cluster-objects.cpp (~200+ clusters) is still compiled "
        "even when optimize_apk_size = true."
    )

# The value should reference the slim casting - cluster - objects.cpp
    assert "casting-cluster-objects.cpp" in override_val, (
        f"COUNTEREXAMPLE: `chip_cluster_objects_source_override` is set to "
        f"`{override_val}` which does not reference casting-cluster-objects.cpp"
    )

# -- - Assert 2 : matter_enable_tlv_decoder_api is false -- -- -- -- -- -- -- -- -- -- -
    tlv_val = _extract_assignment(opt_block, "matter_enable_tlv_decoder_api")
    assert tlv_val is not None, (
        "COUNTEREXAMPLE: `matter_enable_tlv_decoder_api` is NOT set inside "
        "the `optimize_apk_size` block of android/args.gni.  "
        "The Java TLV decoders that reference Decode() for every cluster will "
        "still be compiled, blocking use of the slim cluster override."
    )
    assert tlv_val == "false", (
        f"COUNTEREXAMPLE: `matter_enable_tlv_decoder_api` is set to `{tlv_val}` "
        f"instead of `false` — the TLV decoder dependency is not resolved."
    )


# -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -
# Allow running directly : python test_bug_condition_android_cluster_override.py
# -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -
if __name__ == "__main__":
    print("Running bug condition exploration test...")
    try:
        test_android_optimized_build_uses_slim_cluster_override()
        print("TEST PASSED — expected behavior is satisfied (bug is fixed).")
    except AssertionError as e:
        print(f"TEST FAILED (expected on unfixed code) — counterexample:\n  {e}")
    except Exception as e:
        print(f"TEST ERROR: {e}")
