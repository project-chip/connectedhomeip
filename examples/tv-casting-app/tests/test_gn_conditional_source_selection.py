"""
Property Test — Property 5: GN conditional source selection

**Validates: Requirements 3.3, 3.4, 3.5, 6.1, 6.2**

This test parses `src/controller/java/BUILD.gn` and verifies that the
`if (matter_enable_tlv_decoder_cpp)` block contains conditional logic for
both `chip_tlv_decoder_attribute_source_override` and
`chip_tlv_decoder_event_source_override`, following the pattern:

    if (override != "") { use override } else { use zap-generated }

This ensures that non-empty overrides compile the slim file, while empty
(default) overrides compile the full zap-generated decoder.
"""

import os
import re

from hypothesis import HealthCheck, given, settings
from hypothesis import strategies as st

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------

REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
BUILD_GN = os.path.join(REPO_ROOT, "src", "controller", "java", "BUILD.gn")

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


def _read_file(path: str) -> str:
    with open(path, "r") as f:
        return f.read()


def _extract_brace_block(text: str, start: int) -> str:
    """
    Given *text* and the index of an opening '{', return the content
    between the braces (handling nested braces).
    """
    assert text[start] == "{", f"Expected '{{' at position {start}"
    depth = 1
    pos = start + 1
    while pos < len(text) and depth > 0:
        if text[pos] == "{":
            depth += 1
        elif text[pos] == "}":
            depth -= 1
        pos += 1
    return text[start + 1: pos - 1]


def _find_tlv_decoder_block(content: str) -> str:
    """
    Locate the `if (matter_enable_tlv_decoder_cpp)` block inside BUILD.gn
    and return its body text.
    """
    pattern = r"if\s*\(\s*matter_enable_tlv_decoder_cpp\s*\)"
    m = re.search(pattern, content)
    assert m is not None, (
        "Could not find `if (matter_enable_tlv_decoder_cpp)` in BUILD.gn"
    )
    brace_pos = content.index("{", m.end())
    return _extract_brace_block(content, brace_pos)


def _find_conditional_override(block: str, override_var: str, fallback_file: str):
    """
    Verify that *block* contains a conditional of the form:

        if (<override_var> != "") {
          sources += [ <override_var> ]
        } else {
          sources += [ "<fallback_file>" ]
        }

    Returns (if_body, else_body) strings for further inspection.
    """
    # Match the if-condition
    pattern = rf'if\s*\(\s*{re.escape(override_var)}\s*!=\s*""\s*\)'
    m = re.search(pattern, block)
    assert m is not None, (
        f"Could not find `if ({override_var} != \"\")` inside the "
        f"matter_enable_tlv_decoder_cpp block"
    )

    # Extract the if-body
    if_brace = block.index("{", m.end())
    if_body = _extract_brace_block(block, if_brace)

    # Find the else clause immediately after the if-block closing brace
    after_if = if_brace + 1 + len(if_body) + 1  # skip past closing '}'
    rest = block[after_if:]
    else_match = re.match(r"\s*else\s*\{", rest)
    assert else_match is not None, (
        f"Could not find `else` clause after `if ({override_var} != \"\")` block"
    )

    else_brace = after_if + rest.index("{", else_match.start())
    else_body = _extract_brace_block(block, else_brace)

    return if_body, else_body


# ---------------------------------------------------------------------------
# Property-based tests
# ---------------------------------------------------------------------------

# Override arg names paired with their zap-generated fallback filenames
OVERRIDE_CONFIGS = [
    ("chip_tlv_decoder_attribute_source_override",
     "zap-generated/CHIPAttributeTLVValueDecoder.cpp"),
    ("chip_tlv_decoder_event_source_override",
     "zap-generated/CHIPEventTLVValueDecoder.cpp"),
]


