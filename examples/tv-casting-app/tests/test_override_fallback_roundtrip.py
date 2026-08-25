"""
Property Test — Property 1: Override/Fallback Round-Trip

**Validates: Requirements 1.2, 2.3, 3.2, 3.3, 3.4, 3.5, 3.6, 3.7, 3.8, 3.9, 3.10, 3.11, 3.12, 3.13, 3.14, 3.15, 3.16**

For any MCDeviceInstanceInfo object with an arbitrary subset of fields set to
non-nil valid values and the remaining fields set to nil, when an
MCDeviceInstanceInfoProvider is initialized with that object, each getter
method SHALL return the app-provided value for non-nil fields and the
compile-time default value for nil fields.

Since we cannot compile and run the Objective-C++/C++ code in a Python test,
this test parses the source code of MCDeviceInstanceInfoProvider.mm and
MCDeviceInstanceInfoProvider.h to verify the override/fallback logic is
correctly implemented for every combination of nil/non-nil fields.

Feature: ios-device-instance-info-provider, Property 1: Override/Fallback Round-Trip
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

PROVIDER_MM = os.path.join(
    REPO_ROOT,
    "examples",
    "tv-casting-app",
    "darwin",
    "MatterTvCastingBridge",
    "MatterTvCastingBridge",
    "MCDeviceInstanceInfoProvider.mm",
)

PROVIDER_H = os.path.join(
    REPO_ROOT,
    "examples",
    "tv-casting-app",
    "darwin",
    "MatterTvCastingBridge",
    "MatterTvCastingBridge",
    "MCDeviceInstanceInfoProvider.h",
)

# ---------------------------------------------------------------------------
# Field definitions
# ---------------------------------------------------------------------------

# String fields: name -> (flag name, buffer name, getter method name)
STRING_FIELDS = {
    "vendorName": ("mHasVendorName", "mVendorName", "GetVendorName"),
    "productName": ("mHasProductName", "mProductName", "GetProductName"),
    "serialNumber": ("mHasSerialNumber", "mSerialNumber", "GetSerialNumber"),
    "hardwareVersionString": (
        "mHasHardwareVersionString",
        "mHardwareVersionString",
        "GetHardwareVersionString",
    ),
}

# Numeric fields: name -> (optional member name, getter method name)
NUMERIC_FIELDS = {
    "vendorId": ("mVendorId", "GetVendorId"),
    "productId": ("mProductId", "GetProductId"),
    "hardwareVersion": ("mHardwareVersion", "GetHardwareVersion"),
}

ALL_FIELD_NAMES = list(STRING_FIELDS.keys()) + list(NUMERIC_FIELDS.keys())


# ---------------------------------------------------------------------------
# Helpers — source code reading
# ---------------------------------------------------------------------------


def _read_file(path: str) -> str:
    with open(path, "r") as f:
        return f.read()


# ---------------------------------------------------------------------------
# Structural verification helpers
# ---------------------------------------------------------------------------


def _verify_initialize_copies_string(content: str, field_name: str, flag: str, buffer: str):
    """
    Verify that Initialize() checks if the ObjC property is non-nil,
    copies the string into the internal buffer, and sets the flag.

    Expected pattern:
        if (deviceInstanceInfo.<field> != nil) {
            chip::Platform::CopyString(<buffer>, [deviceInstanceInfo.<field> UTF8String]);
            <flag> = true;
        }
    """
    pattern = (
        r"if\s*\(\s*deviceInstanceInfo\." + re.escape(field_name) + r"\s*!=\s*nil\s*\)"
        r"\s*\{[^}]*CopyString\s*\(\s*" + re.escape(buffer) + r"\s*,"
        r"[^}]*" + re.escape(flag) + r"\s*=\s*true\s*;"
    )
    match = re.search(pattern, content, re.DOTALL)
    assert match is not None, (
        f"Initialize() does not correctly copy '{field_name}' into '{buffer}' "
        f"and set '{flag}' = true when non-nil"
    )


def _verify_initialize_stores_numeric(content: str, field_name: str, optional_member: str):
    """
    Verify that Initialize() checks if the ObjC property is non-nil
    and stores the value via SetValue on the Optional member.

    Expected pattern:
        if (deviceInstanceInfo.<field> != nil) {
            <optional_member>.SetValue([deviceInstanceInfo.<field> unsignedShortValue]);
        }
    """
    pattern = (
        r"if\s*\(\s*deviceInstanceInfo\." + re.escape(field_name) + r"\s*!=\s*nil\s*\)"
        r"\s*\{[^}]*" + re.escape(optional_member) + r"\.SetValue\s*\("
    )
    match = re.search(pattern, content, re.DOTALL)
    assert match is not None, (
        f"Initialize() does not correctly store '{field_name}' into "
        f"'{optional_member}' via SetValue when non-nil"
    )


def _extract_string_getter_body(content: str, getter: str) -> str:
    """
    Extract the full method body for a string getter by finding the
    method signature and then matching braces to find the closing brace.
    """
    sig_pattern = (
        r"CHIP_ERROR\s+MCDeviceInstanceInfoProvider::" + re.escape(getter)
        + r"\s*\(\s*char\s*\*\s*buf\s*,\s*size_t\s*bufSize\s*\)\s*\{"
    )
    sig_match = re.search(sig_pattern, content)
    assert sig_match is not None, (
        f"Could not find method signature for {getter}(char * buf, size_t bufSize)"
    )
    # Walk forward from the opening brace, counting braces
    start = sig_match.end()  # position right after the opening {
    depth = 1
    pos = start
    while pos < len(content) and depth > 0:
        if content[pos] == "{":
            depth += 1
        elif content[pos] == "}":
            depth -= 1
        pos += 1
    return content[start: pos - 1]  # exclude the final }


def _verify_string_getter_override_branch(content: str, getter: str, flag: str, buffer: str):
    """
    Verify that the string getter checks the flag, verifies bufSize,
    and copies from the internal buffer when the flag is set.

    Expected pattern inside Get<Field>(char * buf, size_t bufSize):
        if (<flag>) {
            VerifyOrReturnError(bufSize >= strlen(<buffer>) + 1, CHIP_ERROR_BUFFER_TOO_SMALL);
            chip::Platform::CopyString(buf, bufSize, <buffer>);
            return CHIP_NO_ERROR;
        }
    """
    body = _extract_string_getter_body(content, getter)

    # Check flag guard
    assert re.search(r"if\s*\(\s*" + re.escape(flag) + r"\s*\)", body), (
        f"{getter} does not check '{flag}' to determine if override is present"
    )

    # Check buffer-too-small guard
    assert re.search(
        r"VerifyOrReturnError\s*\(\s*bufSize\s*>=\s*strlen\s*\(\s*"
        + re.escape(buffer) + r"\s*\)\s*\+\s*1\s*,\s*CHIP_ERROR_BUFFER_TOO_SMALL\s*\)",
        body,
    ), (
        f"{getter} does not check bufSize against strlen({buffer}) + 1 "
        f"and return CHIP_ERROR_BUFFER_TOO_SMALL"
    )

    # Check CopyString from buffer
    assert re.search(
        r"CopyString\s*\(\s*buf\s*,\s*bufSize\s*,\s*" + re.escape(buffer) + r"\s*\)",
        body,
    ), (
        f"{getter} does not copy from '{buffer}' into output buf when flag is set"
    )


def _verify_string_getter_fallback_branch(content: str, getter: str):
    """
    Verify that the string getter delegates to mDefaultProvider when
    the flag is not set.

    Expected pattern at end of method:
        return mDefaultProvider.Get<Field>(buf, bufSize);
    """
    body = _extract_string_getter_body(content, getter)

    assert re.search(
        r"return\s+mDefaultProvider\." + re.escape(getter) + r"\s*\(\s*buf\s*,\s*bufSize\s*\)\s*;",
        body,
    ), (
        f"{getter} does not delegate to mDefaultProvider.{getter}(buf, bufSize) "
        f"when the override flag is not set"
    )


def _extract_numeric_getter_body_and_param(content: str, getter: str):
    """
    Extract the full method body and parameter name for a numeric getter
    by finding the method signature and then matching braces.
    """
    sig_pattern = (
        r"CHIP_ERROR\s+MCDeviceInstanceInfoProvider::" + re.escape(getter)
        + r"\s*\(\s*uint16_t\s*&\s*(\w+)\s*\)\s*\{"
    )
    sig_match = re.search(sig_pattern, content)
    assert sig_match is not None, (
        f"Could not find method signature for {getter}(uint16_t &)"
    )
    param_name = sig_match.group(1)
    start = sig_match.end()
    depth = 1
    pos = start
    while pos < len(content) and depth > 0:
        if content[pos] == "{":
            depth += 1
        elif content[pos] == "}":
            depth -= 1
        pos += 1
    return content[start: pos - 1], param_name


def _verify_numeric_getter_override_branch(content: str, getter: str, optional_member: str):
    """
    Verify that the numeric getter checks HasValue() on the Optional member
    and returns Value() when present.

    Expected pattern:
        if (<optional_member>.HasValue()) {
            <param> = <optional_member>.Value();
            return CHIP_NO_ERROR;
        }
    """
    body, _param_name = _extract_numeric_getter_body_and_param(content, getter)

    assert re.search(
        r"if\s*\(\s*" + re.escape(optional_member) + r"\.HasValue\s*\(\s*\)\s*\)",
        body,
    ), (
        f"{getter} does not check '{optional_member}.HasValue()' for override"
    )

    assert re.search(
        re.escape(optional_member) + r"\.Value\s*\(\s*\)",
        body,
    ), (
        f"{getter} does not return '{optional_member}.Value()' when override is present"
    )


def _verify_numeric_getter_fallback_branch(content: str, getter: str):
    """
    Verify that the numeric getter delegates to mDefaultProvider when
    the Optional does not have a value.
    """
    body, param_name = _extract_numeric_getter_body_and_param(content, getter)

    assert re.search(
        r"return\s+mDefaultProvider\." + re.escape(getter)
        + r"\s*\(\s*" + re.escape(param_name) + r"\s*\)\s*;",
        body,
    ), (
        f"{getter} does not delegate to mDefaultProvider.{getter}({param_name}) "
        f"when the Optional has no value"
    )


def _verify_header_has_flag_and_buffer(header: str, flag: str, buffer: str):
    """Verify the header declares the boolean flag and char buffer."""
    assert re.search(r"bool\s+" + re.escape(flag) + r"\s*=\s*false\s*;", header), (
        f"Header missing declaration: bool {flag} = false;"
    )
    assert re.search(r"char\s+" + re.escape(buffer) + r"\s*\[", header), (
        f"Header missing char buffer declaration for {buffer}"
    )


def _verify_header_has_optional(header: str, optional_member: str):
    """Verify the header declares the chip::Optional member."""
    assert re.search(
        r"chip::Optional\s*<\s*uint16_t\s*>\s+" + re.escape(optional_member) + r"\s*;",
        header,
    ), (
        f"Header missing declaration: chip::Optional<uint16_t> {optional_member};"
    )


# ---------------------------------------------------------------------------
# Hypothesis strategies
# ---------------------------------------------------------------------------

# Strategy for a random subset of the 7 fields being "provided" (non-nil)
field_presence_strategy = st.fixed_dictionaries(
    {name: st.booleans() for name in ALL_FIELD_NAMES}
)


# ---------------------------------------------------------------------------
# Property-based tests
# ---------------------------------------------------------------------------


@given(field_presence=field_presence_strategy)
@settings(
    max_examples=128,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_override_fallback_roundtrip_initialize(field_presence):
    """
    **Validates: Requirements 1.2, 2.3, 3.2**

    Property 1 (Initialize): For any combination of nil/non-nil fields,
    Initialize() SHALL copy non-nil string values into internal buffers
    and set the corresponding flag, and store non-nil numeric values via
    SetValue on the Optional member. Nil fields leave flags false and
    Optionals empty.

    We verify this by checking that the source code of Initialize()
    contains the correct conditional copy/store pattern for each field.
    """
    mm_content = _read_file(PROVIDER_MM)

    for field_name, (flag, buffer, _getter) in STRING_FIELDS.items():
        if field_presence[field_name]:
            # Non-nil: Initialize must copy string and set flag
            _verify_initialize_copies_string(mm_content, field_name, flag, buffer)
        else:
            # Nil: the conditional block should NOT execute, so the flag
            # stays false (its default). Verify the guard is conditional
            # on != nil so nil values are skipped.
            pattern = (
                r"if\s*\(\s*deviceInstanceInfo\." + re.escape(field_name)
                + r"\s*!=\s*nil\s*\)"
            )
            assert re.search(pattern, mm_content), (
                f"Initialize() does not guard '{field_name}' copy behind "
                f"a nil check — nil values would incorrectly set the flag"
            )

    for field_name, (optional_member, _getter) in NUMERIC_FIELDS.items():
        if field_presence[field_name]:
            _verify_initialize_stores_numeric(mm_content, field_name, optional_member)
        else:
            pattern = (
                r"if\s*\(\s*deviceInstanceInfo\." + re.escape(field_name)
                + r"\s*!=\s*nil\s*\)"
            )
            assert re.search(pattern, mm_content), (
                f"Initialize() does not guard '{field_name}' store behind "
                f"a nil check — nil values would incorrectly set the Optional"
            )


@given(field_presence=field_presence_strategy)
@settings(
    max_examples=128,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_override_fallback_roundtrip_string_getters(field_presence):
    """
    **Validates: Requirements 3.3, 3.4, 3.5, 3.6, 3.11, 3.12, 3.15, 3.16**

    Property 1 (String Getters): For each string field, when the flag is
    set (non-nil field was provided), the getter SHALL check bufSize and
    copy from the internal buffer. When the flag is not set (nil field),
    the getter SHALL delegate to mDefaultProvider.
    """
    mm_content = _read_file(PROVIDER_MM)

    for field_name, (flag, buffer, getter) in STRING_FIELDS.items():
        if field_presence[field_name]:
            _verify_string_getter_override_branch(mm_content, getter, flag, buffer)
        else:
            _verify_string_getter_fallback_branch(mm_content, getter)


@given(field_presence=field_presence_strategy)
@settings(
    max_examples=128,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_override_fallback_roundtrip_numeric_getters(field_presence):
    """
    **Validates: Requirements 3.7, 3.8, 3.9, 3.10, 3.13, 3.14**

    Property 1 (Numeric Getters): For each numeric field, when the
    Optional has a value (non-nil field was provided), the getter SHALL
    return the stored value. When the Optional is empty (nil field),
    the getter SHALL delegate to mDefaultProvider.
    """
    mm_content = _read_file(PROVIDER_MM)

    for field_name, (optional_member, getter) in NUMERIC_FIELDS.items():
        if field_presence[field_name]:
            _verify_numeric_getter_override_branch(mm_content, getter, optional_member)
        else:
            _verify_numeric_getter_fallback_branch(mm_content, getter)


@given(field_presence=field_presence_strategy)
@settings(
    max_examples=128,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_override_fallback_roundtrip_header_declarations(field_presence):
    """
    **Validates: Requirements 3.2**

    Property 1 (Header Declarations): For each field, the header SHALL
    declare the appropriate private members — boolean flags and char
    buffers for string fields, chip::Optional<uint16_t> for numeric
    fields — ensuring the override/fallback mechanism has proper storage.
    """
    header_content = _read_file(PROVIDER_H)

    for field_name, (flag, buffer, _getter) in STRING_FIELDS.items():
        if field_presence[field_name]:
            _verify_header_has_flag_and_buffer(header_content, flag, buffer)

    for field_name, (optional_member, _getter) in NUMERIC_FIELDS.items():
        if field_presence[field_name]:
            _verify_header_has_optional(header_content, optional_member)


@given(field_presence=field_presence_strategy)
@settings(
    max_examples=128,
    suppress_health_check=[HealthCheck.function_scoped_fixture],
    deadline=None,
)
def test_override_fallback_roundtrip_buffer_too_small_guard(field_presence):
    """
    **Validates: Requirements 3.3, 3.5, 3.11, 3.15**

    Property 1 (Buffer Size Check): For each string field that is
    provided (non-nil), the getter SHALL check bufSize and return
    CHIP_ERROR_BUFFER_TOO_SMALL if the buffer is insufficient.
    """
    mm_content = _read_file(PROVIDER_MM)

    for field_name, (flag, buffer, getter) in STRING_FIELDS.items():
        if field_presence[field_name]:
            # The override branch must include the buffer-too-small guard
            body = _extract_string_getter_body(mm_content, getter)

            assert re.search(
                r"CHIP_ERROR_BUFFER_TOO_SMALL",
                body,
            ), (
                f"{getter} does not return CHIP_ERROR_BUFFER_TOO_SMALL "
                f"when bufSize is insufficient for the override value"
            )


# ---------------------------------------------------------------------------
# Allow running directly
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    import sys

    print("Running override/fallback round-trip property tests...")
    tests = [
        ("1a: Initialize copies/stores correctly", test_override_fallback_roundtrip_initialize),
        ("1b: String getters override/fallback", test_override_fallback_roundtrip_string_getters),
        ("1c: Numeric getters override/fallback", test_override_fallback_roundtrip_numeric_getters),
        ("1d: Header declarations", test_override_fallback_roundtrip_header_declarations),
        ("1e: Buffer-too-small guard", test_override_fallback_roundtrip_buffer_too_small_guard),
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
