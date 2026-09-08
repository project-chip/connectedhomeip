"""
Phase 2 Build Config Property Tests

Feature: casting-apk-size-reduction-phase2

Property 7: CONFIG_BUILD_FOR_HOST_UNIT_TEST is defined consistently
**Validates: Requirements 4.1, 4.2**

Property 8: ICD client deps are conditional on optimize_apk_size
**Validates: Requirements 6.1, 6.2**

This test verifies that:
  - CHIPProjectAppConfig.h defines CONFIG_BUILD_FOR_HOST_UNIT_TEST to 0 inside a
    #ifndef guard. This header is the project-config include, so it is seen by
    EVERY translation unit (app + core src/*). It must resolve to one value
    across the whole link.
  - No BUILD.gn adds a per-target `-DCONFIG_BUILD_FOR_HOST_UNIT_TEST=...`
    override. Such an override reaches only some TUs, so core types (CASESession,
    and thus the Server singleton and its FabricTable) end up with different
    layouts per TU -- an ODR violation that crashes at startup in
    FabricTable::AddFabricDelegate.
  - tv-casting-common/BUILD.gn wraps ICD client deps
    (icd/client:handler, icd/client:manager) inside a !optimize_apk_size
    conditional so they are excluded in size-optimized builds.
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

BUILD_GN_FILE = os.path.join(
    REPO_ROOT,
    "examples",
    "tv-casting-app",
    "tv-casting-common",
    "BUILD.gn",
)

CHIP_PROJECT_APP_CONFIG_FILE = os.path.join(
    REPO_ROOT,
    "examples",
    "tv-casting-app",
    "tv-casting-common",
    "include",
    "CHIPProjectAppConfig.h",
)

# ---------------------------------------------------------------------------
# Constants
# ---------------------------------------------------------------------------

ICD_CLIENT_DEPS = [
    "icd/client:handler",
    "icd/client:manager",
]

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


def _read_file(path: str) -> str:
    """Return the text content of a file."""
    with open(path, "r") as f:
        return f.read()


def _find_conditional_blocks(content: str, condition_pattern: str) -> list:
    """Find all blocks guarded by a GN if-condition matching the pattern.

    Returns a list of strings, each being the body of a matching if-block.
    Handles nested braces.
    """
    blocks = []
    # Match 'if (<condition_pattern>) {'
    pattern = rf'if\s*\(\s*{re.escape(condition_pattern)}\s*\)\s*\{{'
    for m in re.finditer(pattern, content):
        start = m.end()
        depth = 1
        pos = start
        while pos < len(content) and depth > 0:
            if content[pos] == '{':
                depth += 1
            elif content[pos] == '}':
                depth -= 1
            pos += 1
        blocks.append(content[start:pos - 1])
    return blocks


# ---------------------------------------------------------------------------
# Property 8: ICD client deps are conditional on optimize_apk_size
# ---------------------------------------------------------------------------


@given(dep=st.sampled_from(ICD_CLIENT_DEPS))
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_icd_client_deps_inside_not_optimize_apk_size_block(dep: str):
    """
    **Validates: Requirements 6.1, 6.2**

    Property 8: ICD client deps are conditional on optimize_apk_size

    For any ICD client dependency (icd/client:handler, icd/client:manager),
    the tv-casting-common/BUILD.gn SHALL include that dependency only inside
    a !optimize_apk_size conditional block, ensuring it is excluded when
    optimize_apk_size=true.
    """
    content = _read_file(BUILD_GN_FILE)

    # The dep must appear inside an if (!optimize_apk_size) block
    guarded_blocks = _find_conditional_blocks(content, "!optimize_apk_size")
    dep_in_guarded = any(dep in block for block in guarded_blocks)

    assert dep_in_guarded, (
        f"FAIL: ICD client dependency '{dep}' is not inside an "
        f"if (!optimize_apk_size) block in BUILD.gn. "
        f"It must be conditional so it is excluded in size-optimized builds."
    )


def test_icd_client_deps_not_unconditional():
    """
    **Validates: Requirements 6.1, 6.2**

    Property 8 (supplement): ICD client deps are NOT present unconditionally.

    Verify that icd/client deps do not appear outside of any
    !optimize_apk_size conditional block.
    """
    content = _read_file(BUILD_GN_FILE)
    lines = content.split("\n")

    # Find all !optimize_apk_size block line ranges
    guarded_blocks = _find_conditional_blocks(content, "!optimize_apk_size")
    guarded_text = "\n".join(guarded_blocks)

    for dep in ICD_CLIENT_DEPS:
        # Count total occurrences vs occurrences inside guarded blocks
        total_count = content.count(dep)
        guarded_count = guarded_text.count(dep)

        assert total_count == guarded_count, (
            f"FAIL: ICD client dependency '{dep}' appears {total_count} time(s) "
            f"in BUILD.gn but only {guarded_count} time(s) inside "
            f"!optimize_apk_size blocks. All occurrences must be conditional."
        )


# ---------------------------------------------------------------------------
# Property 7: CONFIG_BUILD_FOR_HOST_UNIT_TEST is defined consistently
# ---------------------------------------------------------------------------


def test_config_build_for_host_unit_test_has_ifndef_guard():
    """
    **Validates: Requirements 4.1, 4.2**

    Property 7: CONFIG_BUILD_FOR_HOST_UNIT_TEST is defined consistently

    The CHIPProjectAppConfig.h header SHALL define CONFIG_BUILD_FOR_HOST_UNIT_TEST
    inside a #ifndef guard. This header is the global project-config include, so
    the guarded #define is the single source of truth for every translation
    unit in the build.
    """
    content = _read_file(CHIP_PROJECT_APP_CONFIG_FILE)

    # Check for the #ifndef / #define / #endif pattern
    ifndef_pattern = (
        r'#ifndef\s+CONFIG_BUILD_FOR_HOST_UNIT_TEST\s*\n'
        r'\s*#define\s+CONFIG_BUILD_FOR_HOST_UNIT_TEST\s+\d+\s*\n'
        r'\s*#endif'
    )
    match = re.search(ifndef_pattern, content)

    assert match is not None, (
        "FAIL: CONFIG_BUILD_FOR_HOST_UNIT_TEST in CHIPProjectAppConfig.h "
        "does not use a #ifndef guard. Expected pattern:\n"
        "  #ifndef CONFIG_BUILD_FOR_HOST_UNIT_TEST\n"
        "  #define CONFIG_BUILD_FOR_HOST_UNIT_TEST 1\n"
        "  #endif"
    )


def test_config_build_for_host_unit_test_default_value_is_1():
    """
    **Validates: Requirements 4.2**

    Property 7 (supplement): Default value is 1

    The #define inside the #ifndef guard SHALL set
    CONFIG_BUILD_FOR_HOST_UNIT_TEST to 1. This header is seen by every
    translation unit (app sources and core src/*), so the value must be
    consistent across the whole link. It is 1 because the non-size-optimized
    build compiles the legacy chip-tool InteractionModel.cpp, which uses the
    test-only CommandSender/ReadClient hooks compiled only when this is 1
    (matching chip-tool's own CHIPProjectAppConfig.h, which shares that code).
    """
    content = _read_file(CHIP_PROJECT_APP_CONFIG_FILE)

    # Extract the value from the guarded #define
    pattern = (
        r'#ifndef\s+CONFIG_BUILD_FOR_HOST_UNIT_TEST\s*\n'
        r'\s*#define\s+CONFIG_BUILD_FOR_HOST_UNIT_TEST\s+(\d+)'
    )
    match = re.search(pattern, content)

    assert match is not None, (
        "FAIL: Could not find #ifndef-guarded #define for "
        "CONFIG_BUILD_FOR_HOST_UNIT_TEST."
    )

    value = int(match.group(1))
    assert value == 1, (
        f"FAIL: CONFIG_BUILD_FOR_HOST_UNIT_TEST default value is {value}, "
        f"expected 1. The non-size-optimized build compiles chip-tool's "
        f"InteractionModel.cpp, which needs the test-only hooks gated by this "
        f"macro. It must be set here in the shared header (never as a per-target "
        f"override, which would cause an ODR layout mismatch across TUs)."
    )


def test_config_build_for_host_unit_test_no_unguarded_define():
    """
    **Validates: Requirements 4.1, 4.2**

    Property 7 (supplement): No unguarded #define

    There SHALL NOT be an unguarded (bare) #define CONFIG_BUILD_FOR_HOST_UNIT_TEST
    in the header. All definitions must be inside a #ifndef guard.
    """
    content = _read_file(CHIP_PROJECT_APP_CONFIG_FILE)
    lines = content.split("\n")

    for i, line in enumerate(lines):
        stripped = line.strip()
        if re.match(r'^#define\s+CONFIG_BUILD_FOR_HOST_UNIT_TEST\b', stripped):
            # This #define must be preceded by a #ifndef guard
            if i > 0:
                prev_line = lines[i - 1].strip()
                assert re.match(
                    r'^#ifndef\s+CONFIG_BUILD_FOR_HOST_UNIT_TEST', prev_line
                ), (
                    f"FAIL: Found unguarded #define CONFIG_BUILD_FOR_HOST_UNIT_TEST "
                    f"at line {i + 1}. It must be inside a #ifndef guard."
                )
            else:
                assert False, (
                    "FAIL: #define CONFIG_BUILD_FOR_HOST_UNIT_TEST is on the "
                    "first line with no preceding #ifndef guard."
                )


def test_build_gn_has_no_per_target_config_build_for_host_unit_test_override():
    """
    **Validates: Requirements 4.1**

    Property 7 (supplement): BUILD.gn must NOT override
    CONFIG_BUILD_FOR_HOST_UNIT_TEST per-target.

    A per-target `-DCONFIG_BUILD_FOR_HOST_UNIT_TEST=...` define reaches only the
    translation units of that target, while the rest of the link (core src/*)
    picks up the value from CHIPProjectAppConfig.h. Because this macro changes
    the layout of core types (CASESession, and thus the Server singleton and its
    FabricTable), such a split produces an ODR violation that crashes at startup
    in FabricTable::AddFabricDelegate. The value must come solely from the shared
    header.
    """
    content = _read_file(BUILD_GN_FILE)

    # Any occurrence of the macro name in active (non-comment) GN code is an
    # attempt to configure or define it, whether as a defines/cflags assignment
    # ("CONFIG_BUILD_FOR_HOST_UNIT_TEST=0") or a value-less define
    # ("-DCONFIG_BUILD_FOR_HOST_UNIT_TEST"). Since comments are stripped below,
    # a direct substring check catches every form. The macro name is still
    # allowed to appear in explanatory comments.
    offending = []
    for line in content.split("\n"):
        # Strip GN line comments so the macro name may appear in prose.
        code = line.split("#", 1)[0]
        if "CONFIG_BUILD_FOR_HOST_UNIT_TEST" in code:
            offending.append(line.strip())

    assert not offending, (
        "FAIL: BUILD.gn defines CONFIG_BUILD_FOR_HOST_UNIT_TEST per-target:\n  "
        + "\n  ".join(offending)
        + "\nThis macro must be set only in CHIPProjectAppConfig.h (the global "
        "project-config include), never as a per-target GN define, to avoid an "
        "ODR layout mismatch across translation units."
    )


# ---------------------------------------------------------------------------
# Allow running directly
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    import sys

    print("Running phase 2 build config property tests...")
    failed = False

    tests = [
        ("Property 8 - ICD deps conditional",
         test_icd_client_deps_inside_not_optimize_apk_size_block),
        ("Property 8 (supplement) - ICD deps not unconditional",
         test_icd_client_deps_not_unconditional),
        ("Property 7 - #ifndef guard",
         test_config_build_for_host_unit_test_has_ifndef_guard),
        ("Property 7 (supplement) - default value is 1",
         test_config_build_for_host_unit_test_default_value_is_1),
        ("Property 7 (supplement) - no unguarded define",
         test_config_build_for_host_unit_test_no_unguarded_define),
        ("Property 7 (supplement) - no per-target BUILD.gn override",
         test_build_gn_has_no_per_target_config_build_for_host_unit_test_override),
    ]

    for name, test_fn in tests:
        try:
            test_fn()
            print(f"  PASS: {name}")
        except AssertionError as e:
            print(f"  FAIL: {name}\n    {e}")
            failed = True
        except Exception as e:
            print(f"  ERROR: {name}\n    {e}")
            failed = True

    sys.exit(1 if failed else 0)
