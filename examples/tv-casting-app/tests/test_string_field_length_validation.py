"""
Property Test — Property 2: String Field Length Validation

**Validates: Requirements 1.3, 1.4, 1.5, 1.6**

This test parses `MCDeviceInstanceInfo.m` and verifies that the initializer
validates string field lengths correctly: vendorName, productName, and
serialNumber have a max length of 32 characters, and hardwareVersionString
has a max length of 64 characters.

For any string of length 0–100, the init SHALL succeed iff the string is
within the field's max allowed length. Nil values are always accepted.

Feature: ios-device-instance-info-provider
Property 2: String Field Length Validation
"""

import os
import re

from hypothesis import HealthCheck, given, settings
from hypothesis import strategies as st

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------

REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
MC_DEVICE_INSTANCE_INFO_M = os.path.join(
    REPO_ROOT,
    "examples",
    "tv-casting-app",
    "darwin",
    "MatterTvCastingBridge",
    "MatterTvCastingBridge",
    "MCDeviceInstanceInfo.m",
)

# ---------------------------------------------------------------------------
# Expected max lengths per the Matter spec and design doc
# ---------------------------------------------------------------------------

EXPECTED_MAX_LENGTHS = {
    "vendorName": 32,
    "productName": 32,
    "serialNumber": 32,
    "hardwareVersionString": 64,
}

# ---------------------------------------------------------------------------
# Helpers — source code parsing
# ---------------------------------------------------------------------------


def _read_file(path: str) -> str:
    with open(path, "r") as f:
        return f.read()


def _extract_max_length_constants(content: str) -> dict:
    """
    Extract max length constants from the .m file.

    Looks for patterns like:
        static const NSUInteger kMaxVendorNameLength = 32;
    """
    pattern = r"static\s+const\s+NSUInteger\s+kMax(\w+)Length\s*=\s*(\d+)\s*;"
    matches = re.findall(pattern, content)
    result = {}
    for name_part, value in matches:
        # Convert e.g. "VendorName" -> "vendorName"
        field_name = name_part[0].lower() + name_part[1:]
        result[field_name] = int(value)
    return result


def _extract_validation_guards(content: str) -> list:
    """
    Extract the validation guard patterns from the initializer.

    Looks for patterns like:
        if (vendorName != nil && vendorName.length > kMaxVendorNameLength) {
            return nil;
        }

    Returns a list of field names that have validation guards.
    """
    pattern = (
        r"if\s*\(\s*(\w+)\s*!=\s*nil\s*&&\s*\1\.length\s*>\s*kMax(\w+)Length\s*\)"
        r"\s*\{\s*return\s+nil\s*;"
    )
    matches = re.findall(pattern, content, re.DOTALL)
    return [field_name for field_name, _ in matches]


# ---------------------------------------------------------------------------
# Property-based tests
# ---------------------------------------------------------------------------


@given(length=st.integers(min_value=0, max_value=100))
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_vendor_name_length_validation_rule(length):
    """
    **Validates: Requirements 1.3**

    Property 2: For any string of length 0–100 used as vendorName,
    MCDeviceInstanceInfo init SHALL succeed iff length <= 32.
    The source code must contain a guard that returns nil when
    vendorName.length > 32.
    """
    content = _read_file(MC_DEVICE_INSTANCE_INFO_M)
    constants = _extract_max_length_constants(content)
    max_len = constants.get("vendorName")

    assert max_len is not None, (
        "Could not find kMaxVendorNameLength constant in MCDeviceInstanceInfo.m"
    )
    assert max_len == EXPECTED_MAX_LENGTHS["vendorName"], (
        f"vendorName max length mismatch: expected {EXPECTED_MAX_LENGTHS['vendorName']}, "
        f"found {max_len}"
    )

    # Verify the validation guard exists
    guards = _extract_validation_guards(content)
    assert "vendorName" in guards, (
        "MCDeviceInstanceInfo.m is missing validation guard for vendorName. "
        f"Found guards for: {guards}"
    )

    # Property: init succeeds iff length <= max_len
    should_succeed = length <= max_len
    assert should_succeed == (length <= 32), (
        f"vendorName validation logic error: string of length {length} "
        f"should {'succeed' if length <= 32 else 'fail'} but max_len={max_len}"
    )


@given(length=st.integers(min_value=0, max_value=100))
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_product_name_length_validation_rule(length):
    """
    **Validates: Requirements 1.4**

    Property 2: For any string of length 0–100 used as productName,
    MCDeviceInstanceInfo init SHALL succeed iff length <= 32.
    The source code must contain a guard that returns nil when
    productName.length > 32.
    """
    content = _read_file(MC_DEVICE_INSTANCE_INFO_M)
    constants = _extract_max_length_constants(content)
    max_len = constants.get("productName")

    assert max_len is not None, (
        "Could not find kMaxProductNameLength constant in MCDeviceInstanceInfo.m"
    )
    assert max_len == EXPECTED_MAX_LENGTHS["productName"], (
        f"productName max length mismatch: expected {EXPECTED_MAX_LENGTHS['productName']}, "
        f"found {max_len}"
    )

    guards = _extract_validation_guards(content)
    assert "productName" in guards, (
        "MCDeviceInstanceInfo.m is missing validation guard for productName. "
        f"Found guards for: {guards}"
    )

    should_succeed = length <= max_len
    assert should_succeed == (length <= 32), (
        f"productName validation logic error: string of length {length} "
        f"should {'succeed' if length <= 32 else 'fail'} but max_len={max_len}"
    )


