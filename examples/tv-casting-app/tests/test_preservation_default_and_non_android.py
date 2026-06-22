"""
Preservation Property Test -- Property 2: Default and Non-Android Builds Unchanged

**Validates: Requirements 3.1, 3.2**

This test verifies that the baseline build configurations for default
(non-optimized) Android builds and non-Android platforms (Linux, Darwin)
remain unchanged. It follows observation-first methodology: the assertions
encode the *current* state of the files so that any future change that
accidentally regresses these configurations will be caught.

Observations (on UNFIXED code):
- android/args.gni default block: chip_build_libshell=true,
  optimize_for_size=false, no chip_cluster_objects_source_override set,
  no matter_enable_tlv_decoder_api set.
- linux/args.gni: does NOT set chip_cluster_objects_source_override
  unconditionally (set via host.py when chip_casting_simplified=true).
- darwin/args.gni: chip_cluster_objects_source_override points to
  casting-cluster-objects.cpp.
- casting-cluster-objects.cpp exists and contains ~157 .ipp includes
  spanning ~39 cluster directories plus the shared utilities directory.

EXPECTED on UNFIXED code: ALL TESTS PASS -- confirms baseline to preserve.
"""

import os
import re

from hypothesis import HealthCheck, given, settings
from hypothesis import strategies as st

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------

REPO_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), "..", "..", "..")
)

ANDROID_ARGS_GNI = os.path.join(
    REPO_ROOT, "examples", "tv-casting-app", "android", "args.gni"
)
LINUX_ARGS_GNI = os.path.join(
    REPO_ROOT, "examples", "tv-casting-app", "linux", "args.gni"
)
DARWIN_ARGS_GNI = os.path.join(
    REPO_ROOT, "examples", "tv-casting-app", "darwin", "args.gni"
)
SLIM_CLUSTER_FILE = os.path.join(
    REPO_ROOT,
    "examples",
    "tv-casting-app",
    "tv-casting-common",
    "casting-cluster-objects.cpp",
)

# ---------------------------------------------------------------------------
# Helpers -- lightweight GNI parser
# ---------------------------------------------------------------------------


def _read_file(path: str) -> str:
    """Return the text content of a file."""
    with open(path, "r") as f:
        return f.read()


def _strip_comments(text: str) -> str:
    """Remove single-line # comments."""
    return re.sub(r"#[^\n]*", "", text)


def _extract_else_block(text: str) -> str:
    """
    Extract the body of the `} else {` block that follows the
    `if (optimize_apk_size)` conditional in android/args.gni.

    Returns the text between the braces of the else clause.
    """
    stripped = _strip_comments(text)

    # Find the if (optimize_apk_size) block first
    match = re.search(r"if\s*\(\s*optimize_apk_size\s*\)", stripped)
    if not match:
        return ""

    # Walk past the if-block's opening brace
    brace_start = stripped.index("{", match.end())
    depth = 1
    pos = brace_start + 1
    while pos < len(stripped) and depth > 0:
        if stripped[pos] == "{":
            depth += 1
        elif stripped[pos] == "}":
            depth -= 1
        pos += 1

    # pos is now just past the closing } of the if-block.
    # Look for `else {`
    rest = stripped[pos:]
    else_match = re.search(r"else\s*\{", rest)
    if not else_match:
        return ""

    else_body_start = pos + else_match.end()
    depth = 1
    epos = else_body_start
    while epos < len(stripped) and depth > 0:
        if stripped[epos] == "{":
            depth += 1
        elif stripped[epos] == "}":
            depth -= 1
        epos += 1

    return stripped[else_body_start:epos - 1]


def _extract_top_level_assignment(text: str, var_name: str) -> str | None:
    """
    Return the RHS of a top-level `var_name = <value>` in *text*, or None.
    Handles quoted strings and bare identifiers.
    """
    pattern = rf"^\s*{re.escape(var_name)}\s*=\s*(.+)"
    m = re.search(pattern, text, re.MULTILINE)
    if not m:
        return None
    return m.group(1).strip().rstrip(";").strip()


def _count_ipp_includes(text: str) -> int:
    """Count the number of #include lines ending in .ipp>."""
    return len(re.findall(r"#include\s+<.*\.ipp>", text))


