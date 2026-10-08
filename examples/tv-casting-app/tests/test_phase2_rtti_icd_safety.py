"""
Phase 2 RTTI & ICD Safety Property Tests

Feature: casting-apk-size-reduction-phase2

Property 9: No RTTI usage in casting common C++ sources
**Validates: Requirements 5.3**

Property 10: No ICD client API usage in casting common sources
**Validates: Requirements 6.3**

This test verifies that:
  - No .cpp or .h file in tv-casting-common/ (excluding darwin/ bridge)
    uses dynamic_cast or typeid operators, confirming RTTI can be safely
    disabled for Android builds.
  - No .cpp or .h file in tv-casting-common/core/ or tv-casting-common/support/
    contains #include directives referencing icd/client/ headers, confirming
    the ICD client dependencies can be safely removed.
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

TV_CASTING_COMMON_DIR = os.path.join(
    REPO_ROOT,
    "examples",
    "tv-casting-app",
    "tv-casting-common",
)

CORE_DIR = os.path.join(TV_CASTING_COMMON_DIR, "core")
SUPPORT_DIR = os.path.join(TV_CASTING_COMMON_DIR, "support")

# ---------------------------------------------------------------------------
# File collection helpers
# ---------------------------------------------------------------------------


def _collect_cpp_h_files(root_dir, exclude_dirs=None):
    """Collect all .cpp and .h files under root_dir, excluding specified dirs."""
    if exclude_dirs is None:
        exclude_dirs = set()
    files = []
    for dirpath, dirnames, filenames in os.walk(root_dir):
        # Skip excluded directories
        dirnames[:] = [
            d for d in dirnames
            if os.path.join(dirpath, d) not in exclude_dirs
        ]
        for fname in filenames:
            if fname.endswith(".cpp") or fname.endswith(".h"):
                files.append(os.path.join(dirpath, fname))
    return sorted(files)


# ---------------------------------------------------------------------------
# Build file lists
# ---------------------------------------------------------------------------

# Property 9: All .cpp/.h in tv-casting-common/ excluding darwin/ bridge
# (darwin/ is at examples/tv-casting-app/darwin/, not under tv-casting-common/,
# but we exclude it defensively in case the layout changes)
_DARWIN_EXCLUDE = os.path.join(TV_CASTING_COMMON_DIR, "darwin")
ALL_CASTING_COMMON_CPP_H = _collect_cpp_h_files(
    TV_CASTING_COMMON_DIR, exclude_dirs={_DARWIN_EXCLUDE}
)

# Property 10: .cpp/.h in core/ and support/ only
CORE_SUPPORT_CPP_H = _collect_cpp_h_files(CORE_DIR) + _collect_cpp_h_files(SUPPORT_DIR)

# RTTI patterns: match dynamic_cast<...> or typeid(...) as C++ operators
# Use word boundary to avoid matching inside comments or string literals
# that happen to contain these as substrings of other identifiers.
RTTI_PATTERN = re.compile(r'\bdynamic_cast\s*<|\btypeid\s*\(')

# ICD client include pattern: #include with icd/client/ path
ICD_CLIENT_INCLUDE_PATTERN = re.compile(r'#\s*include\s+[<"].*icd/client/.*[>"]')


# ---------------------------------------------------------------------------
# Property 9: No RTTI usage in casting common C++ sources
# ---------------------------------------------------------------------------

# Guard: hypothesis needs at least one element in sampled_from
assert len(ALL_CASTING_COMMON_CPP_H) > 0, (
    "No .cpp or .h files found in tv-casting-common/. "
    "Check that the directory exists and contains source files."
)


@given(filepath=st.sampled_from(ALL_CASTING_COMMON_CPP_H))
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_no_rtti_usage_in_casting_common(filepath):
    """
    **Validates: Requirements 5.3**

    Property 9: No RTTI usage in casting common C++ sources

    For any .cpp or .h file in tv-casting-common/ (excluding darwin/ bridge),
    the file SHALL NOT contain dynamic_cast or typeid operators, confirming
    RTTI can be safely disabled for Android builds.
    """
    with open(filepath, "r") as f:
        content = f.read()

    # Strip single-line comments to reduce false positives
    content_no_comments = re.sub(r'//.*$', '', content, flags=re.MULTILINE)
    # Strip multi-line comments
    content_no_comments = re.sub(r'/\*.*?\*/', '', content_no_comments, flags=re.DOTALL)

    matches = list(RTTI_PATTERN.finditer(content_no_comments))
    rel_path = os.path.relpath(filepath, REPO_ROOT)

    assert len(matches) == 0, (
        f"FAIL: RTTI usage found in {rel_path}. "
        f"Found {len(matches)} occurrence(s) of dynamic_cast or typeid. "
        f"RTTI must not be used in casting common sources so that "
        f"enable_rtti=false can be safely applied."
    )


# ---------------------------------------------------------------------------
# Property 10: No ICD client API usage in casting common sources
# ---------------------------------------------------------------------------

# Guard: hypothesis needs at least one element in sampled_from
assert len(CORE_SUPPORT_CPP_H) > 0, (
    "No .cpp or .h files found in tv-casting-common/core/ or support/. "
    "Check that the directories exist and contain source files."
)


@given(filepath=st.sampled_from(CORE_SUPPORT_CPP_H))
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_no_icd_client_includes_in_core_support(filepath):
    """
    **Validates: Requirements 6.3**

    Property 10: No ICD client API usage in casting common sources

    For any .cpp or .h file in tv-casting-common/core/ or
    tv-casting-common/support/, the file SHALL NOT contain #include
    directives referencing icd/client/ headers, confirming the ICD client
    dependencies can be safely removed.
    """
    with open(filepath, "r") as f:
        content = f.read()

    matches = list(ICD_CLIENT_INCLUDE_PATTERN.finditer(content))
    rel_path = os.path.relpath(filepath, REPO_ROOT)

    assert len(matches) == 0, (
        f"FAIL: ICD client #include found in {rel_path}. "
        f"Found {len(matches)} #include directive(s) referencing icd/client/ headers. "
        f"ICD client APIs must not be used in casting common sources so that "
        f"the ICD client dependencies can be safely removed."
    )


# ---------------------------------------------------------------------------
# Allow running directly
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    import sys

    print("Running phase 2 RTTI & ICD safety property tests...")
    failed = False

    tests = [
        ("Property 9 - No RTTI usage", test_no_rtti_usage_in_casting_common),
        ("Property 10 - No ICD client includes", test_no_icd_client_includes_in_core_support),
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
