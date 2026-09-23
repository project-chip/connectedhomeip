"""
Property Test — Property 3: android.py passes only the consolidated override arg

Feature: consolidate-overrides-and-generation-script

**Validates: Requirements 6.1, 6.2, 14.3**

Parse `android.py` and verify the TV_CASTING_APP optimize_size block sets
`chip_data_model_overrides_dir` and does NOT set any of the 5 individual
per-file override args.
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

# ---------------------------------------------------------------------------
# The 5 individual per-file override args that should NOT be set
# ---------------------------------------------------------------------------

REMOVED_OVERRIDE_ARGS = [
    "chip_tlv_decoder_attribute_source_override",
    "chip_tlv_decoder_event_source_override",
    "chip_cluster_objects_source_override",
    "chip_cluster_server_source_override",
    "chip_accessors_source_override",
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


# ---------------------------------------------------------------------------
# Property-based tests
# ---------------------------------------------------------------------------


@given(dummy=st.just(True))
@settings(
    max_examples=1,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_tv_casting_app_sets_chip_data_model_overrides_dir(dummy):
    """
    **Validates: Requirements 6.1, 14.3**

    Property 3a: For a size-optimized TV_CASTING_APP build, android.py
    SHALL set `chip_data_model_overrides_dir` pointing to the
    tv-casting-common directory.
    """
    content = _read_file(ANDROID_PY)
    opt_block = _extract_optimize_size_block(content)
    casting_block = _extract_tv_casting_app_block(opt_block)

    value = _find_gn_arg_assignment(casting_block, "chip_data_model_overrides_dir")
    assert value is not None, (
        '`gn_args["chip_data_model_overrides_dir"]` is not set in the '
        "TV_CASTING_APP optimize_size block of android.py"
    )
    assert "tv-casting-app/tv-casting-common" in value, (
        f'`gn_args["chip_data_model_overrides_dir"]` is set to {value}, '
        f"which does not reference tv-casting-app/tv-casting-common"
    )


@given(
    removed_arg=st.sampled_from(REMOVED_OVERRIDE_ARGS),
)
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_tv_casting_app_does_not_set_individual_override_args(removed_arg):
    """
    **Validates: Requirements 6.2, 14.3**

    Property 3b: For a size-optimized TV_CASTING_APP build, android.py
    SHALL NOT set any of the 5 individual per-file override GN args.
    """
    content = _read_file(ANDROID_PY)
    opt_block = _extract_optimize_size_block(content)
    casting_block = _extract_tv_casting_app_block(opt_block)

    value = _find_gn_arg_assignment(casting_block, removed_arg)
    assert value is None, (
        f'REGRESSION: `gn_args["{removed_arg}"]` is set to {value} in the '
        f"TV_CASTING_APP optimize_size block — it should have been replaced "
        f"by chip_data_model_overrides_dir"
    )


# ---------------------------------------------------------------------------
# Allow running directly
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    import sys

    print("Running android.py consolidated arg property tests...")
    tests = [
        ("Property 3a: TV_CASTING_APP sets chip_data_model_overrides_dir",
         test_tv_casting_app_sets_chip_data_model_overrides_dir),
        ("Property 3b: TV_CASTING_APP does not set individual override args",
         test_tv_casting_app_does_not_set_individual_override_args),
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
