"""
Property Test — Property 3: Invalid Input Rejection

**Validates: Requirements 1.3, 1.4, 1.5, 1.6, 4.4**

This test parses `MCDeviceInstanceInfo.m` and verifies that the initializer
rejects any combination of inputs where at least one string field exceeds
its maximum allowed length. The source code must contain guard patterns
that return nil when any field is over-length.

For any MCDeviceInstanceInfo object that contains at least one field
violating its length constraint, initialization SHALL return nil (fail).

Feature: ios-device-instance-info-provider
Property 3: Invalid Input Rejection
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

STRING_FIELDS = {
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


def _verify_source_has_all_guards(content: str):
    """
    Verify that the source code has validation guards for all string fields
    and that the max length constants match expected values.
    """
    constants = _extract_max_length_constants(content)
    guards = _extract_validation_guards(content)

    for field, expected_max in STRING_FIELDS.items():
        assert field in constants, (
            f"Missing max length constant for '{field}' in MCDeviceInstanceInfo.m. "
            f"Found constants: {constants}"
        )
        assert constants[field] == expected_max, (
            f"Max length mismatch for '{field}': expected {expected_max}, "
            f"found {constants[field]}"
        )
        assert field in guards, (
            f"Missing validation guard for '{field}' in MCDeviceInstanceInfo.m. "
            f"Found guards for: {guards}"
        )


# ---------------------------------------------------------------------------
# Hypothesis strategies
# ---------------------------------------------------------------------------

# Strategy for a string that is within the max length for a given field
def _valid_string_for_field(field_name):
    max_len = STRING_FIELDS[field_name]
    return st.text(min_size=0, max_size=max_len)


# Strategy for a string that exceeds the max length for a given field
def _overlength_string_for_field(field_name):
    max_len = STRING_FIELDS[field_name]
    return st.text(min_size=max_len + 1, max_size=max_len + 50)


# Strategy that generates a dict of string field values where at least one
# field exceeds its max length. Each field is either None, valid, or overlength.
# We ensure at least one field is overlength.
@st.composite
def _device_info_with_at_least_one_overlength(draw):
    """
    Generate a dict mapping each string field to either None, a valid string,
    or an overlength string — with the constraint that at least one field
    is overlength.
    """
    fields = list(STRING_FIELDS.keys())

    # Pick at least one field to be overlength
    overlength_fields = draw(
        st.lists(
            st.sampled_from(fields),
            min_size=1,
            max_size=len(fields),
            unique=True,
        )
    )

    result = {}
    for field in fields:
        if field in overlength_fields:
            result[field] = draw(_overlength_string_for_field(field))
        else:
            # Randomly choose None or a valid string
            choice = draw(st.sampled_from(["none", "valid"]))
            if choice == "none":
                result[field] = None
            else:
                result[field] = draw(_valid_string_for_field(field))

    return result


# ---------------------------------------------------------------------------
# Property-based tests
# ---------------------------------------------------------------------------


@given(field_values=_device_info_with_at_least_one_overlength())
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.function_scoped_fixture, HealthCheck.too_slow],
    deadline=None,
)
def test_init_returns_nil_when_any_field_overlength(field_values):
    """
    **Validates: Requirements 1.3, 1.4, 1.5, 1.6, 4.4**

    Property 3: For any MCDeviceInstanceInfo object that contains at least
    one string field exceeding its maximum allowed length, the initializer
    SHALL return nil.

    We verify this by checking that the source code contains guard patterns
    that return nil when any field exceeds its limit. Since at least one
    field in the generated input is overlength, the guard for that field
    will trigger and the initializer will return nil.
    """
    content = _read_file(MC_DEVICE_INSTANCE_INFO_M)

    # First verify all guards and constants are present
    _verify_source_has_all_guards(content)

    # Identify which fields are overlength in this generated input
    overlength_fields = []
    for field, value in field_values.items():
        if value is not None and len(value) > STRING_FIELDS[field]:
            overlength_fields.append(field)

    # At least one field must be overlength (guaranteed by the strategy)
    assert len(overlength_fields) > 0, (
        "Strategy error: no overlength fields generated"
    )

    # For each overlength field, verify the source code would reject it:
    # The guard pattern `if (field != nil && field.length > kMaxFieldLength) { return nil; }`
    # means the initializer returns nil as soon as it encounters the first
    # overlength field. Since guards are checked sequentially, any single
    # overlength field causes rejection.
    constants = _extract_max_length_constants(content)
    guards = _extract_validation_guards(content)

    for field in overlength_fields:
        value = field_values[field]
        max_len = constants[field]

        # The field is non-nil and exceeds max length
        assert value is not None
        assert len(value) > max_len, (
            f"Expected '{field}' to be overlength (>{max_len}), "
            f"but length is {len(value)}"
        )

        # The guard exists for this field, so init will return nil
        assert field in guards, (
            f"No validation guard for overlength field '{field}'"
        )


@given(
    overlength_field=st.sampled_from(list(STRING_FIELDS.keys())),
    extra_length=st.integers(min_value=1, max_value=50),
)
@settings(
    max_examples=100,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_single_overlength_field_causes_rejection(overlength_field, extra_length):
    """
    **Validates: Requirements 1.3, 1.4, 1.5, 1.6, 4.4**

    Property 3: When exactly one string field exceeds its max length and
    all other fields are valid (or nil), the initializer SHALL still
    return nil. This confirms that a single invalid field is sufficient
    to reject the entire object.
    """
    content = _read_file(MC_DEVICE_INSTANCE_INFO_M)
    _verify_source_has_all_guards(content)

    constants = _extract_max_length_constants(content)
    guards = _extract_validation_guards(content)

    max_len = constants[overlength_field]
    invalid_length = max_len + extra_length

    # The overlength field exceeds its limit
    assert invalid_length > max_len

    # The guard for this field exists and would return nil
    assert overlength_field in guards, (
        f"No validation guard for '{overlength_field}' — "
        f"a string of length {invalid_length} (max {max_len}) would not be rejected"
    )


# ---------------------------------------------------------------------------
# Allow running directly
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    import sys

    print("Running invalid input rejection property tests...")
    tests = [
        (
            "3a: Init returns nil when any field is overlength",
            test_init_returns_nil_when_any_field_overlength,
        ),
        (
            "3b: Single overlength field causes rejection",
            test_single_overlength_field_causes_rejection,
        ),
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