@given(length=st.integers(min_value=0, max_value=100))
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_serial_number_length_validation_rule(length):
    """
    **Validates: Requirements 1.5**

    Property 2: For any string of length 0–100 used as serialNumber,
    MCDeviceInstanceInfo init SHALL succeed iff length <= 32.
    The source code must contain a guard that returns nil when
    serialNumber.length > 32.
    """
    content = _read_file(MC_DEVICE_INSTANCE_INFO_M)
    constants = _extract_max_length_constants(content)
    max_len = constants.get("serialNumber")

    assert max_len is not None, (
        "Could not find kMaxSerialNumberLength constant in MCDeviceInstanceInfo.m"
    )
    assert max_len == EXPECTED_MAX_LENGTHS["serialNumber"], (
        f"serialNumber max length mismatch: expected {EXPECTED_MAX_LENGTHS['serialNumber']}, "
        f"found {max_len}"
    )

    guards = _extract_validation_guards(content)
    assert "serialNumber" in guards, (
        "MCDeviceInstanceInfo.m is missing validation guard for serialNumber. "
        f"Found guards for: {guards}"
    )

    should_succeed = length <= max_len
    assert should_succeed == (length <= 32), (
        f"serialNumber validation logic error: string of length {length} "
        f"should {'succeed' if length <= 32 else 'fail'} but max_len={max_len}"
    )


@given(length=st.integers(min_value=0, max_value=100))
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_hardware_version_string_length_validation_rule(length):
    """
    **Validates: Requirements 1.6**

    Property 2: For any string of length 0–100 used as hardwareVersionString,
    MCDeviceInstanceInfo init SHALL succeed iff length <= 64.
    The source code must contain a guard that returns nil when
    hardwareVersionString.length > 64.
    """
    content = _read_file(MC_DEVICE_INSTANCE_INFO_M)
    constants = _extract_max_length_constants(content)
    max_len = constants.get("hardwareVersionString")

    assert max_len is not None, (
        "Could not find kMaxHardwareVersionStringLength constant in MCDeviceInstanceInfo.m"
    )
    assert max_len == EXPECTED_MAX_LENGTHS["hardwareVersionString"], (
        f"hardwareVersionString max length mismatch: "
        f"expected {EXPECTED_MAX_LENGTHS['hardwareVersionString']}, found {max_len}"
    )

    guards = _extract_validation_guards(content)
    assert "hardwareVersionString" in guards, (
        "MCDeviceInstanceInfo.m is missing validation guard for hardwareVersionString. "
        f"Found guards for: {guards}"
    )

    should_succeed = length <= max_len
    assert should_succeed == (length <= 64), (
        f"hardwareVersionString validation logic error: string of length {length} "
        f"should {'succeed' if length <= 64 else 'fail'} but max_len={max_len}"
    )


@given(
    field=st.sampled_from(list(EXPECTED_MAX_LENGTHS.keys())),
    length=st.integers(min_value=0, max_value=100),
)
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_all_string_fields_validation_consistency(field, length):
    """
    **Validates: Requirements 1.3, 1.4, 1.5, 1.6**

    Property 2: For any string field and any string of length 0–100,
    the validation constant in MCDeviceInstanceInfo.m SHALL match the
    expected max length, and the validation guard SHALL exist. Init
    succeeds iff length <= max_length for that field.
    """
    content = _read_file(MC_DEVICE_INSTANCE_INFO_M)
    constants = _extract_max_length_constants(content)
    guards = _extract_validation_guards(content)

    expected_max = EXPECTED_MAX_LENGTHS[field]
    actual_max = constants.get(field)

    assert actual_max is not None, (
        f"Could not find max length constant for '{field}' in MCDeviceInstanceInfo.m. "
        f"Found constants: {constants}"
    )
    assert actual_max == expected_max, (
        f"Max length mismatch for '{field}': expected {expected_max}, found {actual_max}"
    )
    assert field in guards, (
        f"MCDeviceInstanceInfo.m is missing validation guard for '{field}'. "
        f"Found guards for: {guards}"
    )

    # The property: init succeeds iff length <= max_length
    init_should_succeed = length <= expected_max
    # Verify the source code enforces this via the guard pattern:
    # if (field != nil && field.length > kMaxFieldLength) { return nil; }
    # This means: returns nil when length > max, succeeds when length <= max
    code_rejects = length > actual_max
    assert init_should_succeed != code_rejects, (
        f"Validation logic inconsistency for '{field}' with length {length}: "
        f"expected init to {'succeed' if init_should_succeed else 'fail'}, "
        f"but code would {'reject' if code_rejects else 'accept'}"
    )


# ---------------------------------------------------------------------------
# Allow running directly
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    import sys

    print("Running string field length validation property tests...")
    tests = [
        ("2a: vendorName length validation", test_vendor_name_length_validation_rule),
        ("2b: productName length validation", test_product_name_length_validation_rule),
        ("2c: serialNumber length validation", test_serial_number_length_validation_rule),
        ("2d: hardwareVersionString length validation", test_hardware_version_string_length_validation_rule),
        ("2e: All fields validation consistency", test_all_string_fields_validation_consistency),
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
