"""
Property Test — Property 5 + 6 preservation: Non-casting builds unchanged

Feature: casting-client-cluster-reduction
Property 5+6 preservation: Non-casting builds unchanged

**Validates: Requirements 6.1, 6.2, 6.3, 6.4, 6.5**

This test verifies that the GN build configuration preserves correct defaults
for non-casting builds:

1. `config.gni` still declares `matter_enable_tlv_decoder_cpp` with default `true`
2. `config.gni` declares `chip_tlv_decoder_attribute_source_override` with default `""`
3. `config.gni` declares `chip_tlv_decoder_event_source_override` with default `""`
4. `args.gni` default block (else branch) does NOT set the override args
5. `args.gni` default block does NOT set `matter_enable_tlv_decoder_cpp` to false

These properties ensure that non-casting Android builds and non-Android builds
remain completely unaffected by the slim decoder feature.
"""

import os
import re

from hypothesis import HealthCheck, given, settings
from hypothesis import strategies as st

# -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -
# Paths
# -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -

REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
CONFIG_GNI = os.path.join(REPO_ROOT, "build", "chip", "java", "config.gni")
ARGS_GNI = os.path.join(REPO_ROOT, "examples", "tv-casting-app", "android", "args.gni")

# -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -
# Helpers
# -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -


def _read_file(path: str) -> str:
    with open(path, "r") as f:
        return f.read()


def _strip_comments(text: str) -> str:
    """Remove single-line # comments."""
    return re.sub(r"#[^\n]*", "", text)


def _extract_declare_args_block(text: str) -> str:
    r"""
    Extract the body of the `declare_args()
{
    ...
}
` block from config.gni.""
                        "
    pattern = r "declare_args\s*\(\s*\)\s*\{" m           = re.search(pattern, text) assert m is not None,
    "Could not find `declare_args()` in config.gni" depth = 1 pos =
        m.end() while pos < len(text) and depth > 0 : if text[pos] == "{" : depth += 1 elif text[pos] == "}" : depth -= 1 pos +=
    1 return text [m.end():pos - 1]

    def
    _find_assignment_in_block(block : str, var_name : str) -> str |
    None : ""
           "
           Find a `var_name = <value>` assignment in the block.Returns the RHS value as a string,
    or
    None if not found.""
                      "
    pattern = rf "^\s*{re.escape(var_name)}\s*=\s*(.+)" m = re.search(pattern, block, re.MULTILINE) if m
    : return m.group(1)
          .strip() return None

              def _extract_else_block(text : str)
          ->str : ""
                  "
                  Extract the body of the `
}
else
{` block that follows the
    `if (optimize_apk_size)` conditional in android/args.gni.
    """
    stripped = _strip_comments(text)

    match = re.search(r"if\s*\(\s*optimize_apk_size\s*\)", stripped)
    if not match:
        return ""

# Walk past the if - block's opening brace
    brace_start = stripped.index("{", match.end())
    depth = 1
    pos = brace_start + 1
    while pos < len(stripped) and depth > 0:
        if stripped[pos] == "{":
            depth += 1
        elif stripped[pos] == "}":
            depth -= 1
        pos += 1

# pos is now just past the closing } of the if - block.
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

    return stripped[else_body_start: epos - 1]

# -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -
# Property - based tests — config.gni declarations
# -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -


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
def test_config_gni_declares_override_args_with_empty_default(override_arg):
    """
    **Validates: Requirements 6.1, 6.2**

    Property 5+6 preservation (a): config.gni SHALL declare both
    `chip_tlv_decoder_attribute_source_override` and
    `chip_tlv_decoder_event_source_override` with default empty string `""`.
    When empty, the GN build compiles the full zap-generated decoders.
    """
    content = _read_file(CONFIG_GNI)
    block = _extract_declare_args_block(content)

    value = _find_assignment_in_block(block, override_arg)
    assert value is not None, (
        f"`{override_arg}` is not declared in the declare_args() block "
        f"of config.gni"
    )
