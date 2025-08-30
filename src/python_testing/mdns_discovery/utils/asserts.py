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

    Returns:
        None

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

    failed = None
    parts = instance_name.split('-')

    # 1. Must contain exactly one hyphen → two parts
    if len(parts) != 2:
        failed = consts[0]

    # 2. Each part must be 16 characters
    elif not all(len(p) == 16 for p in parts):
        failed = consts[1]

    # 3. Each part must be uppercase hex only
    elif not all(re.fullmatch(r'[A-F0-9]{16}', p) for p in parts):
        failed = consts[2]

    asserts.assert_true(
        failed is None,
        f"Invalid operational instance name: '{instance_name}', failed constraint: [{failed}]"
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

    Returns:
        None

    Raises:
        TestFailure: If `instance_name` does not conform to the constraints.

    Spec:
        https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/secure_channel/Discovery.adoc#1-commissionable-node-discovery
    """
    consts = [
        'Length must be exactly 16 characters',
        'Must only contain hexadecimal uppercase characters [A-F0-9]',
    ]

    failed = None

    # 1. Length must be exactly 16 characters
    if len(instance_name) != 16:
        failed = consts[0]

    # 2. Must be uppercase hex only
    elif not re.fullmatch(r'[A-F0-9]{16}', instance_name):
        failed = consts[1]

    asserts.assert_true(
        failed is None,
        f"Invalid commissionable instance name: '{instance_name}', failed constraint: [{failed}]"
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

    Returns:
        None

    Raises:
        TestFailure: If `hostname` does not conform to the constraints.

    Spec:
        https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/secure_channel/Discovery.adoc#11-host-name-construction
    """
    constraints = [
        "Must start with 12 or 16 uppercase hexadecimal characters [A-F0-9]",
        "Must be followed by a valid domain suffix (e.g., .local.)"
    ]

    failed = None

    # Split into hex prefix + domain suffix
    match = re.fullmatch(r'([A-F0-9]{12}|[A-F0-9]{16})(\..+)', hostname)
    if not match:
        failed = constraints[0]
    else:
        hex_part, domain_part = match.groups()

        # Validate domain suffix
        if not re.fullmatch(r'[A-Za-z0-9-]+(?:\.[A-Za-z0-9-]+)*\.?', domain_part[1:]):
            failed = constraints[1]

    asserts.assert_true(
        failed is None,
        f"Invalid hostname: '{hostname}', failed constraint: [{failed}]"
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

    Returns:
        None

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

    failed = None
    m = re.fullmatch(
        rf'_L(?P<val>0|[1-9]\d{{0,3}})\._sub\.{re.escape(MdnsServiceType.COMMISSIONABLE.value)}',
        ld_subtype
    )

    if not m:
        failed = constraints[0]
    else:
        val_str = m.group("val")
        try:
            val = int(val_str)
            if val > 4095:
                failed = constraints[2]
        except ValueError:
            failed = constraints[1]

    asserts.assert_true(
        failed is None,
        f"Invalid long discriminator subtype: '{ld_subtype}', failed constraint: [{failed}]"
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

    Returns:
        None

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

    failed = None
    m = re.fullmatch(
        rf'_S(?P<val>0|[1-9]\d?)\._sub\.{re.escape(MdnsServiceType.COMMISSIONABLE.value)}',
        sd_subtype
    )

    if not m:
        failed = constraints[0]
    else:
        val_str = m.group("val")
        try:
            val = int(val_str)
            if val > 15:
                failed = constraints[2]
        except ValueError:
            failed = constraints[1]

    asserts.assert_true(
        failed is None,
        f"Invalid short discriminator subtype: '{sd_subtype}', failed constraint: [{failed}]"
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

    Returns:
        None

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

    failed = None
    m = re.fullmatch(
        rf'_V(?P<val>0|[1-9]\d*)\._sub\.{re.escape(MdnsServiceType.COMMISSIONABLE.value)}',
        vendor_subtype
    )

    if not m:
        failed = constraints[0]
    else:
        val_str = m.group("val")
        try:
            val = int(val_str)
            if val > 65535:
                failed = constraints[2]
        except ValueError:
            failed = constraints[1]

    asserts.assert_true(
        failed is None,
        f"Invalid vendor subtype: '{vendor_subtype}', failed constraint: [{failed}]"
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

    Returns:
        None

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

    failed = None
    m = re.fullmatch(
        rf'_T(?P<val>0|[1-9]\d*)\._sub\.{re.escape(MdnsServiceType.COMMISSIONABLE.value)}',
        devtype_subtype
    )

    if not m:
        failed = constraints[0]
    else:
        val_str = m.group("val")
        try:
            val = int(val_str)
            if val > 0xFFFFFFFF:
                failed = constraints[2]
        except ValueError:
            failed = constraints[1]

    asserts.assert_true(
        failed is None,
        f"Invalid device type subtype: '{devtype_subtype}', failed constraint: [{failed}]"
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

    failed = None
    # Require strictly a number, no optional "D=" prefix
    m = re.fullmatch(r'(0|[1-9]\d{0,3})', d_key)

    if not m:
        failed = constraints[0]
    else:
        val_str = m.group(0)
        try:
            val = int(val_str)
            if val > 4095:
                failed = constraints[2]
        except ValueError:
            failed = constraints[1]

    asserts.assert_true(
        failed is None,
        f"Invalid D key: '{d_key}', failed constraint: [{failed}]"
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

    Returns:
        None

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

    failed = None

    if '+' not in vp_key:
        # Vendor ID only
        try:
            assert_valid_vendor_id(vp_key)
        except Exception:
            failed = constraints[2]
    else:
        # Must contain exactly one '+'
        if vp_key.count('+') != 1:
            failed = constraints[1]
        else:
            vid_str, pid_str = vp_key.split('+', 1)
            try:
                assert_valid_vendor_id(vid_str)
            except Exception:
                failed = constraints[2]

            if failed is None:
                try:
                    assert_valid_product_id(pid_str)
                except Exception:
                    failed = constraints[3]

    asserts.assert_true(
        failed is None,
        f"Invalid VP key: '{vp_key}', failed constraint: [{failed}]"
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

    Returns:
        None

    Raises:
        TestFailure: If `cm_key` does not conform to the constraints.

    Spec:
        https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/secure_channel/Discovery.adoc#17-txt-key-for-commissioning-mode-cm
    """
    constraints = [
        "Must be a decimal number",
        "Value must be one of: 0, 1, 2, 3",
    ]

    failed = None
    m = re.fullmatch(r'(0|[1-3])', cm_key)

    if not m:
        # Either not decimal or not in the allowed set
        failed = constraints[1]

    asserts.assert_true(
        failed is None,
        f"Invalid CM key: '{cm_key}', failed constraint: [{failed}]"
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

    Returns:
        None

    Raises:
        TestFailure: If `dt_key` does not conform to the constraints.

    Spec:
        https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/secure_channel/Discovery.adoc#18-txt-key-for-device-type-dt
    """
    constraints = [
        "Must be a decimal integer without leading zeroes",
        "Value must be within 0-4294967295 (32-bit range)",
    ]

    failed = None
    m = re.fullmatch(r'(0|[1-9]\d{0,9})', dt_key)

    if not m:
        failed = constraints[0]
    else:
        try:
            val = int(dt_key)
            if val > 0xFFFFFFFF:
                failed = constraints[1]
        except ValueError:
            failed = constraints[0]

    asserts.assert_true(
        failed is None,
        f"Invalid DT key: '{dt_key}', failed constraint: [{failed}]"
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

    Returns:
        None

    Raises:
        TestFailure: If `dn_key` does not conform to the constraints.

    Spec:
        https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/secure_channel/Discovery.adoc#19-txt-key-for-device-name-dn
    """
    constraints = [
        "Must be a valid UTF-8 string",
        "UTF-8 encoded length must be ≤ 32 bytes",
    ]

    failed = None
    try:
        encoded = dn_key.encode("utf-8")
        if len(encoded) > 32:
            failed = constraints[1]
    except UnicodeEncodeError:
        failed = constraints[0]

    asserts.assert_true(
        failed is None,
        f"Invalid DN key: '{dn_key}', failed constraint: [{failed}]"
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

    Returns:
        None

    Raises:
        TestFailure: If `ri_key` does not conform to the constraints.

    Spec:
        https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/secure_channel/Discovery.adoc#110-txt-key-for-rotating-device-identifier-ri
    """
    constraints = [
        "Must only contain uppercase hexadecimal characters [A-F0-9]",
        "Length must be between 1 and 100 characters",
    ]

    failed = None

    if not re.fullmatch(r'[A-F0-9]+', ri_key):
        failed = constraints[0]
    elif len(ri_key) > 100:
        failed = constraints[1]

    asserts.assert_true(
        failed is None,
        f"Invalid RI key: '{ri_key}', failed constraint: [{failed}]"
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
    - Only bits 0-19 are valid

    Example:
        "33"

    Returns:
        None

    Raises:
        TestFailure: If `ph_key` does not conform to the constraints.

    Spec:
        https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/secure_channel/Discovery.adoc#111-txt-key-for-pairing-hint-ph
    """
    constraints = [
        "Must be a decimal integer without leading zeroes",
        "Value must be greater than 0",
        "Only bits 0-19 may be set (value must fit in 20 bits)",
    ]

    failed = None

    if not re.fullmatch(r'[1-9]\d*', ph_key):
        failed = constraints[0]
    else:
        try:
            v = int(ph_key)
            if v <= 0:
                failed = constraints[1]
            else:
                allowed_mask = (1 << 20) - 1  # 20 valid bits
                if v & ~allowed_mask:
                    failed = constraints[2]
        except ValueError:
            failed = constraints[0]

    asserts.assert_true(
        failed is None,
        f"Invalid PH key: '{ph_key}', failed constraint: [{failed}]"
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

    Returns:
        None

    Raises:
        TestFailure: If `pi_key` does not conform to the constraints.

    Spec:
        https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/secure_channel/Discovery.adoc#112-txt-key-for-pairing-instructions-pi
    """
    constraints = [
        "Must be a valid UTF-8 string",
        "UTF-8 encoded length must be ≤ 128 bytes",
    ]

    failed = None
    try:
        encoded = pi_key.encode("utf-8")
        if len(encoded) > 128:
            failed = constraints[1]
    except UnicodeEncodeError:
        failed = constraints[0]

    asserts.assert_true(
        failed is None,
        f"Invalid PI key: '{pi_key}', failed constraint: [{failed}]"
    )


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

    Returns:
        None

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

    failed = None

    if not re.fullmatch(r'[1-9]\d*', jf_key):
        failed = constraints[0]
    else:
        try:
            v = int(jf_key)

            # only bits 0..3 allowed
            if v & ~0xF:
                failed = constraints[1]
            # bit0 cannot coexist with bits1..3
            elif (v & 0x1) and (v & 0xE):
                failed = constraints[2]
            # bit2 requires bit1
            elif (v & 0x4) and not (v & 0x2):
                failed = constraints[3]
            # bit3 requires bits1 and 2
            elif (v & 0x8) and ((v & 0x6) != 0x6):
                failed = constraints[4]

        except ValueError:
            failed = constraints[0]

    asserts.assert_true(
        failed is None,
        f"Invalid JF key: '{jf_key}', failed constraint: [{failed}]"
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

    Returns:
        None

    Raises:
        TestFailure: If `sii_key` does not conform to the constraints.

    Spec:
        https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/secure_channel/Discovery.adoc#4-common-txt-keyvalue-pairs
    """
    constraints = [
        "Must be a decimal integer without leading zeroes",
        "Value must be ≤ 3600000 (1 hour in milliseconds)",
    ]

    failed = None
    m = re.fullmatch(r'(0|[1-9]\d*)', sii_key)

    if not m:
        failed = constraints[0]
    else:
        try:
            val = int(sii_key)
            if val > 3_600_000:
                failed = constraints[1]
        except ValueError:
            failed = constraints[0]

    asserts.assert_true(
        failed is None,
        f"Invalid SII key: '{sii_key}', failed constraint: [{failed}]"
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

    Returns:
        None

    Raises:
        TestFailure: If `sai_key` does not conform to the constraints.

    Spec:
        https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/secure_channel/Discovery.adoc#4-common-txt-keyvalue-pairs
    """
    constraints = [
        "Must be a decimal integer without leading zeroes",
        "Value must be ≤ 3600000 (1 hour in milliseconds)",
    ]

    failed = None
    m = re.fullmatch(r'(0|[1-9]\d*)', sai_key)

    if not m:
        failed = constraints[0]
    else:
        try:
            val = int(sai_key)
            if val > 3_600_000:
                failed = constraints[1]
        except ValueError:
            failed = constraints[0]

    asserts.assert_true(
        failed is None,
        f"Invalid SAI key: '{sai_key}', failed constraint: [{failed}]"
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

    Returns:
        None

    Raises:
        TestFailure: If `sat_key` does not conform to the constraints.

    Spec:
        https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/secure_channel/Discovery.adoc#4-common-txt-keyvalue-pairs
    """
    constraints = [
        "Must be a decimal integer without leading zeroes",
        "Value must be ≤ 65535 (65.535 seconds)",
    ]

    failed = None
    m = re.fullmatch(r'(0|[1-9]\d*)', sat_key)

    if not m:
        failed = constraints[0]
    else:
        try:
            val = int(sat_key)
            if val > 65_535:
                failed = constraints[1]
        except ValueError:
            failed = constraints[0]

    asserts.assert_true(
        failed is None,
        f"Invalid SAT key: '{sat_key}', failed constraint: [{failed}]"
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

    failed = None

    m = re.fullmatch(r'(0|[1-9]\d*)', t_key)
    if not m:
        failed = constraints[0]
    else:
        try:
            v = int(t_key)
            # Only bits 0-2 allowed
            if v & ~0x7:
                failed = constraints[1]
            # Bit 0 reserved
            elif v & 0x1:
                failed = constraints[2]
            # Bits 1 and 2 provisional
            elif enforce_provisional and (v & 0x6):
                failed = constraints[3]
        except ValueError:
            failed = constraints[0]

    asserts.assert_true(
        failed is None,
        f"Invalid T key: '{t_key}', failed constraint: [{failed}]"
    )


@not_none_args
def assert_valid_icd_key(icd_key: str) -> None:
    consts = [
        'Encoded as a decimal number in ASCII text',
        'Omitting any leading zeros',
        'Allowed values: 0 or 1',
    ]

    failed = None

    # 1. Must be ASCII decimal text
    if not re.fullmatch(r'\d+', icd_key):
        failed = consts[0]

    # 2. No leading zeros (except "0")
    elif not re.fullmatch(r'0|[1-9]\d*', icd_key):
        failed = consts[1]

    # 3. Only 0 or 1 allowed
    elif icd_key not in ("0", "1"):
        failed = consts[2]

    asserts.assert_true(
        failed is None,
        f"Invalid ICD key: '{icd_key}', failed constraint: [{failed}]"
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

    Returns:
        None

    Raises:
        TestFailure: If `vendor_id` does not conform to the constraints.

    Spec:
        https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/secure_channel/Discovery.adoc#16-txt-key-for-vendor-id-and-product-id-vp
    """
    constraints = [
        "Must be a decimal integer without leading zeroes",
        "Value must be within 0-65535 (16-bit range)",
    ]

    failed = None
    m = re.fullmatch(r'(0|[1-9]\d{0,4})', vendor_id)

    if not m:
        failed = constraints[0]
    else:
        try:
            val = int(vendor_id)
            if val > 65_535:
                failed = constraints[1]
        except ValueError:
            failed = constraints[0]

    asserts.assert_true(
        failed is None,
        f"Invalid Vendor ID: '{vendor_id}', failed constraint: [{failed}]"
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

    Returns:
        None

    Raises:
        TestFailure: If `product_id` does not conform to the constraints.

    Spec:
        https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/secure_channel/Discovery.adoc#16-txt-key-for-vendor-id-and-product-id-vp
    """
    constraints = [
        "Must be a decimal integer without leading zeroes",
        "Value must be within 0-65535 (16-bit range)",
    ]

    failed = None
    m = re.fullmatch(r'(0|[1-9]\d{0,4})', product_id)

    if not m:
        failed = constraints[0]
    else:
        try:
            val = int(product_id)
            if val > 65_535:
                failed = constraints[1]
        except ValueError:
            failed = constraints[0]

    asserts.assert_true(
        failed is None,
        f"Invalid Product ID: '{product_id}', failed constraint: [{failed}]"
    )


# Service Types

@not_none_args
def assert_is_commissionable_type(commissionable_type: str) -> None:
    """
    Verify that the given service has the correct
    mDNS service type for a commissionable node.

    Constraints:
    - The service type must equal `_matterc._udp.local.`

    Returns:
        None

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

    Returns:
        None

    Raises:
        TestFailure: If `commissioner_type` does not equal `_matterd._udp.local.`
    """
    asserts.assert_equal(
        commissioner_type,
        MdnsServiceType.COMMISSIONER.value,
        f"Invalid commissionerservice type: '{commissioner_type}', must be '{MdnsServiceType.COMMISSIONER.value}'"
    )


@not_none_args
def assert_is_operational_type(operational_type: str) -> None:
    """
    Verify that the given service has the correct
    mDNS service type for an operational node.

    Constraints:
    - The service type must equal `_matter._tcp.local.`

    Returns:
        None

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

    Returns:
        None

    Raises:
        TestFailure: If `border_router_type` does not equal `_meshcop._udp.local.`
    """
    asserts.assert_equal(
        border_router_type,
        MdnsServiceType.BORDER_ROUTER.value,
        f"Invalid border router service type: '{border_router_type}', must be '{MdnsServiceType.BORDER_ROUTER.value}'"
    )


# Other

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
