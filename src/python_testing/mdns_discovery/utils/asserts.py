#
#    Copyright (c) 2025 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

import functools
import inspect
import ipaddress
import re

from mdns_discovery.mdns_discovery import MdnsServiceType
from mobly import asserts
from mobly.signals import TestFailure


def not_none_args(func):
    """
    Decorator that asserts all arguments
    passed to the function are not None.
    """
    @functools.wraps(func)
    def wrapper(*args, **kwargs):
        bound = inspect.signature(func).bind(*args, **kwargs)
        bound.apply_defaults()

        for arg_name, value in bound.arguments.items():
            asserts.assert_is_not_none(value, f"{func.__name__}: '{arg_name}' cannot be None")

        return func(*args, **kwargs)
    return wrapper

# Discovery


@not_none_args
def assert_valid_operational_instance_name(instance_name: str) -> None:
    """
    Verify that the DNS-SD operational instance name is valid.

    Constraints:
    - 64-bit compressed Fabric identifier, and a 64-bit Node identifier
    - Each expressed as a fixed-length sixteen-character hexadecimal string
    - Encoded as ASCII (UTF-8) text using capital letters
    - Separated by a hyphen

    Example:
        "B7322C948581262F-0000000012344321"

    Raises:
        TestFailure: If `instance_name` does not conform to the constraints.

    Spec:
        https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/secure_channel/Discovery.adoc#21-operational-instance-name
    """
    consts = [
        'Contains exactly one hyphen separating two parts',
        'Each part is exactly 16 characters long',
        'Each part only contains hexadecimal uppercase characters [A-F0-9]',
    ]

    failed: list[str] = []
    parts = instance_name.split('-')

    # Hyphen count must yield exactly two parts
    if len(parts) != 2:
        failed.append(consts[0])
    else:
        # Evaluate length and charset independently (when hyphen is correct),
        # so tests can observe multiple failures at once if applicable.
        part_a, part_b = parts[0], parts[1]

        # Length checks
        len_ok_a = len(part_a) == 16
        len_ok_b = len(part_b) == 16
        if not (len_ok_a and len_ok_b):
            failed.append(consts[1])

        # Charset checks (evaluate regardless of length so we can accumulate)
        hex_ok_a = bool(re.fullmatch(r'[A-F0-9]+', part_a))
        hex_ok_b = bool(re.fullmatch(r'[A-F0-9]+', part_b))
        if not (hex_ok_a and hex_ok_b):
            failed.append(consts[2])

    asserts.assert_true(
        not failed,
        f"Invalid operational instance name: '{instance_name}', failed constraint(s): {failed}"
    )


@not_none_args
def assert_valid_commissionable_instance_name(instance_name: str) -> None:
    """
    Verify that the DNS-SD commissionable instance name is valid.

    Constraints:
    - 64-bit identifier
    - Expressed as a fixed-length sixteen-character hexadecimal string
    - Encoded as ASCII (UTF-8) text using capital letters

    Example:
        "DD200C20D25AE5F7"

    Raises:
        TestFailure: If `instance_name` does not conform to the constraints.

    Spec:
        https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/secure_channel/Discovery.adoc#1-commissionable-node-discovery
    """
    consts = [
        'Length must be exactly 16 characters',
        'Must only contain hexadecimal uppercase characters [A-F0-9]',
    ]

    failed: list[str] = []

    # Evaluate constraints independently so multiple failures can be reported together
    if len(instance_name) != 16:
        failed.append(consts[0])

    # Require exactly 16 uppercase hex characters for charset constraint
    if not re.fullmatch(r'[A-F0-9]{16}', instance_name):
        failed.append(consts[1])

    asserts.assert_true(
        not failed,
        f"Invalid commissionable instance name: '{instance_name}', failed constraint(s): {failed}"
    )


@not_none_args
def assert_valid_hostname(hostname: str) -> None:
    """
    Verify that the DNS-SD hostname is valid.

    Constraints:
    - 48-bit device MAC address (for Ethernet and Wi-Fi) → 12 hex chars
    - Or 64-bit MAC Extended Address (for Thread) → 16 hex chars
    - Must be followed by a valid domain suffix
    - Hex part must be uppercase [A-F0-9]

    Example:
        "B75AFB458ECD.local."

    Raises:
        TestFailure: If `hostname` does not conform to the constraints.

    Spec:
        https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/secure_channel/Discovery.adoc#11-host-name-construction
    """
    constraints = [
        "Must start with 12 or 16 uppercase hexadecimal characters [A-F0-9]",
        "Must be followed by a valid domain suffix (e.g., .local.)"
    ]

    failed: list[str] = []

    # Independent check 1: Hex prefix (uppercase A-F and digits), length 12 or 16
    m = re.match(r'^([A-F0-9]+)', hostname)
    leading_hex = m.group(1) if m else ''
    if len(leading_hex) not in (12, 16):
        failed.append(constraints[0])

    # Independent check 2: Domain suffix after the first dot, regardless of hex validity
    dot_idx = hostname.find('.')
    if dot_idx == -1:
        # No dot at all -> suffix invalid
        failed.append(constraints[1])
    else:
        domain_part = hostname[dot_idx + 1:]  # everything after the first '.'
        # Must be label(s) of [A-Za-z0-9-], separated by dots; optional trailing dot allowed
        if not domain_part or not re.fullmatch(r'[A-Za-z0-9-]+(?:\.[A-Za-z0-9-]+)*\.?', domain_part):
            failed.append(constraints[1])

    asserts.assert_true(
        not failed,
        f"Invalid hostname: '{hostname}', failed constraint(s): {failed}"
    )


