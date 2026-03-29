"""
Property Test — Property 6: android.py casting build args correctness

Feature: casting-client-cluster-reduction
Property 6: android.py casting build args correctness

**Validates: Requirements 4.1, 4.2, 4.3**

This test parses `scripts/build/builders/android.py` and verifies that the
TV_CASTING_APP optimize_size block:

1. Sets `chip_tlv_decoder_attribute_source_override` to the slim attribute
   decoder path containing "casting-CHIPAttributeTLVValueDecoder.cpp"
2. Sets `chip_tlv_decoder_event_source_override` to the slim event decoder
   path containing "casting-CHIPEventTLVValueDecoder.cpp"
3. Does NOT set `matter_enable_tlv_decoder_cpp` to `False`
4. Still sets `chip_cluster_objects_source_override` (existing functionality)
"""

import os
import re

from hypothesis import HealthCheck, given, settings
from hypothesis import strategies as st

# -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -
# Paths
# -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -

REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
ANDROID_PY = os.path.join(REPO_ROOT, "scripts", "build", "builders", "android.py")

# -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -
# Helpers
# -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -


def _read_file(path: str) -> str:
    with open(path, "r") as f:
        return f.read()


def _extract_optimize_size_block(content: str) -> str:
    """
    Extract the body of the `if self.optimize_size:` block from the
    `generate()` method in android.py.

    Returns the indented block text.
    """
# Find the `if self.optimize_size :` line
    pattern = r"^(\s+)if self\.optimize_size:\s*$"
    m = re.search(pattern, content, re.MULTILINE)
    assert m is not None, (
        "Could not find `if self.optimize_size:` in android.py"
    )
    indent = m.group(1)
    block_indent = indent + "    "  # one level deeper

# Collect all lines that are part of this block(indented deeper or blank)
    lines = content[m.end():].split("\n")
    block_lines = []
    for line in lines:
        # Blank lines are part of the block
        if line.strip() == "":
            block_lines.append(line)
            continue
# Lines indented at block_indent level or deeper are part of the block
        if line.startswith(block_indent):
            block_lines.append(line)
            continue
# A line at the same or lesser indent ends the block
        break

    return "\n".join(block_lines)


