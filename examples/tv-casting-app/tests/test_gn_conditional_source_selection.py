"""
Property Test — Property 5: GN conditional source selection

**Validates: Requirements 4.1, 4.2, 5.1, 14.4**

This test parses `src/controller/java/BUILD.gn` and verifies that the
`android_chip_im_jni` source set contains conditional logic for both
TLV decoder overrides using `chip_data_model_overrides_dir`, following
the pattern:

    if (chip_data_model_overrides_dir != "") { use override } else { use zap-generated }

This ensures that when the override directory is set, the slim files are
compiled, and when it is empty (default), the full zap-generated decoders
are compiled.

After the consolidation, `matter_enable_tlv_decoder_cpp` has been removed
and the TLV decoder block is unconditional.
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
BUILD_GN = os.path.join(REPO_ROOT, "src", "controller", "java", "BUILD.gn")

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


def _read_file(path: str) -> str:
    with open(path, "r") as f:
        return f.read()


# Override filenames paired with their zap-generated fallback filenames
OVERRIDE_CONFIGS = [
    ("CHIPAttributeTLVValueDecoder-override.cpp",
     "zap-generated/CHIPAttributeTLVValueDecoder.cpp"),
    ("CHIPEventTLVValueDecoder-override.cpp",
     "zap-generated/CHIPEventTLVValueDecoder.cpp"),
]


# ---------------------------------------------------------------------------
# Property-based tests
# ---------------------------------------------------------------------------


@given(
    override_config=st.sampled_from(OVERRIDE_CONFIGS),
)
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_gn_conditional_source_selection(override_config):
    """
    **Validates: Requirements 4.1, 4.2, 14.4**

    Property 5: For any non-empty value of `chip_data_model_overrides_dir`,
    BUILD.gn SHALL compile the override file instead of the zap-generated
    file. When the override dir is empty (default), the zap-generated file
    SHALL be compiled.

    This test:
    1. Parses BUILD.gn
    2. Verifies the conditional `if (chip_data_model_overrides_dir != "")` exists
    3. Verifies the override filename is referenced with the override dir
    4. Verifies the fallback filename is present for the else branch
    """
    override_filename, fallback_file = override_config

    content = _read_file(BUILD_GN)

    # Verify the conditional pattern exists
    conditional_pattern = r'if\s*\(\s*chip_data_model_overrides_dir\s*!=\s*""\s*\)'
    assert re.search(conditional_pattern, content), (
        "Could not find `if (chip_data_model_overrides_dir != \"\")` in BUILD.gn"
    )

    # The override filename should be referenced with chip_data_model_overrides_dir
    dir_ref_pattern = (
        r"chip_data_model_overrides_dir.*" + re.escape(override_filename)
    )
    assert re.search(dir_ref_pattern, content), (
        f"BUILD.gn does not reference `{override_filename}` in combination "
        f"with `chip_data_model_overrides_dir`"
    )

    # The fallback file should be present
    assert fallback_file in content, (
        f"BUILD.gn does not reference the fallback file `{fallback_file}` — "
        f"there should be an else branch for the default path"
    )


@given(dummy=st.just(True))
@settings(
    max_examples=1,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_tlv_decoder_unconditional_elements(dummy):
    """
    **Validates: Requirements 4.1, 4.2**

    Verify that the android_chip_im_jni source set contains the
    unconditional elements: the USE_JAVA_TLV_ENCODE_DECODE define and the
    CHIPTLVValueDecoder-JNI.cpp source. These are no longer gated behind
    `matter_enable_tlv_decoder_cpp` (which has been removed).
    """
    content = _read_file(BUILD_GN)

    assert "USE_JAVA_TLV_ENCODE_DECODE" in content, (
        "BUILD.gn is missing the `USE_JAVA_TLV_ENCODE_DECODE` define "
        "in the android_chip_im_jni source set"
    )

    assert "CHIPTLVValueDecoder-JNI.cpp" in content, (
        "BUILD.gn is missing `CHIPTLVValueDecoder-JNI.cpp` as a source "
        "in the android_chip_im_jni source set"
    )


@given(dummy=st.just(True))
@settings(
    max_examples=1,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_matter_enable_tlv_decoder_cpp_removed(dummy):
    """
    **Validates: Requirements 2.6, 14.2**

    Verify that `matter_enable_tlv_decoder_cpp` is no longer referenced
    as a conditional in BUILD.gn. The flag has been removed and the TLV
    decoder block is now unconditional.
    """
    content = _read_file(BUILD_GN)

    pattern = r"if\s*\(\s*matter_enable_tlv_decoder_cpp\s*\)"
    assert not re.search(pattern, content), (
        "REGRESSION: `if (matter_enable_tlv_decoder_cpp)` is still present "
        "in BUILD.gn — this conditional should have been removed as part of "
        "the consolidation to chip_data_model_overrides_dir"
    )


# ---------------------------------------------------------------------------
# Allow running directly
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    import sys

    print("Running GN conditional source selection property tests...")
    tests = [
        ("5a: GN conditional source selection with overrides dir",
         test_gn_conditional_source_selection),
        ("5b: Unconditional TLV decoder elements preserved",
         test_tlv_decoder_unconditional_elements),
        ("5c: matter_enable_tlv_decoder_cpp removed from BUILD.gn",
         test_matter_enable_tlv_decoder_cpp_removed),
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
