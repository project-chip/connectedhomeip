"""
Property Test — Property 3: Interface compatibility (attribute & event decoders)

**Validates: Requirements 1.3, 2.3**

This test parses both the slim attribute decoder
(`casting-CHIPAttributeTLVValueDecoder.cpp`) and the full zap-generated
decoder (`zap-generated/CHIPAttributeTLVValueDecoder.cpp`), as well as
the slim event decoder (`casting-CHIPEventTLVValueDecoder.cpp`) and the
full zap-generated event decoder (`zap-generated/CHIPEventTLVValueDecoder.cpp`),
and verifies:

1. The set of `#include` directives matches between slim and full (for each).
2. The function signature matches between slim and full (for each).

Feature: casting-client-cluster-reduction
Property 3: Interface compatibility
"""

import os
import re

from hypothesis import HealthCheck, given, settings
from hypothesis import strategies as st

# -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -
# Paths
# -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -

REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))

SLIM_ATTR_DECODER = os.path.join(
    REPO_ROOT,
    "examples",
    "tv-casting-app",
    "tv-casting-common",
    "casting-CHIPAttributeTLVValueDecoder.cpp",
)

FULL_ATTR_DECODER = os.path.join(
    REPO_ROOT,
    "src",
    "controller",
    "java",
    "zap-generated",
    "CHIPAttributeTLVValueDecoder.cpp",
)

SLIM_EVENT_DECODER = os.path.join(
    REPO_ROOT,
    "examples",
    "tv-casting-app",
    "tv-casting-common",
    "casting-CHIPEventTLVValueDecoder.cpp",
)

FULL_EVENT_DECODER = os.path.join(
    REPO_ROOT,
    "src",
    "controller",
    "java",
    "zap-generated",
    "CHIPEventTLVValueDecoder.cpp",
)

# -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -
# Helpers
# -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -


def _read_file(path: str) -> str:
    with open(path, "r") as f:
        return f.read()


def _extract_includes(content: str) -> set:
    """
    Extract all ``#include`` directives from a C++ source file.

    Returns a set of the raw include strings, e.g.
    ``{'<jni.h>', '<controller/java/CHIPAttributeTLVValueDecoder.h>'}``.
    Angle-bracket and quoted includes are both captured.
    """
    pattern = r'^\s*#include\s+([<"][^>"]+[>"])'
    return set(re.findall(pattern, content, re.MULTILINE))


def _extract_function_signature(content: str, func_name: str) -> str:
    """
    Extract the function signature line for *func_name* from C++ source.

    The signature is normalised: leading/trailing whitespace stripped,
    internal runs of whitespace collapsed to a single space, and the
    opening brace (if on the same line) removed.  This makes comparison
    resilient to minor formatting differences.
    """
# Match the return type + function name + params, possibly spanning
# multiple lines up to the opening brace.
    pattern = (
        r"((?:jobject)\s+"
        + re.escape(func_name)
        + r"\s*\([^)]*\))"
    )
    m = re.search(pattern, content, re.DOTALL)
    if not m:
        return ""
    sig = m.group(1)
# Normalise whitespace
    sig = re.sub(r"\s+", " ", sig).strip()
    return sig

# -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -
# Property - based tests
# -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -


@given(dummy=st.just(True))
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_attribute_decoder_includes_match(dummy):
    """
    **Validates: Requirements 1.3, 2.3**

    Property 3: The set of #include directives in the slim attribute
    decoder SHALL match those in the full zap-generated attribute decoder.
    """
    slim_content = _read_file(SLIM_ATTR_DECODER)
    full_content = _read_file(FULL_ATTR_DECODER)

    slim_includes = _extract_includes(slim_content)
    full_includes = _extract_includes(full_content)

    assert slim_includes == full_includes, (
        f"Include set mismatch between slim and full attribute decoders.\n"
        f"  Only in slim: {sorted(slim_includes - full_includes)}\n"
        f"  Only in full: {sorted(full_includes - slim_includes)}\n"
        f"  Slim includes: {sorted(slim_includes)}\n"
        f"  Full includes: {sorted(full_includes)}"
    )


@given(dummy=st.just(True))
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_attribute_decoder_function_signature_matches(dummy):
    """
    **Validates: Requirements 1.3, 2.3**

    Property 3: The DecodeAttributeValue() function signature in the slim
    attribute decoder SHALL match the signature in the full zap-generated
    attribute decoder, ensuring link-time compatibility with the JNI bridge.
    """
    slim_content = _read_file(SLIM_ATTR_DECODER)
    full_content = _read_file(FULL_ATTR_DECODER)

    slim_sig = _extract_function_signature(slim_content, "DecodeAttributeValue")
    full_sig = _extract_function_signature(full_content, "DecodeAttributeValue")

    assert slim_sig, (
        "Could not find DecodeAttributeValue() signature in slim decoder."
    )
    assert full_sig, (
        "Could not find DecodeAttributeValue() signature in full decoder."
    )
    assert slim_sig == full_sig, (
        f"Function signature mismatch.\n"
        f"  Slim: {slim_sig}\n"
        f"  Full: {full_sig}"
    )

# -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -
# Property - based tests — Event decoder
# -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -


@given(dummy=st.just(True))
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_event_decoder_includes_match(dummy):
    """
    **Validates: Requirements 2.3**

    Property 3: The set of #include directives in the slim event
    decoder SHALL match those in the full zap-generated event decoder.
    """
    slim_content = _read_file(SLIM_EVENT_DECODER)
    full_content = _read_file(FULL_EVENT_DECODER)

    slim_includes = _extract_includes(slim_content)
    full_includes = _extract_includes(full_content)

    assert slim_includes == full_includes, (
        f"Include set mismatch between slim and full event decoders.\n"
        f"  Only in slim: {sorted(slim_includes - full_includes)}\n"
        f"  Only in full: {sorted(full_includes - slim_includes)}\n"
        f"  Slim includes: {sorted(slim_includes)}\n"
        f"  Full includes: {sorted(full_includes)}"
    )


@given(dummy=st.just(True))
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_event_decoder_function_signature_matches(dummy):
    """
    **Validates: Requirements 2.3**

    Property 3: The DecodeEventValue() function signature in the slim
    event decoder SHALL match the signature in the full zap-generated
    event decoder, ensuring link-time compatibility with the JNI bridge.
    """
    slim_content = _read_file(SLIM_EVENT_DECODER)
    full_content = _read_file(FULL_EVENT_DECODER)

    slim_sig = _extract_function_signature(slim_content, "DecodeEventValue")
    full_sig = _extract_function_signature(full_content, "DecodeEventValue")

    assert slim_sig, (
        "Could not find DecodeEventValue() signature in slim event decoder."
    )
    assert full_sig, (
        "Could not find DecodeEventValue() signature in full event decoder."
    )
    assert slim_sig == full_sig, (
        f"Function signature mismatch.\n"
        f"  Slim: {slim_sig}\n"
        f"  Full: {full_sig}"
    )


# -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -
# Allow running directly
# -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -
if __name__ == "__main__":
    import sys

    print("Running interface compatibility property tests (attribute & event decoders)...")
    tests = [
        ("3a: Attribute include sets match", test_attribute_decoder_includes_match),
        ("3b: Attribute function signature matches", test_attribute_decoder_function_signature_matches),
        ("3c: Event include sets match", test_event_decoder_includes_match),
        ("3d: Event function signature matches", test_event_decoder_function_signature_matches),
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