# Commissionable Subtypes

@not_none_args
def assert_valid_long_discriminator_subtype(ld_subtype: str) -> None:
    """
    Verify that the DNS-SD commissioning long discriminator subtype is valid.

    Constraints:
    - Full 12-bits of the discriminator
    - Encoded as a variable-length decimal number in ASCII text
    - Omitting any leading zeroes
    - Format: "_L<value>._sub.<commissionable-service-type>"

    Example:
        "_L3840._sub._matterc._udp.local."

    Raises:
        TestFailure: If `ld_subtype` does not conform to the constraints.

    Spec:
        https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/secure_channel/Discovery.adoc#13-commissioning-subtypes
    """
    constraints = [
        "Must match format '_L<value>._sub.<commissionable-service-type>'",
        "Value must be a decimal integer without leading zeroes",
        "Value must be within 0-4095 (12-bit range)",
    ]

    failed: list[str] = []

    # Strict format check
    strict = re.compile(
        rf'_L(0|[1-9]\d{{0,3}})\._sub\.{re.escape(MdnsServiceType.COMMISSIONABLE.value)}'
    )
    if not strict.fullmatch(ld_subtype):
        failed.append(constraints[0])

    # Best-effort value extraction to evaluate DEC and RNG independently
    val_str = None
    m = re.search(r'_L(?P<val>[^.]*)', ld_subtype)
    if m:
        val_str = m.group('val')

    # Decimal without leading zeroes
    if val_str is None or not re.fullmatch(r'(0|[1-9]\d*)', val_str):
        failed.append(constraints[1])

    # Range (only when digits-only)
    if val_str is not None and re.fullmatch(r'\d+', val_str):
        try:
            v = int(val_str)
            if v > 4095:
                failed.append(constraints[2])
        except ValueError:
            failed.append(constraints[1])

    asserts.assert_true(
        not failed,
        f"Invalid long discriminator subtype: '{ld_subtype}', failed constraint(s): {failed}"
    )


@not_none_args
def assert_valid_short_discriminator_subtype(sd_subtype: str) -> None:
    """
    Verify that the DNS-SD commissioning short discriminator subtype is valid.

    Constraints:
    - Upper 4-bits of the discriminator
    - Encoded as a variable-length decimal number in ASCII text
    - Omitting any leading zeroes
    - Format: "_S<value>._sub.<commissionable-service-type>"

    Example:
        "_S15._sub._matterc._udp.local."

    Raises:
        TestFailure: If `sd_subtype` does not conform to the constraints.

    Spec:
        https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/secure_channel/Discovery.adoc#13-commissioning-subtypes
    """
    constraints = [
        "Must match format '_S<value>._sub.<commissionable-service-type>'",
        "Value must be a decimal integer without leading zeroes",
        "Value must be within 0-15 (4-bit range)",
    ]

    failed: list[str] = []

    # Loose capture for the value, so we can independently validate decimal/leading zeroes and range.
    # Also separately validate the full strict format.
    service = re.escape(MdnsServiceType.COMMISSIONABLE.value)

    # Strict format check (structure only)
    if not re.fullmatch(rf'_S[^.]*\._sub\.{service}', sd_subtype):
        failed.append(constraints[0])

    # Try to extract the value string between "_S" and "._sub."
    val_str: str | None = None
    m_val = re.search(r'_S(?P<val>[^.]*)\._sub\.', sd_subtype)
    if m_val:
        val_str = m_val.group("val")

    # Decimal without leading zeroes
    if val_str is None or not re.fullmatch(r'(0|[1-9]\d*)', val_str):
        failed.append(constraints[1])

    # Range check if the value is digits (allow leading zeros here to enable multi-fail)
    if val_str is not None and re.fullmatch(r'\d+', val_str):
        try:
            val = int(val_str)
            if val > 15:
                failed.append(constraints[2])
        except ValueError:
            # Ignore; decimal check already captured above
            pass

    asserts.assert_true(
        not failed,
        f"Invalid short discriminator subtype: '{sd_subtype}', failed constraint(s): {failed}"
    )


