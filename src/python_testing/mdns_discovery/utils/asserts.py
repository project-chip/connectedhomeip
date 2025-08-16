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

import re
import functools
import inspect

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
    asserts.assert_true(
        bool(re.fullmatch(r'[A-F0-9]{16}-[A-F0-9]{16}', instance_name)),
        f"Invalid operational instance name: '{instance_name}'"
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
    asserts.assert_true(
        bool(re.fullmatch(r'[A-F0-9]{16}', instance_name)),
        f"Invalid instance name: '{instance_name}'"
    )


@not_none_args
def assert_valid_hostname(hostname: str) -> None:
    """
    Verify that the DNS-SD hostname is valid.

    Constraints:
    - 48-bit device MAC address (for Ethernet and Wi-Fi)
    - Or 64-bit MAC Extended Address (for Thread)
    - Expressed as a fixed-length twelve-character (or
      sixteen-character) hexadecimal string, encoded as
      ASCII (UTF-8) text using capital letters

    Example:
        "B75AFB458ECD.local."

    Returns:
        None

    Raises:
        TestFailure: If `hostname` does not conform to the constraints.

    Spec:
        https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/secure_channel/Discovery.adoc#11-host-name-construction
    """
    asserts.assert_true(
        bool(re.fullmatch(
            r'(?:[0-9A-F]{12}|[0-9A-F]{16})\.(?:[A-Za-z0-9-]+\.)*[A-Za-z0-9-]+\.?',
            hostname
        )),
        f"Invalid hostname: '{hostname}'"
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
    - Format: "_L\<value\>._sub.\<commissionable-service-type\>"

    Example:
        "_L3840._sub._matterc._udp.local."

    Returns:
        None

    Raises:
        TestFailure: If `ld_subtype` does not conform to the constraints.

    Spec:
        https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/secure_channel/Discovery.adoc#13-commissioning-subtypes
    """
    m = re.fullmatch(
        rf'_L(?P<val>0|[1-9]\d{{0,3}})\._sub\.{re.escape(MdnsServiceType.COMMISSIONABLE.value)}',
        ld_subtype
    )

    is_match = bool(m)
    value_in_range = not is_match or int(m.group('val')) <= 4095
    valid = is_match and value_in_range

    asserts.assert_true(valid,f"Invalid long discriminator subtype: '{ld_subtype}'")


@not_none_args
def assert_valid_short_discriminator_subtype(sd_subtype: str) -> None:
    """
    Verify that the DNS-SD commissioning short discriminator subtype is valid.

    Constraints:
    - Upper 4-bits of the discriminator
    - Encoded as a variable-length decimal number in ASCII text
    - Omitting any leading zeroes
    - Format: "_S\<value\>._sub.\<commissionable-service-type\>"

    Example:
        "_S15._sub._matterc._udp.local."

    Returns:
        None

    Raises:
        TestFailure: If `sd_subtype` does not conform to the constraints.

    Spec:
        https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/secure_channel/Discovery.adoc#13-commissioning-subtypes
    """
    m = re.fullmatch(
        rf'_S(?P<val>0|[1-9]\d?)\._sub\.{re.escape(MdnsServiceType.COMMISSIONABLE.value)}',
        sd_subtype
    )

    is_match = bool(m)
    value_in_range = not is_match or int(m.group('val')) <= 15
    valid = is_match and value_in_range

    asserts.assert_true(valid,f"Invalid long discriminator subtype: '{sd_subtype}'")


@not_none_args
def assert_valid_vendor_subtype(vendor_subtype: str) -> None:
    """
    Verify that the DNS-SD commissioning vendor subtype is valid.

    Constraints:
    - 16-bit Vendor ID
    - Encoded as a variable-length decimal number in ASCII text
    - Omitting any leading zeroes
    - Format: "_V\<value\>._sub.\<commissionable-service-type\>"

    Example:
        "_V65521._sub._matterc._udp.local."

    Returns:
        None

    Raises:
        TestFailure: If `vendor_subtype` does not conform to the constraints.

    Spec:
        https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/secure_channel/Discovery.adoc#13-commissioning-subtypes
    """
    m = re.fullmatch(
        rf'_V(?P<val>0|[1-9]\d*)\._sub\.{re.escape(MdnsServiceType.COMMISSIONABLE.value)}',
        vendor_subtype
    )

    is_match = bool(m)
    value_in_range = not is_match or int(m.group('val')) <= 65535
    valid = is_match and value_in_range

    asserts.assert_true(valid, f"Invalid vendor subtype: '{vendor_subtype}'")


@not_none_args
def assert_valid_devtype_subtype(devtype_subtype: str) -> None:
    """
    Verify that the DNS-SD commissioning device type subtype is valid.

    Constraints:
    - 32-bit Devtype ID
    - Encoded as a variable-length decimal number in ASCII (UTF-8) text
    - Omitting any leading zeroes
    - Format: "_T\<value\>._sub.\<commissionable-service-type\>"

    Example:
        "_T10._sub._matterc._udp.local."

    Returns:
        None

    Raises:
        TestFailure: If `devtype_subtype` does not conform to the constraints.

    Spec:
        https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/secure_channel/Discovery.adoc#13-commissioning-subtypes
    """
    m = re.fullmatch(
        rf'_T(?P<val>0|[1-9]\d*)\._sub\.{re.escape(MdnsServiceType.COMMISSIONABLE.value)}',
        devtype_subtype
    )

    is_match = bool(m)
    value_in_range = not is_match or int(m.group('val')) <= 0xFFFFFFFF
    valid = is_match and value_in_range

    asserts.assert_true(valid, f"Invalid device type subtype: '{devtype_subtype}'")


# Commissionable Node Discovery TXT Record Keys

@not_none_args
def assert_valid_d_key(d_key: str) -> None:
    """
    **Discriminator**\n
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
    m = re.fullmatch(r'(?:D=)?(?P<val>0|[1-9]\d{0,3})', d_key)
    is_match = bool(m)
    value_in_range = not is_match or int(m.group('val')) <= 4095
    valid = is_match and value_in_range

    asserts.assert_true(valid, f"Invalid D key: '{d_key}'")


@not_none_args
def assert_valid_vp_key(vp_key: str) -> None:
    """
    **Vendor ID and Product ID**\n
    Verify that the TXT record VP key is valid.

    Constraints:
    - Shall contain at least the Vendor ID
    - If Product ID is present, it shall be separated
      from the Vendor ID using a '+' character

    Example:
        "123", "132+456"

    Returns:
        None

    Raises:
        TestFailure: If `vp_key` does not conform to the constraints.

    Spec:
        https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/secure_channel/Discovery.adoc#16-txt-key-for-vendor-id-and-product-id-vp
    """
    if '+' not in vp_key:
        # Vendor ID only
        assert_valid_vendor_id(vp_key)
    else:
        # Must contain exactly one '+'
        asserts.assert_true(vp_key.count('+') == 1, f"Invalid VP key: '{vp_key}'")

        vid_str, pid_str = vp_key.split('+', 1)
        assert_valid_vendor_id(vid_str)
        assert_valid_product_id(pid_str)


@not_none_args
def assert_valid_cm_key(cm_key: str) -> None:
    """
    **Commissioning Mode**\n
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
    m = re.fullmatch(r'(0|[1-3])', cm_key)
    valid = bool(m)

    asserts.assert_true(valid, f"Invalid CM key: '{cm_key}'")


@not_none_args
def assert_valid_dt_key(dt_key: str) -> None:
    """
    **Device Type**\n
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
    m = re.fullmatch(r'(0|[1-9]\d{0,9})', dt_key)
    is_match = bool(m)
    value_in_range = not is_match or int(dt_key) <= 0xFFFFFFFF
    valid = is_match and value_in_range

    asserts.assert_true(valid, f"Invalid DT key: '{dt_key}'")


@not_none_args
def assert_valid_dn_key(dn_key: str) -> None:
    """
    **Device Name**\n
    Verify that the TXT record DN key is valid.

    Constraints:
    - Encoded as a valid UTF-8 string
    - Maximum length of 32 bytes

    Example:
        "Living Room"

    Returns:
        None

    Raises:
        TestFailure: If `dn_key` does not conform to the constraints.

    Spec:
        https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/secure_channel/Discovery.adoc#19-txt-key-for-device-name-dn
    """
    try:
        valid = len(dn_key.encode("utf-8")) <= 32
    except UnicodeEncodeError:
        valid = False

    asserts.assert_true(valid, f"Invalid DN key: '{dn_key}'")


@not_none_args
def assert_valid_ri_key(ri_key: str) -> None:
    """
    **Rotating Device Identifier**\n
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
    valid = bool(re.fullmatch(r'[A-F0-9]{1,100}', ri_key))
    asserts.assert_true(valid, f"Invalid RI key: '{ri_key}'")


@not_none_args
def assert_valid_ph_key(ph_key: str) -> None:
    """
    **Pairing Hint**\n
    Verify that the TXT record PH key is valid.

    Constraints:
    - Encoded as a variable-length decimal number in ASCII text
    - Omitting any leading zeroes
    - Must be greater than 0
    - Valid bits are 0-19

    Example:
        "33"

    Returns:
        None

    Raises:
        TestFailure: If `ph_key` does not conform to the constraints.

    Spec:
        https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/secure_channel/Discovery.adoc#111-txt-key-for-pairing-hint-ph
    """
    if not re.fullmatch(r'[1-9]\d*', ph_key):
        valid = False
    else:
        v = int(ph_key)
        allowed_mask = (1 << 20) - 1  # only bits 0..19 allowed
        valid = (v & ~allowed_mask) == 0

    asserts.assert_true(valid, f"Invalid PH key: '{ph_key}'")


@not_none_args
def assert_valid_pi_key(pi_key: str) -> None:
    """
    **Pairing Instruction**\n
    Verify that the TXT record PI key is valid.

    Constraints:
    - Encoded as a valid UTF-8 string
    - Maximum length of 128 bytes

    Example:
        "10"

    Returns:
        None

    Raises:
        TestFailure: If `pi_key` does not conform to the constraints.

    Spec:
        https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/secure_channel/Discovery.adoc#112-txt-key-for-pairing-instructions-pi
    """
    try:
        valid = len(pi_key.encode("utf-8")) <= 128
    except UnicodeEncodeError:
        valid = False

    asserts.assert_true(valid, f"Invalid PI key: '{pi_key}'")


@not_none_args
def assert_valid_jf_key(jf_key: str) -> None:
    """
    **Joint Fabric**\n
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
    if not re.fullmatch(r'[1-9]\d*', jf_key):
        valid = False
    else:
        v = int(jf_key)
        valid = True

        # only bits 0..3 allowed
        if v & ~0xF:
            valid = False
        # bit0 cannot coexist with bits1..3
        if (v & 0x1) and (v & 0xE):
            valid = False
        # bit2 requires bit1
        if (v & 0x4) and not (v & 0x2):
            valid = False
        # bit3 requires bits1 and 2
        if (v & 0x8) and ((v & 0x6) != 0x6):
            valid = False

    asserts.assert_true(valid, f"Invalid JF key: '{jf_key}'")


# Common TXT Record Keys

@not_none_args
def assert_valid_sii_key(sii_key: str) -> None:
    """
    **Session Idle Interval**\n
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
    m = re.fullmatch(r'(0|[1-9]\d*)', sii_key)
    valid = bool(m) and int(sii_key) <= 3_600_000
    asserts.assert_true(valid, f"Invalid SII key: '{sii_key}'")


@not_none_args
def assert_valid_sai_key(sai_key: str) -> None:
    """
    **Session Active Interval**\n
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
    m = re.fullmatch(r'(0|[1-9]\d*)', sai_key)
    valid = bool(m) and int(sai_key) <= 3_600_000
    asserts.assert_true(valid, f"Invalid SAI key: '{sai_key}'")


@not_none_args
def assert_valid_sat_key(sat_key: str) -> None:
    """
    **Session Active Threshold**\n
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
    m = re.fullmatch(r'(0|[1-9]\d*)', sat_key)
    valid = bool(m) and int(sat_key) <= 65_535
    asserts.assert_true(valid, f"Invalid SAT key: '{sat_key}'")


@not_none_args
def assert_valid_t_key(t_key: str) -> None:
    """
    **Transport Protocol Modes**\n
    Verify that the TXT record T key is valid.

    Constraints:
    - Encoded as a decimal number in ASCII text
    - Omitting any leading zeros
    - Reject any value with bits 3 or higher set
    - Bit 0 is reserved and MUST be 0
    - Bits 1 and 2 are provisional and shall not be set

    Example:
        "0"

    Returns:
        None

    Raises:
        TestFailure: If `t_key` does not conform to the constraints.

    Spec:
        https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/secure_channel/Discovery.adoc#4-common-txt-keyvalue-pairs
    """
    m = re.fullmatch(r'(0|[1-9]\d*)', t_key)
    if not m:
        valid = False
    else:
        v = int(t_key)
        # Only bits 0–2 allowed, but all must be unset
        valid = (v & ~0x7) == 0 and (v & 0x7) == 0

    asserts.assert_true(valid, f"Invalid T key: '{t_key}'")


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

    Returns:
        None

    Raises:
        TestFailure: If `icd_key` does not conform to the constraints.

    Spec:
        https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/secure_channel/Discovery.adoc#4-common-txt-keyvalue-pairs
    """
    valid = bool(re.fullmatch(r'[01]', icd_key))
    asserts.assert_true(valid, f"Invalid ICD key: '{icd_key}'")


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
    m = re.fullmatch(r'(0|[1-9]\d{0,4})', vendor_id)
    valid = bool(m) and int(vendor_id) <= 65_535
    asserts.assert_true(valid, f"Invalid Vendor ID: '{vendor_id}'")


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
    m = re.fullmatch(r'(0|[1-9]\d{0,4})', product_id)
    valid = bool(m) and int(product_id) <= 65_535
    asserts.assert_true(valid, f"Invalid Product ID: '{product_id}'")


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
