"""
Bug Condition Exploration Test — Property 1: Override Directory on Android

Validates: Requirements 1.1, 2.1

This test parses `examples/tv-casting-app/android/args.gni` and checks that
when `optimize_apk_size = true`, the build configuration:

  1. Sets `chip_data_model_overrides_dir` to the tv-casting-common directory
     (so only ~36 casting-relevant clusters are compiled instead of all ~200+).
     The directory-based override mechanism also handles TLV decoder overrides
     via well-known filenames, eliminating the need for separate flags.

EXPECTED on FIXED code: PASS — the `optimize_apk_size` block sets
`chip_data_model_overrides_dir` to the override directory.
"""

import os
import re
from pathlib import Path

from hypothesis import HealthCheck, given, settings
from hypothesis import strategies as st

# ---------------------------------------------------------------------------
# Helpers — lightweight GNI parser
# ---------------------------------------------------------------------------

REPO_ROOT = next(filter(lambda p: (p / 'SPECIFICATION_VERSION').is_file(), Path(__file__).parents))
ANDROID_ARGS_GNI = os.path.join(
    REPO_ROOT, "examples", "tv-casting-app", "android", "args.gni"
)
SLIM_CLUSTER_FILE = os.path.join(
    REPO_ROOT,
    "examples",
    "tv-casting-app",
    "tv-casting-common",
    "cluster-objects-override.cpp",
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
    Extract the body of the first `if (optimize_apk_size) { ... }` block.

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

    return stripped[start + 1:pos - 1]


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

# ---------------------------------------------------------------------------
# Property-based test
# ---------------------------------------------------------------------------


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
    `chip_data_model_overrides_dir` to the tv-casting-common directory.
    The directory-based override mechanism handles all source overrides
    (cluster-objects, TLV decoders, accessors, cluster servers).
    """
    assert optimize_apk_size, "Test only applies when optimize_apk_size is true"

    # --- Parse the android args.gni ---
    gni_text = _read_gni(ANDROID_ARGS_GNI)
    opt_block = _extract_optimize_block(gni_text)
    assert opt_block, (
        "Could not find `if (optimize_apk_size) { ... }` block in android/args.gni"
    )

    # --- Assert 1: chip_data_model_overrides_dir is set ---
    override_val = _extract_assignment(opt_block, "chip_data_model_overrides_dir")
    assert override_val is not None, (
        "COUNTEREXAMPLE: `chip_data_model_overrides_dir` is NOT set inside "
        "the `optimize_apk_size` block of android/args.gni.  "
        "The full generated sources (~200+ clusters) are still compiled "
        "even when optimize_apk_size = true."
    )

    # The value should reference the tv-casting-common directory
    assert "tv-casting-common" in override_val, (
        f"COUNTEREXAMPLE: `chip_data_model_overrides_dir` is set to "
        f"`{override_val}` which does not reference tv-casting-common"
    )


# ---------------------------------------------------------------------------
# Allow running directly: python test_bug_condition_android_cluster_override.py
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    print("Running bug condition exploration test...")
    try:
        test_android_optimized_build_uses_slim_cluster_override()
        print("TEST PASSED — expected behavior is satisfied (bug is fixed).")
    except AssertionError as e:
        print(f"TEST FAILED (expected on unfixed code) — counterexample:\n  {e}")
    except Exception as e:
        print(f"TEST ERROR: {e}")