@not_none_args
def assert_valid_vendor_subtype(vendor_subtype: str) -> None:
    """
    Verify that the DNS-SD commissioning vendor subtype is valid.

    Constraints:
    - 16-bit Vendor ID
    - Encoded as a variable-length decimal number in ASCII text
    - Omitting any leading zeroes
    - Format: "_V<value>._sub.<commissionable-service-type>"

    Example:
        "_V65521._sub._matterc._udp.local."

    Raises:
        TestFailure: If `vendor_subtype` does not conform to the constraints.

    Spec:
        https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/secure_channel/Discovery.adoc#13-commissioning-subtypes
    """
    constraints = [
        "Must match format '_V<value>._sub.<commissionable-service-type>'",
        "Value must be a decimal integer without leading zeroes",
        "Value must be within 0-65535 (16-bit range)",
    ]

    failed: list[str] = []

    service = re.escape(MdnsServiceType.COMMISSIONABLE.value)

    # Strict format check (structure & placement of tokens)
    strict = re.fullmatch(rf'_V(?P<val>0|[1-9]\d*)\._sub\.{service}', vendor_subtype)
    if not strict:
        failed.append(constraints[0])

    # Try to extract the raw value using a loose pattern so we can independently
    # validate decimal/leading-zero and range even when the strict format fails.
    m_val = re.search(r'_V(?P<val>[^.]*)\._sub\.', vendor_subtype)
    val_str = m_val.group("val") if m_val else None

    # Decimal without leading zeroes (except "0")
    if val_str is None or not re.fullmatch(r'(0|[1-9]\d*)', val_str):
        failed.append(constraints[1])

    # Range check whenever value is all digits (even if leading zeroes were present)
    if val_str is not None and re.fullmatch(r'\d+', val_str):
        try:
            v = int(val_str)
            if v > 65_535:
                failed.append(constraints[2])
        except ValueError:
            # Decimal check already recorded above
            pass

    asserts.assert_true(
        not failed,
        f"Invalid vendor subtype: '{vendor_subtype}', failed constraint(s): {failed}"
    )


@not_none_args
def assert_valid_devtype_subtype(devtype_subtype: str) -> None:
    """
    Verify that the DNS-SD commissioning device type subtype is valid.

    Constraints:
    - 32-bit Devtype ID
    - Encoded as a variable-length decimal number in ASCII (UTF-8) text
    - Omitting any leading zeroes
    - Format: "_T<value>._sub.<commissionable-service-type>"

    Example:
        "_T10._sub._matterc._udp.local."

    Raises:
        TestFailure: If `devtype_subtype` does not conform to the constraints.

    Spec:
        https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/secure_channel/Discovery.adoc#13-commissioning-subtypes
    """
    constraints = [
        "Must match format '_T<value>._sub.<commissionable-service-type>'",
        "Value must be a decimal integer without leading zeroes",
        "Value must be within 0-4294967295 (32-bit range)",
    ]

    failed: list[str] = []

    # Strict format check (enforces correct service type and no leading zeros)
    strict_re = re.compile(
        rf'_T(0|[1-9]\d*)\._sub\.{re.escape(MdnsServiceType.COMMISSIONABLE.value)}'
    )
    format_ok = bool(strict_re.fullmatch(devtype_subtype))
    if not format_ok:
        failed.append(constraints[0])

    # Best-effort value extraction so we can independently check DEC and RNG even if format fails.
    val_str: str | None = None

    # Try to extract value between '_T' and the next dot, regardless of other pieces.
    m = re.search(r'_T(?P<val>[^.]*)', devtype_subtype)
    if m:
        val_str = m.group('val')

    # Decimal-without-leading-zeroes rule
    if val_str is None or not re.fullmatch(r'(0|[1-9]\d*)', val_str):
        failed.append(constraints[1])

    # Range rule: only if we can parse an integer
    if val_str is not None and re.fullmatch(r'\d+', val_str):
        try:
            val = int(val_str)
            if val > 0xFFFFFFFF:
                failed.append(constraints[2])
        except ValueError:
            # Shouldn't happen due to \d+ guard; keep parity with safety.
            failed.append(constraints[1])

    asserts.assert_true(
        not failed,
        f"Invalid device type subtype: '{devtype_subtype}', failed constraint(s): {failed}"
    )


# Commissionable Node Discovery TXT Record Keys

@not_none_args
def assert_valid_d_key(d_key: str) -> None:
    """
    **Discriminator**

    Verify that the TXT record D key is valid.

    Constraints:
    - Full 12-bit discriminator
    - Encoded as a variable-length decimal number in ASCII text
    - Omitting any leading zeroes
    - Up to four digits

    Example:
        "3840"

    Returns:
        None

    Raises:
        TestFailure: If `d_key` does not conform to the constraints.

    Spec:
        https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/secure_channel/Discovery.adoc#15-txt-key-for-discriminator-d
    """
    constraints = [
        "Must be a decimal integer",
        "Value must be a decimal integer without leading zeroes",
        "Value must be within 0-4095 (12-bit range)",
    ]

    failed: list[str] = []

    is_decimal = bool(re.fullmatch(r'\d+', d_key))
    if not is_decimal:
        failed.append(constraints[0])

    # Leading zeroes check applies only to decimal strings
    if is_decimal and not re.fullmatch(r'(0|[1-9]\d*)', d_key):
        failed.append(constraints[1])

    # Range check applies only to decimal strings
    if is_decimal:
        try:
            val = int(d_key)
            if val > 4095:
                failed.append(constraints[2])
        except ValueError:
            # Shouldn't occur because is_decimal implies only digits,
            # but keep parity with original safety.
            failed.append(constraints[0])

    asserts.assert_true(
        not failed,
        f"Invalid D key: '{d_key}', failed constraint(s): {failed}"
    )