# The default should be an empty string : ""
    assert value == '""', (
        f"`{override_arg}` has default `{value}` instead of `\"\"` in "
        f"config.gni — non-casting builds require the empty default"
    )


@given(dummy=st.just(True))
@settings(
    max_examples=1,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_config_gni_preserves_matter_enable_tlv_decoder_cpp_true(dummy):
    """
    **Validates: Requirements 6.5**

    Property 5+6 preservation (b): config.gni SHALL still declare
    `matter_enable_tlv_decoder_cpp` with default `true`. This flag must
    remain available for other use cases and default to enabling C++ TLV
    decoding.
    """
    content = _read_file(CONFIG_GNI)
    block = _extract_declare_args_block(content)

    value = _find_assignment_in_block(block, "matter_enable_tlv_decoder_cpp")
    assert value is not None, (
        "`matter_enable_tlv_decoder_cpp` is not declared in the "
        "declare_args() block of config.gni"
    )
    assert value == "true", (
        f"`matter_enable_tlv_decoder_cpp` has default `{value}` instead of "
        f"`true` in config.gni — this flag must default to true to preserve "
        f"existing behavior for all non-casting builds"
    )

# -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -
# Property - based tests — args.gni default(else) block
# -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -


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
def test_args_gni_default_block_does_not_set_override_args(override_arg):
    """
    **Validates: Requirements 6.3, 6.4**

    Property 5+6 preservation (c): The default (else) block in args.gni
    (when optimize_apk_size is false) SHALL NOT set
    `chip_tlv_decoder_attribute_source_override` or
    `chip_tlv_decoder_event_source_override`. Non-optimized builds must
    use the full zap-generated decoders via the empty-string default.
    """
    content = _read_file(ARGS_GNI)
    else_block = _extract_else_block(content)
    assert else_block, (
        "Could not find the `else` block after `if (optimize_apk_size)` "
        "in android/args.gni"
    )

    value = _find_assignment_in_block(else_block, override_arg)
    assert value is None, (
        f"REGRESSION: `{override_arg}` is set to `{value}` in the default "
        f"(else) block of args.gni — it should only be set in the "
        f"optimize_apk_size block"
    )


@given(dummy=st.just(True))
@settings(
    max_examples=1,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_args_gni_default_block_does_not_disable_tlv_decoder(dummy):
    """
    **Validates: Requirements 6.3, 6.5**

    Property 5+6 preservation (d): The default (else) block in args.gni
    SHALL NOT set `matter_enable_tlv_decoder_cpp` to false. Non-optimized
    builds must keep C++ TLV decoding enabled (the config.gni default).
    """
    content = _read_file(ARGS_GNI)
    else_block = _extract_else_block(content)
    assert else_block, (
        "Could not find the `else` block after `if (optimize_apk_size)` "
        "in android/args.gni"
    )

    value = _find_assignment_in_block(else_block, "matter_enable_tlv_decoder_cpp")
    assert value is None, (
        f"REGRESSION: `matter_enable_tlv_decoder_cpp` is set to `{value}` "
        f"in the default (else) block of args.gni — it should not be "
        f"overridden for default development builds"
    )


# -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -
# Allow running directly
# -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -
if __name__ == "__main__":
    import sys

    print("Running preservation of non-casting builds property tests...")
    tests = [
        ("5+6a: config.gni declares override args with empty default",
         test_config_gni_declares_override_args_with_empty_default),
        ("5+6b: config.gni preserves matter_enable_tlv_decoder_cpp = true",
         test_config_gni_preserves_matter_enable_tlv_decoder_cpp_true),
        ("5+6c: args.gni default block does not set override args",
         test_args_gni_default_block_does_not_set_override_args),
        ("5+6d: args.gni default block does not disable TLV decoder",
         test_args_gni_default_block_does_not_disable_tlv_decoder),
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