def _extract_tv_casting_app_block(optimize_block: str) -> str:
    """
    Extract the body of the `if self.app == AndroidApp.TV_CASTING_APP:`
    block from within the optimize_size block.

    Returns the indented block text.
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
# Match multi - line string assignments using parentheses
    pattern = (
        rf'gn_args\[\s*"{re.escape(arg_name)}"\s*\]\s*=\s*'
        r'(\([\s\S]*?\)|"[^"]*"|[^\n]+)'
    )
    m = re.search(pattern, block)
    if m:
        return m.group(1).strip()
    return None

# -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -
# Property - based tests
# -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -


@given(
    override_arg=st.sampled_from([
        ("chip_tlv_decoder_attribute_source_override",
         "casting-CHIPAttributeTLVValueDecoder.cpp"),
        ("chip_tlv_decoder_event_source_override",
         "casting-CHIPEventTLVValueDecoder.cpp"),
    ])
)
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_tv_casting_app_sets_slim_decoder_overrides(override_arg):
    """
    **Validates: Requirements 4.2, 4.3**

    Property 6a: For any size-optimized TV_CASTING_APP build, android.py
    SHALL set `chip_tlv_decoder_attribute_source_override` and
    `chip_tlv_decoder_event_source_override` pointing to the slim decoder
    files.
    """
    arg_name, expected_filename = override_arg

    content = _read_file(ANDROID_PY)
    opt_block = _extract_optimize_size_block(content)
    casting_block = _extract_tv_casting_app_block(opt_block)

    value = _find_gn_arg_assignment(casting_block, arg_name)
    assert value is not None, (
        f"`gn_args[\"{arg_name}\"]` is not set in the "
        f"TV_CASTING_APP optimize_size block"
    )
    assert expected_filename in value, (
        f"`gn_args[\"{arg_name}\"]` is set to {value}, which does not "
        f"contain the expected filename `{expected_filename}`"
    )


@given(dummy=st.just(True))
@settings(
    max_examples=1,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_tv_casting_app_does_not_disable_tlv_decoder(dummy):
    """
    **Validates: Requirements 4.1**

    Property 6b: For any size-optimized TV_CASTING_APP build, android.py
    SHALL NOT set `matter_enable_tlv_decoder_cpp` to `False` in the
    TV_CASTING_APP block. The flag should not appear at all.
    """
    content = _read_file(ANDROID_PY)
    opt_block = _extract_optimize_size_block(content)
    casting_block = _extract_tv_casting_app_block(opt_block)

# Check that matter_enable_tlv_decoder_cpp is not set to False
    value = _find_gn_arg_assignment(casting_block, "matter_enable_tlv_decoder_cpp")
    assert value is None, (
        f"`gn_args[\"matter_enable_tlv_decoder_cpp\"]` is set to {value} in the "
        f"TV_CASTING_APP optimize_size block — it should not appear at all "
        f"(TLV decoding must remain enabled)"
    )


@given(dummy=st.just(True))
@settings(
    max_examples=1,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_tv_casting_app_preserves_cluster_objects_override(dummy):
    """
    **Validates: Requirements 4.2, 4.3**

    Property 6c: The TV_CASTING_APP optimize_size block SHALL still set
    `chip_cluster_objects_source_override` (existing functionality preserved).
    """
    content = _read_file(ANDROID_PY)
    opt_block = _extract_optimize_size_block(content)
    casting_block = _extract_tv_casting_app_block(opt_block)

    value = _find_gn_arg_assignment(casting_block, "chip_cluster_objects_source_override")
    assert value is not None, (
        "`gn_args[\"chip_cluster_objects_source_override\"]` is not set in the "
        "TV_CASTING_APP optimize_size block — existing functionality must be preserved"
    )
    assert "casting-cluster-objects.cpp" in value, (
        f"`gn_args[\"chip_cluster_objects_source_override\"]` is set to {value}, "
        f"which does not reference `casting-cluster-objects.cpp`"
    )


@given(
    override_arg=st.sampled_from([
        "chip_tlv_decoder_attribute_source_override",
        "chip_tlv_decoder_event_source_override",
    ])
)
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_slim_decoder_paths_use_correct_gn_prefix(override_arg):
    """
    **Validates: Requirements 4.2, 4.3**

    Property 6d: The slim decoder override paths SHALL use the
    `//third_party/connectedhomeip/examples/` GN path prefix, matching
    the pattern used by `chip_cluster_objects_source_override`.
    """
    content = _read_file(ANDROID_PY)
    opt_block = _extract_optimize_size_block(content)
    casting_block = _extract_tv_casting_app_block(opt_block)

    value = _find_gn_arg_assignment(casting_block, override_arg)
    assert value is not None, (
        f"`gn_args[\"{override_arg}\"]` is not set in the "
        f"TV_CASTING_APP optimize_size block"
    )
    assert "//third_party/connectedhomeip/examples/" in value, (
        f"`gn_args[\"{override_arg}\"]` path does not use the expected "
        f"`//third_party/connectedhomeip/examples/` GN prefix. Got: {value}"
    )
    assert "tv-casting-app/tv-casting-common/" in value, (
        f"`gn_args[\"{override_arg}\"]` path does not reference the "
        f"`tv-casting-app/tv-casting-common/` directory. Got: {value}"
    )


# -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -
# Allow running directly
# -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -
if __name__ == "__main__":
    import sys

    print("Running android.py casting build args property tests...")
    tests = [
        ("6a: TV_CASTING_APP sets slim decoder overrides",
         test_tv_casting_app_sets_slim_decoder_overrides),
        ("6b: TV_CASTING_APP does not disable TLV decoder",
         test_tv_casting_app_does_not_disable_tlv_decoder),
        ("6c: TV_CASTING_APP preserves cluster objects override",
         test_tv_casting_app_preserves_cluster_objects_override),
        ("6d: Slim decoder paths use correct GN prefix",
         test_slim_decoder_paths_use_correct_gn_prefix),
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