@not_none_args
def assert_valid_vp_key(vp_key: str) -> None:
    """
    **Vendor ID and Product ID**

    Verify that the TXT record VP key is valid.

    Constraints:
    - Must contain at least the Vendor ID
    - If Product ID is present, it must be separated
      from the Vendor ID using a '+' character
    - Only one '+' separator is allowed
    - Both Vendor ID and Product ID must be valid decimal integers

    Examples:
        "123"
        "132+456"

    Raises:
        TestFailure: If `vp_key` does not conform to the constraints.

    Spec:
        https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/secure_channel/Discovery.adoc#16-txt-key-for-vendor-id-and-product-id-vp
    """
    constraints = [
        "Must contain at least a Vendor ID",
        "If Product ID is present, there must be exactly one '+' separator",
        "Vendor ID must be a valid decimal integer",
        "Product ID (if present) must be a valid decimal integer",
    ]

    failed: list[str] = []

    if '+' not in vp_key:
        # Vendor ID only
        try:
            assert_valid_vendor_id(vp_key)
        except (TestFailure, ValueError):
            failed.append(constraints[2])
    else:
        # '+' present
        if vp_key.count('+') != 1:
            failed.append(constraints[1])
        else:
            vid_str, pid_str = vp_key.split('+', 1)

            # Vendor ID check
            try:
                assert_valid_vendor_id(vid_str)
            except (TestFailure, ValueError):
                failed.append(constraints[2])

            # Product ID check runs regardless
            try:
                if pid_str == "":
                    raise ValueError("empty product id")
                assert_valid_product_id(pid_str)
            except (TestFailure, ValueError):
                failed.append(constraints[3])

    asserts.assert_true(
        not failed,
        f"Invalid VP key: '{vp_key}', failed constraint(s): {failed}"
    )


@not_none_args
def assert_valid_cm_key(cm_key: str) -> None:
    """
    **Commissioning Mode**

    Verify that the TXT record CM key is valid.

    Constraints:
    - Allowed values are 0, 1, 2, 3
    - Encoded as a decimal number in ASCII text

    Example:
        "2"

    Raises:
        TestFailure: If `cm_key` does not conform to the constraints.

    Spec:
        https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/secure_channel/Discovery.adoc#17-txt-key-for-commissioning-mode-cm
    """
    constraints = [
        "Must be a decimal number",
        "Value must be one of: 0, 1, 2, 3",
    ]

    failed: list[str] = []

    is_decimal = bool(re.fullmatch(r'\d+', cm_key))
    if not is_decimal:
        failed.append(constraints[0])

    # Independently evaluate membership so multiple failures can be reported together
    if cm_key not in {"0", "1", "2", "3"}:
        failed.append(constraints[1])

    asserts.assert_true(
        not failed,
        f"Invalid CM key: '{cm_key}', failed constraint(s): {failed}"
    )


@not_none_args
def assert_valid_dt_key(dt_key: str) -> None:
    """
    **Device Type**

    Verify that the TXT record DT key is valid.

    Constraints:
    - 32-bit value
    - Variable-length decimal number in ASCII text
    - Omitting any leading zeroes

    Example:
        "10"

    Raises:
        TestFailure: If `dt_key` does not conform to the constraints.

    Spec:
        https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/secure_channel/Discovery.adoc#18-txt-key-for-device-type-dt
    """
    constraints = [
        "Must be a decimal integer without leading zeroes",
        "Value must be within 0-4294967295 (32-bit range)",
    ]

    failed: list[str] = []

    # Independently evaluate the integer/leading-zero rule
    is_int_no_leading = bool(re.fullmatch(r'(0|[1-9]\d*)', dt_key))
    if not is_int_no_leading:
        failed.append(constraints[0])

    # Independently evaluate the range rule whenever it's numeric (digits only)
    if re.fullmatch(r'\d+', dt_key):
        try:
            val = int(dt_key)
            if val > 0xFFFFFFFF:
                failed.append(constraints[1])
        except ValueError:
            # Defensive: shouldn't happen with \d+ guard
            failed.append(constraints[0])

    asserts.assert_true(
        not failed,
        f"Invalid DT key: '{dt_key}', failed constraint(s): {failed}"
    )


@not_none_args
def assert_valid_dn_key(dn_key: str) -> None:
    """
    **Device Name**

    Verify that the TXT record DN key is valid.

    Constraints:
    - Must be encodable as a valid UTF-8 string
    - Maximum length of 32 bytes when UTF-8 encoded

    Example:
        "Living Room"

    Raises:
        TestFailure: If `dn_key` does not conform to the constraints.

    Spec:
        https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/secure_channel/Discovery.adoc#19-txt-key-for-device-name-dn
    """
    constraints = [
        "Must be a valid UTF-8 string",
        "UTF-8 encoded length must be ≤ 32 bytes",
    ]

    failed: list[str] = []

    try:
        encoded = dn_key.encode("utf-8")
        if len(encoded) > 32:
            failed.append(constraints[1])
    except UnicodeEncodeError:
        failed.append(constraints[0])

    asserts.assert_true(
        not failed,
        f"Invalid DN key: '{dn_key}', failed constraint(s): {failed}"
    )


