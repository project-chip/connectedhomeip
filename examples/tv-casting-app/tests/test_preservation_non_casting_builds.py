"""
Property Test — Preservation: Non-casting builds unchanged

Feature: consolidate-overrides-and-generation-script
Preservation: Non-casting builds unchanged

**Validates: Requirements 14.1, 14.2**

This test verifies that the GN build configuration preserves correct defaults
for non-casting builds after the consolidation of override args into
`chip_data_model_overrides_dir`:

1. `common_flags.gni` declares `chip_data_model_overrides_dir` with default `""`
2. `args.gni` default block (else branch) does NOT set `chip_data_model_overrides_dir`

These properties ensure that non-casting Android builds and non-Android builds
remain completely unaffected by the override directory mechanism.
"""

import os
import re
from pathlib import Path

from hypothesis import HealthCheck, given, settings
from hypothesis import strategies as st

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------

REPO_ROOT = next(filter(lambda p: (p / 'SPECIFICATION_VERSION').is_file(), Path(__file__).parents))
COMMON_FLAGS_GNI = os.path.join(REPO_ROOT, "src", "app", "common_flags.gni")
ARGS_GNI = os.path.join(REPO_ROOT, "examples", "tv-casting-app", "android", "args.gni")

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


def _read_file(path: str) -> str:
    with open(path, "r") as f:
        return f.read()


def _strip_comments(text: str) -> str:
    """Remove single-line # comments."""
    return re.sub(r"#[^\n]*", "", text)


def _extract_declare_args_block(text: str) -> str:
    r"""
    Extract the body of the `declare_args() { ... }` block.
    """
    pattern = r"declare_args\s*\(\s*\)\s*\{"
    m = re.search(pattern, text)
    assert m is not None, "Could not find `declare_args()` block"
    depth = 1
    pos = m.end()
    while pos < len(text) and depth > 0:
        if text[pos] == "{":
            depth += 1
        elif text[pos] == "}":
            depth -= 1
        pos += 1
    return text[m.end():pos - 1]


def _find_assignment_in_block(block: str, var_name: str) -> str | None:
    """
    Find a `var_name = <value>` assignment in the block. Returns the RHS
    value as a string, or None if not found.
    """
    pattern = rf"^\s*{re.escape(var_name)}\s*=\s*(.+)"
    m = re.search(pattern, block, re.MULTILINE)
    if m:
        return m.group(1).strip()
    return None


def _extract_else_block(text: str) -> str:
    """
    Extract the body of the `} else {` block that follows the
    `if (optimize_apk_size)` conditional in android/args.gni.
    """
    stripped = _strip_comments(text)

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

# ---------------------------------------------------------------------------
# Property-based tests — common_flags.gni declarations
# ---------------------------------------------------------------------------


@given(dummy=st.just(True))
@settings(
    max_examples=1,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_common_flags_gni_declares_chip_data_model_overrides_dir(dummy):
    """
    **Validates: Requirements 14.1**

    Preservation (a): common_flags.gni SHALL declare
    `chip_data_model_overrides_dir` with default empty string `""`.
    When empty, the GN build uses the full zap-generated sources.
    """
    content = _read_file(COMMON_FLAGS_GNI)
    block = _extract_declare_args_block(content)

    value = _find_assignment_in_block(block, "chip_data_model_overrides_dir")
    assert value is not None, (
        "`chip_data_model_overrides_dir` is not declared in the "
        "declare_args() block of common_flags.gni"
    )
    # The default should be an empty string: ""
    assert value == '""', (
        f"`chip_data_model_overrides_dir` has default `{value}` instead of "
        f'`""` in common_flags.gni — non-casting builds require the empty default'
    )


# ---------------------------------------------------------------------------
# Property-based tests — args.gni default (else) block
# ---------------------------------------------------------------------------


@given(dummy=st.just(True))
@settings(
    max_examples=1,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_args_gni_default_block_does_not_set_overrides_dir(dummy):
    """
    **Validates: Requirements 14.2**

    Preservation (b): The default (else) block in args.gni
    (when optimize_apk_size is false) SHALL NOT set
    `chip_data_model_overrides_dir`. Non-optimized builds must
    use the full zap-generated sources via the empty-string default.
    """
    content = _read_file(ARGS_GNI)
    else_block = _extract_else_block(content)
    assert else_block, (
        "Could not find the `else` block after `if (optimize_apk_size)` "
        "in android/args.gni"
    )

    value = _find_assignment_in_block(else_block, "chip_data_model_overrides_dir")
    assert value is None, (
        f"REGRESSION: `chip_data_model_overrides_dir` is set to `{value}` "
        f"in the default (else) block of args.gni — it should only be set "
        f"in the optimize_apk_size block"
    )


# ---------------------------------------------------------------------------
# Allow running directly
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    import sys

    print("Running preservation of non-casting builds property tests...")
    tests = [
        ("a: common_flags.gni declares chip_data_model_overrides_dir with empty default",
         test_common_flags_gni_declares_chip_data_model_overrides_dir),
        ("b: args.gni default block does not set chip_data_model_overrides_dir",
         test_args_gni_default_block_does_not_set_overrides_dir),
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