@given(
    override_config=st.sampled_from(OVERRIDE_CONFIGS),
    dummy_path=st.text(
        alphabet=st.characters(whitelist_categories=("L", "N", "P")),
        min_size=1,
        max_size=80,
    ),
)
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_gn_conditional_source_selection(override_config, dummy_path):
    """
    **Validates: Requirements 3.3, 3.4, 3.5, 6.1, 6.2**

    Property 5: For any non-empty value of the override arg, BUILD.gn SHALL
    compile the override file instead of the zap-generated file. When the
    override is empty (default), the zap-generated file SHALL be compiled.

    This test:
    1. Parses BUILD.gn and locates the matter_enable_tlv_decoder_cpp block
    2. Verifies the conditional `if (override != "")` exists for each arg
    3. Verifies the if-branch references the override variable
    4. Verifies the else-branch references the zap-generated fallback
    """
    override_var, fallback_file = override_config

    content = _read_file(BUILD_GN)
    block = _find_tlv_decoder_block(content)

    # Verify the conditional pattern exists and extract bodies
    if_body, else_body = _find_conditional_override(block, override_var, fallback_file)

    # The if-body should reference the override variable (the dynamic path)
    assert override_var in if_body, (
        f"The if-branch for `{override_var} != \"\"` does not reference "
        f"`{override_var}` — expected `sources += [ {override_var} ]`"
    )

    # The else-body should reference the zap-generated fallback file
    assert fallback_file in else_body, (
        f"The else-branch for `{override_var}` does not reference "
        f"`{fallback_file}` — expected `sources += [ \"{fallback_file}\" ]`"
    )

    # The if-body should contain a sources += assignment
    assert "sources +=" in if_body, (
        f"The if-branch for `{override_var}` does not contain `sources +=`"
    )

    # The else-body should contain a sources += assignment
    assert "sources +=" in else_body, (
        f"The else-branch for `{override_var}` does not contain `sources +=`"
    )


@given(dummy=st.just(True))
@settings(
    max_examples=1,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_tlv_decoder_block_preserves_unconditional_elements(dummy):
    """
    **Validates: Requirements 3.5, 6.1, 6.2**

    Verify that the matter_enable_tlv_decoder_cpp block still contains the
    unconditional elements: the USE_JAVA_TLV_ENCODE_DECODE define and the
    CHIPTLVValueDecoder-JNI.cpp source. These must not be gated behind the
    override conditionals.
    """
    content = _read_file(BUILD_GN)
    block = _find_tlv_decoder_block(content)

    assert "USE_JAVA_TLV_ENCODE_DECODE" in block, (
        "The matter_enable_tlv_decoder_cpp block is missing the "
        "`USE_JAVA_TLV_ENCODE_DECODE` define"
    )

    assert "CHIPTLVValueDecoder-JNI.cpp" in block, (
        "The matter_enable_tlv_decoder_cpp block is missing "
        "`CHIPTLVValueDecoder-JNI.cpp` as an unconditional source"
    )


@given(dummy=st.just(True))
@settings(
    max_examples=1,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_both_override_conditionals_present(dummy):
    """
    **Validates: Requirements 3.3, 3.4**

    Verify that both override conditionals are present in the
    matter_enable_tlv_decoder_cpp block — one for attributes, one for events.
    """
    content = _read_file(BUILD_GN)
    block = _find_tlv_decoder_block(content)

    for override_var, fallback_file in OVERRIDE_CONFIGS:
        pattern = rf'if\s*\(\s*{re.escape(override_var)}\s*!=\s*""\s*\)'
        assert re.search(pattern, block) is not None, (
            f"Missing conditional for `{override_var}` in the "
            f"matter_enable_tlv_decoder_cpp block"
        )


# ---------------------------------------------------------------------------
# Allow running directly
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    import sys

    print("Running GN conditional source selection property tests...")
    tests = [
        ("5a: GN conditional source selection",
         test_gn_conditional_source_selection),
        ("5b: Unconditional elements preserved",
         test_tlv_decoder_block_preserves_unconditional_elements),
        ("5c: Both override conditionals present",
         test_both_override_conditionals_present),
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