@not_none_args
def assert_valid_ri_key(ri_key: str) -> None:
    """
    **Rotating Device Identifier**

    Verify that the TXT record RI key is valid.

    Constraints:
    - Uppercase hexadecimal ASCII (A-F, 0-9)
    - Maximum length of 100 characters
    - No separators or prefixes

    Example:
        "0A1B2C3D"

    Raises:
        TestFailure: If ri_key does not conform to the constraints.

    Spec:
        https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/secure_channel/Discovery.adoc#110-txt-key-for-rotating-device-identifier-ri
    """
    constraints = [
        "Must only contain uppercase hexadecimal characters [A-F0-9]",
        "Length must be between 1 and 100 characters",
    ]

    failed: list[str] = []

    # Check charset independently of length
    if not re.fullmatch(r'[A-F0-9]+', ri_key):
        failed.append(constraints[0])

    # Check length independently of charset
    if not (1 <= len(ri_key) <= 100):
        failed.append(constraints[1])

    asserts.assert_true(
        not failed,
        f"Invalid RI key: '{ri_key}', failed constraint(s): {failed}"
    )


@not_none_args
def assert_valid_ph_key(ph_key: str) -> None:
    """
    **Pairing Hint**

    Verify that the TXT record PH key is valid.

    Constraints:
    - Encoded as a variable-length decimal number in ASCII text
    - Omitting any leading zeroes
    - Must be greater than 0
    - Only bits 0-22 are valid

    Example:
        "33"

    Raises:
        TestFailure: If `ph_key` does not conform to the constraints.

    Spec:
        https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/secure_channel/Discovery.adoc#111-txt-key-for-pairing-hint-ph
    """
    constraints = [
        "Must be a decimal integer without leading zeroes",
        "Value must be greater than 0",
        "Only bits 0-22 may be set (value must fit in 23 bits)",
    ]

    failed: list[str] = []

    # Evaluate constraints independently to accumulate failures where meaningful.
    is_digits = bool(re.fullmatch(r'\d+', ph_key))
    is_dec_no_leading = bool(re.fullmatch(r'(0|[1-9]\d*)', ph_key))
    if not is_dec_no_leading:
        failed.append(constraints[0])

    if is_digits:
        try:
            v = int(ph_key)
            if v <= 0:
                failed.append(constraints[1])
            # Bit-mask validity only meaningful for positive integers
            allowed_mask = (1 << 23) - 1
            if v > 0 and (v & ~allowed_mask):
                failed.append(constraints[2])
        except ValueError:
            failed.append(constraints[0])

    asserts.assert_true(
        not failed,
        f"Invalid PH key: '{ph_key}', failed constraint(s): {failed}"
    )


@not_none_args
def assert_valid_pi_key(pi_key: str) -> None:
    """
    **Pairing Instruction**

    Verify that the TXT record PI key is valid.

    Constraints:
    - Must be encodable as a valid UTF-8 string
    - Maximum length of 128 bytes when UTF-8 encoded

    Example:
        "10"

    Raises:
        TestFailure: If `pi_key` does not conform to the constraints.

    Spec:
        https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/secure_channel/Discovery.adoc#112-txt-key-for-pairing-instructions-pi
    """
    constraints = [
        "Must be a valid UTF-8 string",
        "UTF-8 encoded length must be ≤ 128 bytes",
    ]

    failed: list[str] = []

    try:
        encoded = pi_key.encode("utf-8")
        if len(encoded) > 128:
            failed.append(constraints[1])
    except UnicodeEncodeError:
        failed.append(constraints[0])

    asserts.assert_true(
        not failed,
        f"Invalid PI key: '{pi_key}', failed constraint(s): {failed}"
    )


@not_none_args
def assert_valid_ph_pi_relationship(txt: dict[str, str]) -> None:
    """
    Verify the relationship between Pairing Hint (PH) and Pairing Instruction (PI) keys.

    Constraints:
    - If any of bits 4, 8, 10, 12, 15, 17, 19, 20, 21, or 22 are set in PH, then PI MUST be present.
    - If PI is present, PH MUST be present and have at least one of the bits 4, 8, 9, 10, 11, 12, 15, 16, 17, 18, 19, 20, 21, or 22 set.

    Raises:
        TestFailure: If the relationship constraints are violated.
    """
    ph_key = txt.get('PH')
    pi_key = txt.get('PI')

    if ph_key:
        try:
            ph_val = int(ph_key)
            mandatory_pi_bits = [4, 8, 10, 12, 15, 17, 19, 20, 21, 22]
            if any((ph_val & (1 << bit)) for bit in mandatory_pi_bits):
                asserts.assert_in('PI', txt, f"'PI' key must be present if any of bits {mandatory_pi_bits} are set in 'PH' key.")
        except (ValueError, TypeError):
            pass

    if pi_key is not None:
        asserts.assert_in('PH', txt, "'PH' key must be present if 'PI' key is present.")
        try:
            ph_val = int(ph_key)
            pi_related_bits = [4, 8, 9, 10, 11, 12, 15, 16, 17, 18, 19, 20, 21, 22]
            asserts.assert_true(any((ph_val & (1 << bit)) for bit in pi_related_bits),
                                f"'PH' key must have at least one of bits {pi_related_bits} set if 'PI' key is present.")
        except (ValueError, TypeError):
            pass


