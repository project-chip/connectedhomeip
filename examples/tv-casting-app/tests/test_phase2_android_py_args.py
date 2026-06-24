"""
Property Test — Property 6: android.py passes phase 2 args for TV_CASTING_APP only

Feature: casting-apk-size-reduction-phase2, Property 6: android.py passes
phase 2 args for TV_CASTING_APP only

**Validates: Requirements 3.1, 5.1, 7.1, 7.2, 7.3, 8.2**

This test parses `scripts/build/builders/android.py` and verifies that
`chip_data_model_extra_logging` and `enable_rtti` are set to False within
the TV_CASTING_APP + optimize_size scope, and are NOT set outside that scope.
"""

import os
import re

from hypothesis import HealthCheck, given, settings
from hypothesis import strategies as st

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------

REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
ANDROID_PY = os.path.join(REPO_ROOT, "scripts", "build", "builders", "android.py")

# Phase 2 GN args that must be set only for TV_CASTING_APP + optimize_size
PHASE2_ARGS = [
    ("chip_data_model_extra_logging", "False"),
    ("enable_rtti", "False"),
]

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


def _read_file(path: str) -> str:
    with open(path, "r") as f:
        return f.read()


def _extract_optimize_size_block(content: str) -> str:
    """
    Extract the body of the `if self.optimize_size:` block from the
    `generate()` method in android.py.
    """
    pattern = r"^(\s+)if self\.optimize_size:\s*$"
    m = re.search(pattern, content, re.MULTILINE)
    assert m is not None, (
        "Could not find `if self.optimize_size:` in android.py"
    )
    indent = m.group(1)
    block_indent = indent + "    "

    lines = content[m.end():].split("\n")
    block_lines = []
    for line in lines:
        if line.strip() == "":
            block_lines.append(line)
            continue
        if line.startswith(block_indent):
            block_lines.append(line)
            continue
        break

    return "\n".join(block_lines)


def _extract_tv_casting_app_block(optimize_block: str) -> str:
    """
    Extract the body of the `if self.app == AndroidApp.TV_CASTING_APP:`
    block from within the optimize_size block.
    """
    pattern = r"^(\s+)if self\.app\s*==\s*AndroidApp\.TV_CASTING_APP:\s*$"
    m = re.search(pattern, optimize_block, re.MULTILINE)
    assert m is not None, (
        "Could not find `if self.app == AndroidApp.TV_CASTING_APP:` "
        "inside the optimize_size block"
    )
    indent = m.group(1)
    block_indent = indent + "    "

    lines = optimize_block[m.end():].split("\n")
    block_lines = []
    for line in lines:
        if line.strip() == "":
            block_lines.append(line)
            continue
        if line.startswith(block_indent):
            block_lines.append(line)
            continue
        break

    return "\n".join(block_lines)


def _find_gn_arg_assignment(block: str, arg_name: str) -> str | None:
    """
    Find a `gn_args["<arg_name>"] = <value>` assignment in the block.
    Returns the RHS value as a string, or None if not found.
    """
    pattern = (
        rf'gn_args\[\s*"{re.escape(arg_name)}"\s*\]\s*=\s*'
        r'(\([\s\S]*?\)|"[^"]*"|[^\n]+)'
    )
    m = re.search(pattern, block)
    if m:
        return m.group(1).strip()
    return None


def _remove_block(content: str, block_start_pattern: str) -> str:
    """
    Remove an indented block (the header line + all deeper-indented lines)
    from content. Used to get the 'outside' portion of a scope.
    """
    m = re.search(block_start_pattern, content, re.MULTILINE)
    if m is None:
        return content

    indent = m.group(1)
    block_indent = indent + "    "
    before = content[:m.start()]

    lines = content[m.end():].split("\n")
    after_lines = []
    in_block = True
    for line in lines:
        if in_block:
            if line.strip() == "" or line.startswith(block_indent):
                continue
            in_block = False
        after_lines.append(line)

    return before + "\n".join(after_lines)


# ---------------------------------------------------------------------------
# Property-based tests
# ---------------------------------------------------------------------------


@given(
    phase2_arg=st.sampled_from(PHASE2_ARGS),
)
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_phase2_args_set_in_tv_casting_app_optimize_block(phase2_arg):
    """
    **Validates: Requirements 3.1, 5.1, 7.1, 7.2**

    Property 6a: For any phase 2 GN argument, android.py SHALL set that
    argument within the TV_CASTING_APP block inside the optimize_size
    conditional, with the value False.
    """
    arg_name, expected_value = phase2_arg

    content = _read_file(ANDROID_PY)
    opt_block = _extract_optimize_size_block(content)
    casting_block = _extract_tv_casting_app_block(opt_block)

    value = _find_gn_arg_assignment(casting_block, arg_name)
    assert value is not None, (
        f'`gn_args["{arg_name}"]` is not set in the '
        f"TV_CASTING_APP optimize_size block"
    )
    assert value == expected_value, (
        f'`gn_args["{arg_name}"]` is set to {value}, '
        f"expected {expected_value}"
    )


@given(
    phase2_arg=st.sampled_from([name for name, _ in PHASE2_ARGS]),
)
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_phase2_args_not_set_outside_tv_casting_app_scope(phase2_arg):
    """
    **Validates: Requirements 7.3, 8.2**

    Property 6b: For any phase 2 GN argument, android.py SHALL NOT set
    that argument outside the TV_CASTING_APP + optimize_size scope.
    This ensures non-casting-app targets are unaffected.
    """
    content = _read_file(ANDROID_PY)

    # Get the optimize_size block and remove the TV_CASTING_APP sub-block
    opt_block = _extract_optimize_size_block(content)
    outside_casting = _remove_block(
        opt_block,
        r"^(\s+)if self\.app\s*==\s*AndroidApp\.TV_CASTING_APP:\s*$",
    )

    value = _find_gn_arg_assignment(outside_casting, phase2_arg)
    assert value is None, (
        f'`gn_args["{phase2_arg}"]` is set to {value} in the '
        f"optimize_size block but OUTSIDE the TV_CASTING_APP scope — "
        f"phase 2 args must only apply to TV_CASTING_APP"
    )

    # Also check the full file outside the optimize_size block
    full_outside = _remove_block(
        content,
        r"^(\s+)if self\.optimize_size:\s*$",
    )
    value_outside = _find_gn_arg_assignment(full_outside, phase2_arg)
    assert value_outside is None, (
        f'`gn_args["{phase2_arg}"]` is set to {value_outside} '
        f"OUTSIDE the optimize_size block entirely — "
        f"phase 2 args must only apply when optimize_size is true"
    )


# ---------------------------------------------------------------------------
# Allow running directly
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    import sys

    print("Running phase 2 android.py args property tests...")
    tests = [
        ("6a: Phase 2 args set in TV_CASTING_APP optimize block",
         test_phase2_args_set_in_tv_casting_app_optimize_block),
        ("6b: Phase 2 args not set outside TV_CASTING_APP scope",
         test_phase2_args_not_set_outside_tv_casting_app_scope),
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