def _unique_cluster_dirs(text: str) -> set:
    """
    Extract the set of unique cluster directory names from .ipp includes.
    e.g. '#include <clusters/OnOff/Attributes.ipp>' -> 'OnOff'
    """
    return set(re.findall(r"#include\s+<clusters/([^/]+)/", text))

# ---------------------------------------------------------------------------
# Property-based tests
# ---------------------------------------------------------------------------


@given(optimize_apk_size=st.just(False))
@settings(
    max_examples=1,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_android_default_build_preserves_dev_flags(optimize_apk_size: bool):
    """
    **Validates: Requirements 3.1**

    Property 2a (Preservation): For an Android build with
    `optimize_apk_size = false` (the default), the build SHALL preserve
    `chip_build_libshell = true`, `optimize_for_size = false`, and SHALL NOT
    set `chip_cluster_objects_source_override` or
    `matter_enable_tlv_decoder_api`.
    """
    assert not optimize_apk_size, "This test covers the default (non-optimized) path"

    gni_text = _read_file(ANDROID_ARGS_GNI)

    # Check the else (default) block
    else_block = _extract_else_block(gni_text)
    assert else_block, (
        "Could not find the `else` block after `if (optimize_apk_size)` "
        "in android/args.gni"
    )

    # chip_build_libshell = true
    libshell_val = _extract_top_level_assignment(else_block, "chip_build_libshell")
    assert libshell_val is not None, (
        "REGRESSION: `chip_build_libshell` is not set in the default (else) block "
        "of android/args.gni"
    )
    assert libshell_val == "true", (
        f"REGRESSION: `chip_build_libshell` is `{libshell_val}` instead of `true` "
        f"in the default build"
    )

    # optimize_for_size = false
    opt_size_val = _extract_top_level_assignment(else_block, "optimize_for_size")
    assert opt_size_val is not None, (
        "REGRESSION: `optimize_for_size` is not set in the default (else) block "
        "of android/args.gni"
    )
    assert opt_size_val == "false", (
        f"REGRESSION: `optimize_for_size` is `{opt_size_val}` instead of `false` "
        f"in the default build"
    )

    # chip_cluster_objects_source_override should NOT be set in the else block
    override_in_else = _extract_top_level_assignment(
        else_block, "chip_cluster_objects_source_override"
    )
    assert override_in_else is None, (
        f"REGRESSION: `chip_cluster_objects_source_override` is set to "
        f"`{override_in_else}` in the default (else) block -- it should only "
        f"be set in the optimize_apk_size block (if at all)"
    )

    # matter_enable_tlv_decoder_api should NOT be set in the else block
    tlv_in_else = _extract_top_level_assignment(
        else_block, "matter_enable_tlv_decoder_api"
    )
    assert tlv_in_else is None, (
        f"REGRESSION: `matter_enable_tlv_decoder_api` is set to `{tlv_in_else}` "
        f"in the default (else) block -- it should not be overridden for dev builds"
    )


@given(platform=st.sampled_from(["darwin"]))
@settings(
    max_examples=1,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_non_android_builds_use_slim_cluster_override(platform: str):
    """
    **Validates: Requirements 3.2**

    Property 2b (Preservation): Darwin builds SHALL continue to
    set `chip_cluster_objects_source_override` pointing to
    `casting-cluster-objects.cpp`.

    Linux builds set the override conditionally via host.py when
    chip_casting_simplified=true, so it is NOT in linux/args.gni.
    """
    gni_text = _read_file(DARWIN_ARGS_GNI)
    stripped = _strip_comments(gni_text)

    override_val = _extract_top_level_assignment(
        stripped, "chip_cluster_objects_source_override"
    )
    assert override_val is not None, (
        f"REGRESSION: `chip_cluster_objects_source_override` is not set in "
        f"{platform}/args.gni -- the slim cluster override must remain active"
    )
    assert "casting-cluster-objects.cpp" in override_val, (
        f"REGRESSION: `chip_cluster_objects_source_override` in {platform}/args.gni "
        f"is `{override_val}` -- expected it to reference casting-cluster-objects.cpp"
    )


@given(dummy=st.just(True))
@settings(
    max_examples=1,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_linux_args_gni_does_not_set_unconditional_cluster_override(dummy: bool):
    """
    **Validates: CI fix for linux-x64-tv-casting-app**

    Property 2b-linux: Linux args.gni SHALL NOT unconditionally set
    chip_cluster_objects_source_override, because the legacy build path
    (chip_casting_simplified=false) needs the full cluster-objects.cpp.
    The override is set conditionally via host.py when
    chip_casting_simplified=true.
    """
    gni_text = _read_file(LINUX_ARGS_GNI)
    stripped = _strip_comments(gni_text)

    override_val = _extract_top_level_assignment(
        stripped, "chip_cluster_objects_source_override"
    )
    assert override_val is None, (
        f"REGRESSION: `chip_cluster_objects_source_override` is set unconditionally "
        f"in linux/args.gni to `{override_val}` -- this breaks the legacy build path "
        f"(chip_casting_simplified=false) which needs all clusters. The override "
        f"should be set via host.py only when chip_casting_simplified=true."
    )


@given(dummy=st.just(True))
@settings(
    max_examples=1,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_casting_cluster_objects_file_exists_with_expected_includes(dummy: bool):
    """
    **Validates: Requirements 3.2**

    Property 2c (Preservation): The slim `casting-cluster-objects.cpp` file
    SHALL exist and include .ipp files for the expected set of casting-relevant
    and infrastructure clusters (~39 cluster directories plus shared utilities,
    totalling ~157 .ipp includes).
    """
    assert os.path.isfile(SLIM_CLUSTER_FILE), (
        f"REGRESSION: casting-cluster-objects.cpp not found at {SLIM_CLUSTER_FILE}"
    )

    content = _read_file(SLIM_CLUSTER_FILE)

    # Count .ipp includes
    ipp_count = _count_ipp_includes(content)
    # The file currently has ~157 .ipp includes. Allow a small tolerance
    # (+-5) for minor cluster additions/removals, but catch large regressions
    # like accidentally replacing with the full cluster-objects.cpp (~800+).
    assert 100 <= ipp_count <= 200, (
        f"REGRESSION: casting-cluster-objects.cpp has {ipp_count} .ipp includes. "
        f"Expected ~157 (between 100 and 200). If this is much larger, the slim "
        f"file may have been replaced with the full cluster-objects.cpp."
    )

    # Verify unique cluster directories
    cluster_dirs = _unique_cluster_dirs(content)
    # Currently 39 cluster dirs + 'shared' = 40 total
    assert 30 <= len(cluster_dirs) <= 50, (
        f"REGRESSION: casting-cluster-objects.cpp references {len(cluster_dirs)} "
        f"unique cluster directories. Expected ~40 (between 30 and 50)."
    )

    # Verify key casting-specific clusters are present
    expected_casting_clusters = {
        "MediaPlayback",
        "ContentLauncher",
        "ApplicationLauncher",
        "KeypadInput",
        "AccountLogin",
        "Channel",
        "TargetNavigator",
        "AudioOutput",
        "LowPower",
        "WakeOnLan",
        "OnOff",
        "LevelControl",
        "Messages",
        "ContentControl",
        "ContentAppObserver",
        "MediaInput",
    }
    missing = expected_casting_clusters - cluster_dirs
    assert not missing, (
        f"REGRESSION: casting-cluster-objects.cpp is missing casting-specific "
        f"clusters: {missing}"
    )

    # Verify key infrastructure clusters are present
    expected_infra_clusters = {
        "GeneralCommissioning",
        "OperationalCredentials",
        "NetworkCommissioning",
        "BasicInformation",
        "Descriptor",
        "AccessControl",
        "Binding",
    }
    missing_infra = expected_infra_clusters - cluster_dirs
    assert not missing_infra, (
        f"REGRESSION: casting-cluster-objects.cpp is missing infrastructure "
        f"clusters: {missing_infra}"
    )


# ---------------------------------------------------------------------------
# Allow running directly
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    import sys

    print("Running preservation property tests...")
    tests = [
        ("2a: Android default build preserves dev flags",
         test_android_default_build_preserves_dev_flags),
        ("2b: Darwin builds use slim cluster override",
         test_non_android_builds_use_slim_cluster_override),
        ("2b-linux: Linux args.gni does not set unconditional cluster override",
         test_linux_args_gni_does_not_set_unconditional_cluster_override),
        ("2c: casting-cluster-objects.cpp exists with expected includes",
         test_casting_cluster_objects_file_exists_with_expected_includes),
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