@not_none_args
def assert_valid_jf_key(jf_key: str) -> None:
    """
    **Joint Fabric**

    Verify that the TXT record JF key is valid.

    Constraints:
    - Encoded as a variable-length decimal number in ASCII text
    - Omitting any leading zeroes
    - Reject any value with bits 4 or higher set
    - Bit 0 cannot coexist with bits 1-3
    - Bit 2 requires bit 1
    - Bit 3 requires both bits 1 and 2

    Example:
        "14"

    Raises:
        TestFailure: If `jf_key` does not conform to the constraints.

    Spec:
        https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/secure_channel/Discovery.adoc#113-txt-key-for-joint-fabric
    """
    constraints = [
        "Must be a decimal integer without leading zeroes",
        "Only bits 0-3 may be set (value must fit in 4 bits)",
        "Bit 0 cannot coexist with bits 1-3",
        "Bit 2 requires bit 1",
        "Bit 3 requires both bits 1 and 2",
    ]

    failed: list[str] = []

    # Decimal check without leading zeroes
    if not re.fullmatch(r'(0|[1-9]\d*)', jf_key):
        failed.append(constraints[0])
    else:
        try:
            v = int(jf_key)

            # Evaluate all bit constraints independently to accumulate failures
            if v & ~0xF:
                failed.append(constraints[1])
            if (v & 0x1) and (v & 0xE):
                failed.append(constraints[2])
            if (v & 0x4) and not (v & 0x2):
                failed.append(constraints[3])
            if (v & 0x8) and ((v & 0x6) != 0x6):
                failed.append(constraints[4])

        except ValueError:
            failed.append(constraints[0])

    asserts.assert_true(
        not failed,
        f"Invalid JF key: '{jf_key}', failed constraint(s): {failed}"
    )


# Common TXT Record Keys

@not_none_args
def assert_valid_sii_key(sii_key: str) -> None:
    """
    **Session Idle Interval**

    Verify that the TXT record SII key is valid.

    Constraints:
    - Unsigned integer with units of milliseconds
    - Encoded as a variable-length decimal number in ASCII encoding
    - Omitting any leading zeros
    - Maximum value of 3600000 (1 hour in milliseconds)

    Example:
        "5300"

    Raises:
        TestFailure: If `sii_key` does not conform to the constraints.

    Spec:
        https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/secure_channel/Discovery.adoc#4-common-txt-keyvalue-pairs
    """
    constraints = [
        "Must be a decimal integer without leading zeroes",
        "Value must be ≤ 3600000 (1 hour in milliseconds)",
    ]

    failed: list[str] = []

    # Check decimal-without-leading-zeroes independently
    if not re.fullmatch(r'(0|[1-9]\d*)', sii_key):
        failed.append(constraints[0])

    # Range check if it's digits-only (even if leading zeroes are present)
    if re.fullmatch(r'\d+', sii_key):
        try:
            val = int(sii_key)
            if val > 3_600_000:
                failed.append(constraints[1])
        except ValueError:
            # Defensive: parsing failed, format already captured above if relevant
            pass

    asserts.assert_true(
        not failed,
        f"Invalid SII key: '{sii_key}', failed constraint(s): {failed}"
    )


@not_none_args
def assert_valid_sai_key(sai_key: str) -> None:
    """
    **Session Active Interval**

    Verify that the TXT record SAI key is valid.

    Constraints:
    - Unsigned integer with units of milliseconds
    - Encoded as a variable-length decimal number in ASCII encoding
    - Omitting any leading zeros
    - Maximum value of 3600000 (1 hour in milliseconds)

    Example:
        "1250"

    Raises:
        TestFailure: If `sai_key` does not conform to the constraints.

    Spec:
        https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/secure_channel/Discovery.adoc#4-common-txt-keyvalue-pairs
    """
    constraints = [
        "Must be a decimal integer without leading zeroes",
        "Value must be ≤ 3600000 (1 hour in milliseconds)",
    ]

    failed: list[str] = []

    # Format (no leading zeroes) — evaluated independently
    if not re.fullmatch(r'(0|[1-9]\d*)', sai_key):
        failed.append(constraints[0])

    # Range — parse if it's all digits (even if leading zeroes present) to allow multi-fail
    if re.fullmatch(r'\d+', sai_key):
        try:
            val = int(sai_key)
            if val > 3_600_000:
                failed.append(constraints[1])
        except ValueError:
            # If somehow int() fails despite digits-only, still mark format as bad (already added above if applicable)
            pass

    asserts.assert_true(
        not failed,
        f"Invalid SAI key: '{sai_key}', failed constraint(s): {failed}"
    )


