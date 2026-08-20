"""
Property Test — Property 1: Removed GN args are not declared

Feature: consolidate-overrides-and-generation-script

**Validates: Requirements 2.1, 2.2, 2.3, 2.4, 2.5, 2.6**

For each of the 6 removed GN arguments, parse the corresponding `.gni` file
and verify the arg is NOT in any `declare_args()` block.

Removed args:
- chip_tlv_decoder_attribute_source_override (was in config.gni)
- chip_tlv_decoder_event_source_override (was in config.gni)
- matter_enable_tlv_decoder_cpp (was in config.gni)
- chip_cluster_objects_source_override (was in common_flags.gni)
- chip_cluster_server_source_override (was in common_flags.gni)
- chip_accessors_source_override (was in common_flags.gni)
"""

import os
import re

from hypothesis import HealthCheck, given, settings
from hypothesis import strategies as st

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------

REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
CONFIG_GNI = os.path.join(REPO_ROOT, "build", "chip", "java", "config.gni")
COMMON_FLAGS_GNI = os.path.join(REPO_ROOT, "src", "app", "common_flags.gni")

# ---------------------------------------------------------------------------
# Mapping: removed arg -> file it was declared in
# ---------------------------------------------------------------------------

REMOVED_ARGS_CONFIG_GNI = [
    "chip_tlv_decoder_attribute_source_override",
    "chip_tlv_decoder_event_source_override",
    "matter_enable_tlv_decoder_cpp",
]

REMOVED_ARGS_COMMON_FLAGS_GNI = [
    "chip_cluster_objects_source_override",
    "chip_cluster_server_source_override",
    "chip_accessors_source_override",
]

ALL_REMOVED_ARGS = [
    ("chip_tlv_decoder_attribute_source_override", CONFIG_GNI),
    ("chip_tlv_decoder_event_source_override", CONFIG_GNI),
    ("matter_enable_tlv_decoder_cpp", CONFIG_GNI),
    ("chip_cluster_objects_source_override", COMMON_FLAGS_GNI),
    ("chip_cluster_server_source_override", COMMON_FLAGS_GNI),
    ("chip_accessors_source_override", COMMON_FLAGS_GNI),
]

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


def _read_file(path: str) -> str:
    with open(path, "r") as f:
        return f.read()


def _extract_all_declare_args_blocks(text: str) -> list[str]:
    """
    Extract the bodies of ALL `declare_args() { ... }` blocks from a file.
    Returns a list of block body strings.
    """
    blocks = []
    pattern = r"declare_args\s*\(\s*\)\s*\{"
    for m in re.finditer(pattern, text):
        depth = 1
        pos = m.end()
        while pos < len(text) and depth > 0:
            if text[pos] == "{":
                depth += 1
            elif text[pos] == "}":
                depth -= 1
            pos += 1
        blocks.append(text[m.end():pos - 1])
    return blocks


def _arg_declared_in_blocks(blocks: list[str], arg_name: str) -> bool:
    """
    Check if `arg_name` appears as a declaration (assignment) in any of
    the declare_args blocks.
    """
    pattern = rf"^\s*{re.escape(arg_name)}\s*="
    for block in blocks:
        if re.search(pattern, block, re.MULTILINE):
            return True
    return False


# ---------------------------------------------------------------------------
# Property-based test
# ---------------------------------------------------------------------------


@given(
    removed_arg_entry=st.sampled_from(ALL_REMOVED_ARGS),
)
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_removed_gn_args_not_declared(removed_arg_entry):
    """
    **Validates: Requirements 2.1, 2.2, 2.3, 2.4, 2.5, 2.6**

    Property 1: For any of the 6 removed GN arguments, the argument SHALL
    NOT appear as a declaration in its original `declare_args()` block.
    """
    arg_name, gni_file = removed_arg_entry

    content = _read_file(gni_file)
    blocks = _extract_all_declare_args_blocks(content)

    assert not _arg_declared_in_blocks(blocks, arg_name), (
        f"REGRESSION: `{arg_name}` is still declared in a declare_args() block "
        f"in {os.path.relpath(gni_file, REPO_ROOT)} — it should have been "
        f"removed as part of the consolidation to chip_data_model_overrides_dir"
    )


# ---------------------------------------------------------------------------
# Allow running directly
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    import sys

    print("Running removed GN args property test...")
    try:
        test_removed_gn_args_not_declared()
        print("  PASS: Property 1 — removed GN args not declared")
    except AssertionError as e:
        print(f"  FAIL: Property 1\n    {e}")
        sys.exit(1)
    except Exception as e:
        print(f"  ERROR: Property 1\n    {e}")
        sys.exit(1)

    sys.exit(0)