@not_none_args
def assert_valid_sat_key(sat_key: str) -> None:
    """
    **Session Active Threshold**

    Verify that the TXT record SAT key is valid.

    Constraints:
    - Unsigned integer with units of milliseconds
    - Encoded as a variable-length decimal number in ASCII encoding
    - Omitting any leading zeros
    - Maximum value of 65535 (65.535 seconds)

    Example:
        "1250"

    Raises:
        TestFailure: If `sat_key` does not conform to the constraints.

    Spec:
        https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/secure_channel/Discovery.adoc#4-common-txt-keyvalue-pairs
    """
    constraints = [
        "Must be a decimal integer without leading zeroes",
        "Value must be ≤ 65535 (65.535 seconds)",
    ]

    failed: list[str] = []

    # Format (no leading zeroes) — evaluated independently
    if not re.fullmatch(r'(0|[1-9]\d*)', sat_key):
        failed.append(constraints[0])

    # Range — parse if it's all digits (even if leading zeroes present) to allow multi-fail
    if re.fullmatch(r'\d+', sat_key):
        try:
            val = int(sat_key)
            if val > 65_535:
                failed.append(constraints[1])
        except ValueError:
            # If parse fails despite digits, ignore (format already captured)
            pass

    asserts.assert_true(
        not failed,
        f"Invalid SAT key: '{sat_key}', failed constraint(s): {failed}"
    )


@not_none_args
def assert_valid_t_key(t_key: str, enforce_provisional: bool = True) -> None:
    """
    **Transport Protocol Modes**

    Verify that the TXT record T key is valid.

    Constraints:
    - Encoded as a decimal number in ASCII text
    - Omitting any leading zeros
    - Reject any value with bits 3 or higher set
    - Bit 0 is reserved and MUST be 0
    - Bits 1 and 2 are provisional:
        * If enforce_provisional=True → must not be set
        * If enforce_provisional=False → allowed, only bit 0 must be clear

    Example:
        "0"

    Args:
        t_key (str): The value to validate
        enforce_provisional (bool): Whether to enforce strict prohibition
                                    of provisional bits 1 and 2 (default: True)

    Raises:
        TestFailure: If `t_key` does not conform to the constraints.

    Spec:
        https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/secure_channel/Discovery.adoc#4-common-txt-keyvalue-pairs
    """
    constraints = [
        "Must be a decimal integer without leading zeroes",
        "Only bits 0-2 may be present (value must fit in 3 bits)",
        "Bit 0 is reserved and must be 0",
        "Bits 1 and 2 are provisional and must not be set (strict mode)",
    ]

    failed: list[str] = []

    # Integer format (no leading zeroes except "0") — independent of bit checks
    if not re.fullmatch(r'(0|[1-9]\d*)', t_key):
        failed.append(constraints[0])

    # Bit checks: run whenever it's digits-only (even if leading zeros were present)
    if re.fullmatch(r'\d+', t_key):
        try:
            v = int(t_key)
            # Evaluate all conditions independently (no elif gating)
            if v & ~0x7:
                failed.append(constraints[1])   # bits above 2 present
            if v & 0x1:
                failed.append(constraints[2])   # bit 0 must be 0
            if enforce_provisional and (v & 0x6):
                failed.append(constraints[3])   # bits 1 or 2 set (strict mode)
        except ValueError:
            # Defensive: if parsing somehow fails, treat as integer-format error
            failed.append(constraints[0])

    asserts.assert_true(
        not failed,
        f"Invalid T key: '{t_key}', failed constraint(s): {failed}"
    )


@not_none_args
def assert_valid_icd_key(icd_key: str) -> None:
    """
    **Intermittently Connected Device**\n
    Verify that the TXT record ICD key is valid.

    Constraints:
    - Encoded as a decimal number in ASCII text
    - Omitting any leading zeros
    - Allowed values: 0 or 1

    Example:
        "1"

    Raises:
        TestFailure: If `icd_key` does not conform to the constraints.

    Spec:
        https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/secure_channel/Discovery.adoc#4-common-txt-keyvalue-pairs
    """
    consts = [
        'Encoded as a decimal number in ASCII text',
        'Omitting any leading zeros',
        'Allowed values: 0 or 1',
    ]

    failed: list[str] = []

    # Check decimal ASCII
    is_decimal = bool(re.fullmatch(r'\d+', icd_key))
    if not is_decimal:
        failed.append(consts[0])

    # Check leading zeros (applies only to purely decimal strings)
    if is_decimal and not re.fullmatch(r'0|[1-9]\d*', icd_key):
        failed.append(consts[1])

    # Check allowed set independently (string membership)
    if icd_key not in ("0", "1"):
        failed.append(consts[2])

    asserts.assert_true(
        not failed,
        f"Invalid ICD key: '{icd_key}', failed constraint(s): {failed}"
    )


# Device Identity Attributes

@not_none_args
def assert_valid_vendor_id(vendor_id: str) -> None:
    """
    Verify that the Vendor ID is valid.

    Constraints:
    - 16-bit value
    - Encoded as a variable-length decimal number in ASCII text
    - Omitting any leading zeroes
    - Up to five digits

    Example:
        "123", "54632"

    Raises:
        TestFailure: If `vendor_id` does not conform to the constraints.

    Spec:
        https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/secure_channel/Discovery.adoc#16-txt-key-for-vendor-id-and-product-id-vp
    """
    constraints = [
        "Must be a decimal integer without leading zeroes",
        "Value must be within 0-65535 (16-bit range)",
    ]

    failed: list[str] = []

    # Check integer/format constraint independently
    if not re.fullmatch(r'(0|[1-9]\d{0,4})', vendor_id):
        failed.append(constraints[0])

    # Check numeric range constraint independently (if digits-only)
    if re.fullmatch(r'\d+', vendor_id):
        try:
            val = int(vendor_id)
            if val > 65_535:
                failed.append(constraints[1])
        except ValueError:
            failed.append(constraints[0])

    asserts.assert_true(
        not failed,
        f"Invalid Vendor ID: '{vendor_id}', failed constraint(s): {failed}"
    )


@not_none_args
def assert_valid_product_id(product_id: str) -> None:
    """
    Verify that the Product ID is valid.

    Constraints:
    - 16-bit value
    - Encoded as a variable-length decimal number in ASCII text
    - Omitting any leading zeroes
    - Up to five digits

    Example:
        "456", "21387"

    Raises:
        TestFailure: If `product_id` does not conform to the constraints.

    Spec:
        https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/secure_channel/Discovery.adoc#16-txt-key-for-vendor-id-and-product-id-vp
    """
    constraints = [
        "Must be a decimal integer without leading zeroes",
        "Value must be within 0-65535 (16-bit range)",
    ]

    failed: list[str] = []

    # Syntactic check: decimal without leading zeroes (except "0") and up to 5 digits
    syntactic_ok = bool(re.fullmatch(r'(0|[1-9]\d{0,4})', product_id))
    if not syntactic_ok:
        failed.append(constraints[0])
    else:
        # Range check only when the syntax is valid
        try:
            val = int(product_id)
            if val > 65_535:
                failed.append(constraints[1])
        except ValueError:
            # Defensive: should not happen when syntactic_ok is True
            failed.append(constraints[0])

    asserts.assert_true(
        not failed,
        f"Invalid Product ID: '{product_id}', failed constraint(s): {failed}"
    )


# Service Types

@not_none_args
def assert_is_commissionable_type(commissionable_type: str) -> None:
    """
    Verify that the given service has the correct
    mDNS service type for a commissionable node.

    Constraints:
    - The service type must equal `_matterc._udp.local.`

    Raises:
        TestFailure: If `commissionable_type` does not equal `_matterc._udp.local.`
    """
    asserts.assert_equal(
        commissionable_type,
        MdnsServiceType.COMMISSIONABLE.value,
        f"Invalid commissionable service type: '{commissionable_type}', must be '{MdnsServiceType.COMMISSIONABLE.value}'"
    )


@not_none_args
def assert_is_commissioner_type(commissioner_type: str) -> None:
    """
    Verify that the given service has the correct
    mDNS service type for a commissioner node.

    Constraints:
    - The service type must equal `_matterd._udp.local.`

    Raises:
        TestFailure: If `commissioner_type` does not equal `_matterd._udp.local.`
    """
    asserts.assert_equal(
        commissioner_type,
        MdnsServiceType.COMMISSIONER.value,
        f"Invalid commissioner service type: '{commissioner_type}', must be '{MdnsServiceType.COMMISSIONER.value}'"
    )


@not_none_args
def assert_is_operational_type(operational_type: str) -> None:
    """
    Verify that the given service has the correct
    mDNS service type for an operational node.

    Constraints:
    - The service type must equal `_matter._tcp.local.`

    Raises:
        TestFailure: If `operational_type` does not equal `_matter._tcp.local.`
    """
    asserts.assert_equal(
        operational_type,
        MdnsServiceType.OPERATIONAL.value,
        f"Invalid operational service type: '{operational_type}', must be '{MdnsServiceType.OPERATIONAL.value}'"
    )


@not_none_args
def assert_is_border_router_type(border_router_type: str) -> None:
    """
    Verify that the given service has the correct
    mDNS service type for a border router.

    Constraints:
    - The service type must equal `_meshcop._udp.local.`

    Raises:
        TestFailure: If `border_router_type` does not equal `_meshcop._udp.local.`
    """
    asserts.assert_equal(
        border_router_type,
        MdnsServiceType.BORDER_ROUTER.value,
        f"Invalid border router service type: '{border_router_type}', must be '{MdnsServiceType.BORDER_ROUTER.value}'"
    )


# Other

@not_none_args
def assert_valid_ipv6_addresses(addresses: list[str]) -> None:
    """
    Verify that all given addresses are valid IPv6 addresses.

    Args:
        addresses (list[str]): A list of strings representing IPv6 addresses.

    Raises:
        TestFailure: If any of the addresses are not valid IPv6 addresses.
    """
    invalid = []
    for addr in addresses:
        try:
            ipaddress.IPv6Address(addr)
        except ipaddress.AddressValueError:
            invalid.append(addr)

    if invalid:
        asserts.fail(f"Invalid IPv6 addresses: {invalid}")
