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

import unittest

from mdns_discovery.utils.asserts import (assert_is_border_router_type, assert_is_commissionable_type, assert_is_commissioner_type,
                                          assert_is_operational_type, assert_valid_cm_key,
                                          assert_valid_commissionable_instance_name, assert_valid_d_key,
                                          assert_valid_devtype_subtype, assert_valid_dn_key, assert_valid_dt_key,
                                          assert_valid_hostname, assert_valid_icd_key, assert_valid_ipv6_addresses,
                                          assert_valid_jf_key, assert_valid_long_discriminator_subtype,
                                          assert_valid_operational_instance_name, assert_valid_ph_key,
                                          assert_valid_ph_pi_relationship, assert_valid_pi_key, assert_valid_product_id,
                                          assert_valid_ri_key, assert_valid_sai_key, assert_valid_sat_key,
                                          assert_valid_short_discriminator_subtype, assert_valid_sii_key, assert_valid_t_key,
                                          assert_valid_vendor_id, assert_valid_vendor_subtype, assert_valid_vp_key)
from mobly import signals


def fail_msg(assert_fn, *args, **kwargs) -> str:
    """Run an assert_* expecting failure; return its message."""
    try:
        assert_fn(*args, **kwargs)
    except (AssertionError, signals.TestFailure) as e:
        return str(e)
    raise AssertionError("Expected failure but assertion passed")


class TestAssertIsBorderRouterType(unittest.TestCase):
    EXPECTED = "_meshcop._udp.local."

    def test_valid_value(self):
        # Correct service type should pass
        assert_is_border_router_type(self.EXPECTED)

    def test_invalid_due_to_wrong_service_type(self):
        # Wrong service type should fail
        wrong = "_matter._tcp.local."
        msg = fail_msg(assert_is_border_router_type, wrong)
        self.assertIn("Invalid border router service type", msg)
        self.assertIn(self.EXPECTED, msg)
        self.assertIn(wrong, msg)

    def test_invalid_due_to_empty_string(self):
        # Empty string should fail
        msg = fail_msg(assert_is_border_router_type, "")
        self.assertIn("Invalid border router service type", msg)
        self.assertIn(self.EXPECTED, msg)

    def test_invalid_due_to_partial_match(self):
        # Missing trailing dot should fail
        partial = "_meshcop._udp.local"
        msg = fail_msg(assert_is_border_router_type, partial)
        self.assertIn("Invalid border router service type", msg)
        self.assertIn(self.EXPECTED, msg)
        self.assertIn(partial, msg)


class TestAssertIsCommissionableType(unittest.TestCase):
    EXPECTED = "_matterc._udp.local."

    def test_valid_value(self):
        # Correct service type should pass
        assert_is_commissionable_type(self.EXPECTED)

    def test_invalid_due_to_wrong_service_type(self):
        # Wrong service type should fail
        wrong = "_matterd._udp.local."
        msg = fail_msg(assert_is_commissionable_type, wrong)
        self.assertIn("Invalid commissionable service type", msg)
        self.assertIn(self.EXPECTED, msg)
        self.assertIn(wrong, msg)

    def test_invalid_due_to_empty_string(self):
        # Empty string should fail
        msg = fail_msg(assert_is_commissionable_type, "")
        self.assertIn("Invalid commissionable service type", msg)
        self.assertIn(self.EXPECTED, msg)

    def test_invalid_due_to_partial_match(self):
        # Missing trailing dot should fail
        partial = "_matterc._udp.local"
        msg = fail_msg(assert_is_commissionable_type, partial)
        self.assertIn("Invalid commissionable service type", msg)
        self.assertIn(self.EXPECTED, msg)
        self.assertIn(partial, msg)


class TestAssertIsCommissionerType(unittest.TestCase):
    EXPECTED = "_matterd._udp.local."

    def test_valid_value(self):
        # Correct service type should pass
        assert_is_commissioner_type(self.EXPECTED)

    def test_invalid_due_to_wrong_service_type(self):
        # Wrong service type should fail
        wrong = "_matterc._udp.local."
        msg = fail_msg(assert_is_commissioner_type, wrong)
        self.assertIn("Invalid commissioner service type", msg)
        self.assertIn(self.EXPECTED, msg)
        self.assertIn(wrong, msg)

    def test_invalid_due_to_empty_string(self):
        # Empty string should fail
        msg = fail_msg(assert_is_commissioner_type, "")
        self.assertIn("Invalid commissioner service type", msg)
        self.assertIn(self.EXPECTED, msg)

    def test_invalid_due_to_partial_match(self):
        # Missing trailing dot should fail
        partial = "_matterd._udp.local"
        msg = fail_msg(assert_is_commissioner_type, partial)
        self.assertIn("Invalid commissioner service type", msg)
        self.assertIn(self.EXPECTED, msg)
        self.assertIn(partial, msg)


class TestAssertIsOperationalType(unittest.TestCase):
    EXPECTED = "_matter._tcp.local."

    def test_valid_value(self):
        # Correct service type should pass
        assert_is_operational_type(self.EXPECTED)

    def test_invalid_due_to_wrong_service_type(self):
        # Wrong service type should fail
        wrong = "_matterc._udp.local."
        msg = fail_msg(assert_is_operational_type, wrong)
        self.assertIn("Invalid operational service type", msg)
        self.assertIn(self.EXPECTED, msg)
        self.assertIn(wrong, msg)

    def test_invalid_due_to_empty_string(self):
        # Empty string should fail
        msg = fail_msg(assert_is_operational_type, "")
        self.assertIn("Invalid operational service type", msg)
        self.assertIn(self.EXPECTED, msg)

    def test_invalid_due_to_partial_match(self):
        # Missing trailing dot should fail
        partial = "_matter._tcp.local"
        msg = fail_msg(assert_is_operational_type, partial)
        self.assertIn("Invalid operational service type", msg)
        self.assertIn(self.EXPECTED, msg)
        self.assertIn(partial, msg)


class TestAssertValidCmKey(unittest.TestCase):
    DEC_MSG = "Must be a decimal number"
    SET_MSG = "Value must be one of: 0, 1, 2, 3"

    # Valid values
    VALID_VALUES = [
        "0",  # lowest allowed
        "1",  # commissioning mode 1
        "2",  # commissioning mode 2
        "3",  # highest allowed
    ]

    def test_valid_values(self):
        # All values in VALID_VALUES should pass
        for value in self.VALID_VALUES:
            with self.subTest(value=value):
                assert_valid_cm_key(value)

    def test_invalid_due_to_non_decimal(self):
        # Not a decimal number -> now accumulates: non-decimal AND not in allowed set
        msg = fail_msg(assert_valid_cm_key, "A")
        self.assertIn(self.DEC_MSG, msg)
        self.assertIn(self.SET_MSG, msg)

    def test_invalid_due_to_negative_number(self):
        # Negative not allowed -> non-decimal and also not in allowed set
        msg = fail_msg(assert_valid_cm_key, "-1")
        self.assertIn(self.DEC_MSG, msg)
        self.assertIn(self.SET_MSG, msg)

    def test_invalid_due_to_out_of_range(self):
        # Decimal but outside allowed set
        msg = fail_msg(assert_valid_cm_key, "4")
        self.assertNotIn(self.DEC_MSG, msg)
        self.assertIn(self.SET_MSG, msg)

    def test_invalid_due_to_empty_string(self):
        # Empty string not valid -> non-decimal and also not in allowed set
        msg = fail_msg(assert_valid_cm_key, "")
        self.assertIn(self.DEC_MSG, msg)
        self.assertIn(self.SET_MSG, msg)

    def test_invalid_due_to_non_decimal_and_out_of_set_letter(self):
        # Triggers both: non-decimal and not in allowed set
        msg = fail_msg(assert_valid_cm_key, "x")
        self.assertIn(self.DEC_MSG, msg)
        self.assertIn(self.SET_MSG, msg)

    def test_invalid_due_to_non_decimal_and_out_of_set_mixed(self):
        # Mixed characters still should accumulate both failures
        msg = fail_msg(assert_valid_cm_key, "1a")
        self.assertIn(self.DEC_MSG, msg)
        self.assertIn(self.SET_MSG, msg)

    def test_invalid_due_to_non_decimal_and_out_of_set_empty(self):
        # Empty string accumulates both failures
        msg = fail_msg(assert_valid_cm_key, "")
        self.assertIn(self.DEC_MSG, msg)
        self.assertIn(self.SET_MSG, msg)


class TestAssertValidCommissionableInstanceName(unittest.TestCase):
    LEN_MSG = 'Length must be exactly 16 characters'
    HEX_MSG = 'Must only contain hexadecimal uppercase characters [A-F0-9]'

    # Valid values (all exactly 16 chars)
    VALID_VALUES = [
        "0000000000000000",   # all zeros
        "FFFFFFFFFFFFFFFF",   # all F's
        "1234567890ABCDEF",   # mix of digits and hex letters
        "ABCDEF1234567890",   # reversed mix
        "A1B2C3D4E5F60708",   # alternating pattern
        "90ABCDEF12345678",   # shifted pattern (fixed to 16 chars)
    ]

    def test_valid_values(self):
        # All values in VALID_VALUES should pass without raising
        for value in self.VALID_VALUES:
            with self.subTest(value=value):
                assert_valid_commissionable_instance_name(value)

    def test_invalid_due_to_wrong_length_short(self):
        # Invalid: only 15 characters → now fails length and charset (regex requires 16)
        msg = fail_msg(assert_valid_commissionable_instance_name, "1234567890ABCDE")
        self.assertIn(self.LEN_MSG, msg)
        self.assertIn(self.HEX_MSG, msg)

    def test_invalid_due_to_wrong_length_long(self):
        # Invalid: 17 characters → fails length and charset (regex requires 16)
        msg = fail_msg(assert_valid_commissionable_instance_name, "1234567890ABCDEFF")
        self.assertIn(self.LEN_MSG, msg)
        self.assertIn(self.HEX_MSG, msg)

    def test_invalid_due_to_non_hex_characters(self):
        # Invalid: contains 'G' (not hex) → charset only (length is correct)
        msg = fail_msg(assert_valid_commissionable_instance_name, "1234567890ABCDEG")
        self.assertNotIn(self.LEN_MSG, msg)
        self.assertIn(self.HEX_MSG, msg)

    def test_invalid_due_to_lowercase_characters(self):
        # Invalid: contains lowercase letters → charset only (length is correct)
        msg = fail_msg(assert_valid_commissionable_instance_name, "1234567890abcDEF")
        self.assertNotIn(self.LEN_MSG, msg)
        self.assertIn(self.HEX_MSG, msg)

    def test_invalid_due_to_empty_string(self):
        # Invalid: empty string → fails both length and charset
        msg = fail_msg(assert_valid_commissionable_instance_name, "")
        self.assertIn(self.LEN_MSG, msg)
        self.assertIn(self.HEX_MSG, msg)

    def test_invalid_due_to_short_and_non_hex(self):
        # Too short and includes lowercase → both failures
        msg = fail_msg(assert_valid_commissionable_instance_name, "abc")
        self.assertIn(self.LEN_MSG, msg)
        self.assertIn(self.HEX_MSG, msg)

    def test_invalid_due_to_long_and_non_hex(self):
        # Too long and includes non-hex 'G' → both failures
        msg = fail_msg(assert_valid_commissionable_instance_name, "1234567890ABCDEGHI")
        self.assertIn(self.LEN_MSG, msg)
        self.assertIn(self.HEX_MSG, msg)

    def test_invalid_due_to_all_spaces(self):
        # Exactly 16 spaces → length ok, charset fails
        msg = fail_msg(assert_valid_commissionable_instance_name, "                ")
        self.assertNotIn(self.LEN_MSG, msg)
        self.assertIn(self.HEX_MSG, msg)

    def test_invalid_due_to_mixed_case_and_short(self):
        # Short and contains lowercase → both failures
        msg = fail_msg(assert_valid_commissionable_instance_name, "AbCdEf123")
        self.assertIn(self.LEN_MSG, msg)
        self.assertIn(self.HEX_MSG, msg)


class TestAssertValidDKey(unittest.TestCase):
    DEC_MSG = "Must be a decimal integer"
    LZ_MSG = "Value must be a decimal integer without leading zeroes"
    RNG_MSG = "Value must be within 0-4095 (12-bit range)"

    # Valid values
    VALID_VALUES = [
        "0",       # minimum
        "1",       # smallest nonzero
        "1234",    # typical
        "3840",    # example from docstring
        "4095",    # maximum 12-bit
    ]

    def test_valid_values(self):
        # All values in VALID_VALUES should pass without raising
        for value in self.VALID_VALUES:
            with self.subTest(value=value):
                assert_valid_d_key(value)

    def test_invalid_due_to_non_decimal(self):
        # Contains non-decimal characters
        msg = fail_msg(assert_valid_d_key, "12AB")
        self.assertIn(self.DEC_MSG, msg)
        self.assertNotIn(self.LZ_MSG, msg)
        self.assertNotIn(self.RNG_MSG, msg)

    def test_invalid_due_to_prefix(self):
        # Optional "D=" prefix is not allowed
        msg = fail_msg(assert_valid_d_key, "D=123")
        self.assertIn(self.DEC_MSG, msg)

    def test_invalid_due_to_leading_zero(self):
        # Leading zeros not allowed (except "0"); within range so RNG should be absent
        msg = fail_msg(assert_valid_d_key, "0123")
        self.assertNotIn(self.DEC_MSG, msg)
        self.assertIn(self.LZ_MSG, msg)
        self.assertNotIn(self.RNG_MSG, msg)

    def test_invalid_due_to_out_of_range(self):
        # Above 12-bit maximum
        msg = fail_msg(assert_valid_d_key, "4096")
        self.assertNotIn(self.DEC_MSG, msg)
        self.assertNotIn(self.LZ_MSG, msg)
        self.assertIn(self.RNG_MSG, msg)

    def test_invalid_due_to_empty_string(self):
        # Empty string invalid
        msg = fail_msg(assert_valid_d_key, "")
        self.assertIn(self.DEC_MSG, msg)
        self.assertNotIn(self.LZ_MSG, msg)
        self.assertNotIn(self.RNG_MSG, msg)

    # Multi-failure: leading zeros and out-of-range
    def test_invalid_due_to_leading_zero_and_out_of_range(self):
        msg = fail_msg(assert_valid_d_key, "05000")
        self.assertNotIn(self.DEC_MSG, msg)
        self.assertIn(self.LZ_MSG, msg)
        self.assertIn(self.RNG_MSG, msg)

    # Another combined case within the same pattern
    def test_invalid_due_to_leading_zero_and_out_of_range_boundary(self):
        # 04096 -> leading zero present; numeric value 4096 is out of range
        msg = fail_msg(assert_valid_d_key, "04096")
        self.assertNotIn(self.DEC_MSG, msg)
        self.assertIn(self.LZ_MSG, msg)
        self.assertIn(self.RNG_MSG, msg)

    def test_non_decimal_cannot_combine_with_other_failures(self):
        # Non-decimal inputs cannot also fail leading-zero or range constraints,
        # since those are only applicable when the input is strictly decimal.
        self.skipTest("Multiple failures with DEC_MSG are not feasible: LZ/RNG apply only to decimal inputs.")


class TestAssertValidDevtypeSubtype(unittest.TestCase):
    FMT_MSG = "Must match format '_T<value>._sub.<commissionable-service-type>'"
    DEC_MSG = "Value must be a decimal integer without leading zeroes"
    RNG_MSG = "Value must be within 0-4294967295 (32-bit range)"

    # Valid values
    VALID_VALUES = [
        "_T0._sub._matterc._udp.local.",                 # minimum valid
        "_T1._sub._matterc._udp.local.",                 # smallest nonzero
        "_T12345._sub._matterc._udp.local.",             # typical mid-range
        "_T4294967295._sub._matterc._udp.local.",        # maximum valid (32-bit)
    ]

    def test_valid_values(self):
        # All valid values should pass without raising
        for value in self.VALID_VALUES:
            with self.subTest(value=value):
                assert_valid_devtype_subtype(value)

    def test_invalid_due_to_wrong_format(self):
        # Invalid: missing '_T' prefix
        msg = fail_msg(assert_valid_devtype_subtype, "123._sub._matterc._udp.local.")
        self.assertIn(self.FMT_MSG, msg)

    def test_invalid_due_to_leading_zero(self):
        # Invalid: leading zero not allowed except for zero itself
        msg = fail_msg(assert_valid_devtype_subtype, "_T0123._sub._matterc._udp.local.")
        self.assertIn(self.FMT_MSG, msg)
        self.assertIn(self.DEC_MSG, msg)
        self.assertNotIn(self.RNG_MSG, msg)

    def test_invalid_due_to_non_decimal_value(self):
        # Invalid: contains non-decimal characters
        msg = fail_msg(assert_valid_devtype_subtype, "_T12AB._sub._matterc._udp.local.")
        self.assertIn(self.FMT_MSG, msg)
        self.assertIn(self.DEC_MSG, msg)
        self.assertNotIn(self.RNG_MSG, msg)

    def test_invalid_due_to_value_out_of_range(self):
        # Invalid: 4294967296 is outside 32-bit range
        msg = fail_msg(assert_valid_devtype_subtype, "_T4294967296._sub._matterc._udp.local.")
        self.assertNotIn(self.DEC_MSG, msg)
        self.assertIn(self.RNG_MSG, msg)

    def test_invalid_due_to_missing_sub_suffix(self):
        # Invalid: missing '._sub.<service>' -> format fails, but value is fine and in range
        msg = fail_msg(assert_valid_devtype_subtype, "_T123._matterc._udp.local.")
        self.assertIn(self.FMT_MSG, msg)
        self.assertNotIn(self.DEC_MSG, msg)
        self.assertNotIn(self.RNG_MSG, msg)

    # Pairwise: format + decimal rule (non-decimal)
    def test_invalid_due_to_format_and_decimal_rule_non_decimal(self):
        msg = fail_msg(assert_valid_devtype_subtype, "_T1x3._sub._matterc._udp.local.")
        self.assertIn(self.FMT_MSG, msg)
        self.assertIn(self.DEC_MSG, msg)
        self.assertNotIn(self.RNG_MSG, msg)

    # Pairwise: format + range (format broken but numeric and out-of-range)
    def test_invalid_due_to_format_and_range_missing_sub_but_big(self):
        msg = fail_msg(assert_valid_devtype_subtype, "_T4294967296._matterc._udp.local.")
        self.assertIn(self.FMT_MSG, msg)
        self.assertNotIn(self.DEC_MSG, msg)
        self.assertIn(self.RNG_MSG, msg)

    # All three: format + decimal rule + range
    def test_invalid_due_to_format_decimal_rule_and_range(self):
        # Leading zero plus too-large value, with correct service section so only format is violated by leading zero
        msg = fail_msg(assert_valid_devtype_subtype, "_T04294967296._sub._matterc._udp.local.")
        self.assertIn(self.FMT_MSG, msg)
        self.assertIn(self.DEC_MSG, msg)
        self.assertIn(self.RNG_MSG, msg)

    def test_impossible_decimal_and_range_without_format(self):
        # DEC+RNG without FMT is impossible because strict format requires the decimal-without-leading-zeroes rule.
        self.skipTest("Multiple failures DEC+RNG without FMT are not feasible: strict format enforces the DEC rule.")


class TestAssertValidDnKey(unittest.TestCase):
    UTF8_MSG = "Must be a valid UTF-8 string"
    LEN_MSG = "UTF-8 encoded length must be ≤ 32 bytes"

    # Valid values
    VALID_VALUES = [
        "Living Room",                       # simple ASCII
        "Kitchen",                           # shorter ASCII
        "Cámara",                            # UTF-8 accented char
        "设备",                              # UTF-8 multibyte chars, within 32 bytes
        "12345678901234567890123456789012",  # exactly 32 ASCII chars
        "",                                  # empty string allowed (0 bytes)
    ]

    def test_valid_values(self):
        # All values in VALID_VALUES should pass
        for value in self.VALID_VALUES:
            with self.subTest(value=value):
                assert_valid_dn_key(value)

    def test_invalid_due_to_too_long_ascii(self):
        # 33 ASCII chars → length fail
        msg = fail_msg(assert_valid_dn_key, "123456789012345678901234567890123")
        self.assertIn(self.LEN_MSG, msg)
        self.assertNotIn(self.UTF8_MSG, msg)

    def test_invalid_due_to_too_long_utf8_multibyte(self):
        # 17 Chinese characters (each 3 bytes) → 51 bytes → length fail
        msg = fail_msg(assert_valid_dn_key, "设备" * 17)
        self.assertIn(self.LEN_MSG, msg)
        self.assertNotIn(self.UTF8_MSG, msg)

    def test_invalid_due_to_non_utf8(self):
        # Simulate by using surrogate that can't be encoded in UTF-8
        bad_string = "\udcff"
        msg = fail_msg(assert_valid_dn_key, bad_string)
        self.assertIn(self.UTF8_MSG, msg)
        self.assertNotIn(self.LEN_MSG, msg)

    def test_multiple_failures_not_possible_due_to_gating(self):
        # For DN, the length is defined only for strings that encode as valid UTF-8.
        # If UTF-8 encoding fails, we cannot evaluate the byte-length constraint.
        # Therefore, no input can trigger both UTF8_MSG and LEN_MSG simultaneously.
        self.skipTest("Multiple failures not possible: UTF-8 validity is prerequisite for byte-length evaluation.")


class TestAssertValidDtKey(unittest.TestCase):
    INT_MSG = "Must be a decimal integer without leading zeroes"
    RNG_MSG = "Value must be within 0-4294967295 (32-bit range)"

    # Valid values
    VALID_VALUES = [
        "0",                # minimum
        "1",                # smallest nonzero
        "10",               # typical
        "4294967295",       # maximum (32-bit)
        "123456789",        # mid-range
    ]

    def test_valid_values(self):
        # All values in VALID_VALUES should pass
        for value in self.VALID_VALUES:
            with self.subTest(value=value):
                assert_valid_dt_key(value)

    def test_invalid_due_to_leading_zero(self):
        # Leading zeros not allowed
        msg = fail_msg(assert_valid_dt_key, "0123")
        self.assertIn(self.INT_MSG, msg)
        self.assertNotIn(self.RNG_MSG, msg)

    def test_invalid_due_to_non_decimal(self):
        # Non-decimal string not valid
        msg = fail_msg(assert_valid_dt_key, "ABC")
        self.assertIn(self.INT_MSG, msg)

    def test_invalid_due_to_out_of_range(self):
        # Greater than 32-bit max
        msg = fail_msg(assert_valid_dt_key, "4294967296")
        self.assertIn(self.RNG_MSG, msg)

    def test_invalid_due_to_empty_string(self):
        # Empty string not valid
        msg = fail_msg(assert_valid_dt_key, "")
        self.assertIn(self.INT_MSG, msg)

    def test_invalid_due_to_leading_zero_and_out_of_range(self):
        # Numeric with leading zero and value > 32-bit → INT and RNG should both appear
        msg = fail_msg(assert_valid_dt_key, "04294967296")
        self.assertIn(self.INT_MSG, msg)
        self.assertIn(self.RNG_MSG, msg)

    def test_non_decimal_cannot_combine_with_range(self):
        # Non-decimal inputs can't trigger RNG because they aren't parsed as numbers
        self.skipTest("Multiple failures with non-decimal inputs are not feasible: range check requires digits-only input.")


class TestAssertValidHostname(unittest.TestCase):
    HEX_MSG = "Must start with 12 or 16 uppercase hexadecimal characters [A-F0-9]"
    SUF_MSG = "Must be followed by a valid domain suffix (e.g., .local.)"

    # Valid values
    VALID_VALUES = [
        "B75AFB458ECD.local.",                # 12-hex + .local. with trailing dot
        "ABCDEF123456.local",                 # 12-hex + .local without trailing dot
        "ABCDEF1234567890.local.",            # 16-hex + .local.
        "1234567890ABCDEF.example",           # 16-hex + single-label domain
        "A1B2C3D4E5F60708.my-domain.local.",  # hyphenated domain labels allowed
        "ABCDEF123456.example.com",           # multi-label domain
    ]

    def test_valid_values(self):
        # All values in VALID_VALUES should pass without raising
        for value in self.VALID_VALUES:
            with self.subTest(value=value):
                assert_valid_hostname(value)

    def test_invalid_due_to_wrong_hex_length_short(self):
        # Hex prefix too short (11 chars) → hex constraint fails
        msg = fail_msg(assert_valid_hostname, "ABCDEF12345.local.")
        self.assertIn(self.HEX_MSG, msg)
        self.assertNotIn(self.SUF_MSG, msg)

    def test_invalid_due_to_wrong_hex_length_long(self):
        # Hex prefix too long (13 chars) → hex constraint fails
        msg = fail_msg(assert_valid_hostname, "ABCDEF1234567.local.")
        self.assertIn(self.HEX_MSG, msg)
        self.assertNotIn(self.SUF_MSG, msg)

    def test_invalid_due_to_lowercase_hex(self):
        # Lowercase in hex prefix → hex constraint fails
        msg = fail_msg(assert_valid_hostname, "abcdef123456.local.")
        self.assertIn(self.HEX_MSG, msg)
        self.assertNotIn(self.SUF_MSG, msg)

    def test_invalid_due_to_missing_domain_suffix(self):
        # No domain suffix (no dot after hex) → suffix constraint fails
        msg = fail_msg(assert_valid_hostname, "ABCDEF123456")
        self.assertNotIn(self.HEX_MSG, msg)  # hex part is valid length/content
        self.assertIn(self.SUF_MSG, msg)

    def test_invalid_due_to_suffix_not_starting_with_dot(self):
        # Characters after hex but no leading dot → suffix constraint fails
        msg = fail_msg(assert_valid_hostname, "ABCDEF123456local.")
        self.assertNotIn(self.HEX_MSG, msg)
        self.assertIn(self.SUF_MSG, msg)

    def test_invalid_due_to_bad_domain_characters(self):
        # Underscore not allowed in labels → suffix constraint fails
        msg = fail_msg(assert_valid_hostname, "ABCDEF123456.loc_al.")
        self.assertNotIn(self.HEX_MSG, msg)
        self.assertIn(self.SUF_MSG, msg)

    def test_invalid_due_to_hex_length_and_missing_suffix(self):
        # Too-short hex (11) and no dot at all -> both HEX and SUF should fail
        msg = fail_msg(assert_valid_hostname, "ABCDEF12345")
        self.assertIn(self.HEX_MSG, msg)
        self.assertIn(self.SUF_MSG, msg)

    def test_invalid_due_to_lowercase_hex_and_missing_suffix(self):
        # Lowercase hex and no dot -> both constraints fail
        msg = fail_msg(assert_valid_hostname, "abcdef123456")
        self.assertIn(self.HEX_MSG, msg)
        self.assertIn(self.SUF_MSG, msg)

    def test_invalid_due_to_hex_length_and_bad_domain_characters(self):
        # Too-short hex and bad domain label (underscore)
        msg = fail_msg(assert_valid_hostname, "ABCDEF12345._lo_cal.")
        self.assertIn(self.HEX_MSG, msg)
        self.assertIn(self.SUF_MSG, msg)

    def test_invalid_due_to_lowercase_hex_and_bad_domain_characters(self):
        # Lowercase hex and invalid domain label -> both constraints fail
        msg = fail_msg(assert_valid_hostname, "abcdef123456._bad_.")
        self.assertIn(self.HEX_MSG, msg)
        self.assertIn(self.SUF_MSG, msg)


class TestAssertValidIcdKey(unittest.TestCase):
    DEC_MSG = 'Encoded as a decimal number in ASCII text'
    LEAD_MSG = 'Omitting any leading zeros'
    VAL_MSG = 'Allowed values: 0 or 1'

    # Valid values
    VALID_VALUES = [
        "0",  # minimum allowed
        "1",  # maximum allowed
    ]

    def test_valid_values(self):
        # All valid values should pass
        for value in self.VALID_VALUES:
            with self.subTest(value=value):
                assert_valid_icd_key(value)

    def test_invalid_due_to_non_decimal(self):
        # Not a decimal
        msg = fail_msg(assert_valid_icd_key, "A")
        self.assertIn(self.DEC_MSG, msg)

    def test_invalid_due_to_leading_zero(self):
        # Leading zero not allowed except "0"
        msg = fail_msg(assert_valid_icd_key, "01")
        self.assertIn(self.LEAD_MSG, msg)

    def test_invalid_due_to_out_of_range(self):
        # Decimal but not 0 or 1
        msg = fail_msg(assert_valid_icd_key, "2")
        self.assertIn(self.VAL_MSG, msg)

    def test_invalid_due_to_empty_string(self):
        # Empty string
        msg = fail_msg(assert_valid_icd_key, "")
        self.assertIn(self.DEC_MSG, msg)

    def test_invalid_due_to_non_decimal_and_not_allowed(self):
        # Non-decimal implies not allowed set membership as well
        msg = fail_msg(assert_valid_icd_key, "1A")
        self.assertIn(self.DEC_MSG, msg)
        self.assertIn(self.VAL_MSG, msg)
        self.assertNotIn(self.LEAD_MSG, msg)

    def test_invalid_due_to_leading_zero_and_not_allowed(self):
        # Decimal with leading zero and not exactly "0" or "1"
        msg = fail_msg(assert_valid_icd_key, "02")
        self.assertIn(self.LEAD_MSG, msg)
        self.assertIn(self.VAL_MSG, msg)
        self.assertNotIn(self.DEC_MSG, msg)

    def test_invalid_due_to_multiple_leading_zeros_and_not_allowed(self):
        msg = fail_msg(assert_valid_icd_key, "0002")
        self.assertIn(self.LEAD_MSG, msg)
        self.assertIn(self.VAL_MSG, msg)
        self.assertNotIn(self.DEC_MSG, msg)

    def test_invalid_due_to_empty_string_reports_non_decimal_and_not_allowed(self):
        msg = fail_msg(assert_valid_icd_key, "")
        self.assertIn(self.DEC_MSG, msg)
        self.assertIn(self.VAL_MSG, msg)
        self.assertNotIn(self.LEAD_MSG, msg)

    def test_impossible_non_decimal_and_leading_zero_combination(self):
        # Leading-zero rule only applies to digit-only strings; cannot co-occur with non-decimal.
        self.skipTest("Multiple failures with DEC+LEAD are not feasible: leading-zero check requires digits-only input.")


class TestAssertValidIPv6Addresses(unittest.TestCase):
    def _fail_msg(self, values: list[str]) -> str:
        # Helper: run expecting failure and return the assertion message (catch both types)
        try:
            assert_valid_ipv6_addresses(values)
        except (AssertionError, signals.TestFailure) as e:
            return str(e)
        self.fail("Expected failure but assertion passed")
        return None

    def test_valid_single_address(self):
        # Single valid IPv6 address should pass
        assert_valid_ipv6_addresses(["2001:db8::1"])

    def test_valid_multiple_addresses(self):
        # Multiple valid IPv6 addresses should pass
        assert_valid_ipv6_addresses([
            "2001:db8::1",
            "fe80::1",
            "::1"
        ])

    def test_invalid_single_address(self):
        # One invalid IPv6 address should fail
        msg = self._fail_msg(["invalid"])
        self.assertIn("Invalid IPv6 addresses", msg)
        self.assertIn("invalid", msg)

    def test_invalid_mixed_addresses(self):
        # Mixed valid and invalid addresses should fail
        msg = self._fail_msg(["2001:db8::1", "bad_address", "fe80::1::1"])
        self.assertIn("Invalid IPv6 addresses", msg)
        self.assertIn("bad_address", msg)
        self.assertIn("fe80::1::1", msg)
        self.assertNotIn("2001:db8::1", msg)

    def test_invalid_empty_string(self):
        # Empty string should fail
        msg = self._fail_msg([""])
        self.assertIn("Invalid IPv6 addresses", msg)
        self.assertIn("''", msg)  # representation of empty string


class TestAssertValidJfKey(unittest.TestCase):
    DEC_MSG = "Must be a decimal integer without leading zeroes"
    MASK_MSG = "Only bits 0-3 may be set (value must fit in 4 bits)"
    B0_MSG = "Bit 0 cannot coexist with bits 1-3"
    B2_REQ_B1_MSG = "Bit 2 requires bit 1"
    B3_REQ_B12_MSG = "Bit 3 requires both bits 1 and 2"

    # Valid values
    VALID_VALUES = [
        "1",   # only bit 0
        "2",   # only bit 1
        "6",   # bits 1 and 2
        "14",  # bits 1,2,3
    ]

    def test_valid_values(self):
        # All values in VALID_VALUES should pass
        for value in self.VALID_VALUES:
            with self.subTest(value=value):
                assert_valid_jf_key(value)

    def test_invalid_due_to_non_decimal(self):
        # Not a decimal
        msg = fail_msg(assert_valid_jf_key, "A")
        self.assertIn(self.DEC_MSG, msg)

    def test_invalid_due_to_leading_zero(self):
        # Leading zero not allowed
        msg = fail_msg(assert_valid_jf_key, "01")
        self.assertIn(self.DEC_MSG, msg)

    def test_invalid_due_to_bits_above_3(self):
        # Value uses bit 4 (16)
        msg = fail_msg(assert_valid_jf_key, "16")
        self.assertIn(self.MASK_MSG, msg)
        self.assertNotIn(self.B0_MSG, msg)

    def test_invalid_due_to_bit0_with_others(self):
        # Bit 0 coexisting with bit 1
        msg = fail_msg(assert_valid_jf_key, "3")
        self.assertIn(self.B0_MSG, msg)
        self.assertNotIn(self.MASK_MSG, msg)

    def test_invalid_due_to_bit2_without_bit1(self):
        # Bit 2 requires bit 1
        msg = fail_msg(assert_valid_jf_key, "4")
        self.assertIn(self.B2_REQ_B1_MSG, msg)
        self.assertNotIn(self.B0_MSG, msg)

    def test_invalid_due_to_bit3_without_bits12(self):
        # Bit 3 requires both bits 1 and 2
        msg = fail_msg(assert_valid_jf_key, "8")
        self.assertIn(self.B3_REQ_B12_MSG, msg)
        self.assertNotIn(self.B0_MSG, msg)

    def test_invalid_due_to_empty_string(self):
        # Empty string invalid
        msg = fail_msg(assert_valid_jf_key, "")
        self.assertIn(self.DEC_MSG, msg)

    def test_invalid_due_to_negative(self):
        # Negative numbers invalid
        msg = fail_msg(assert_valid_jf_key, "-1")
        self.assertIn(self.DEC_MSG, msg)

    def test_invalid_due_to_bit0_and_bit3(self):
        # 9 (0b1001): bit0 with bit3; also bit3 lacks bits1&2.
        msg = fail_msg(assert_valid_jf_key, "9")
        self.assertIn(self.B0_MSG, msg)
        self.assertIn(self.B3_REQ_B12_MSG, msg)
        self.assertNotIn(self.MASK_MSG, msg)

    def test_invalid_due_to_bits2_and_3_without_bit1(self):
        # 12 (0b1100): bit3 set without bits1&2; bit2 set without bit1.
        msg = fail_msg(assert_valid_jf_key, "12")
        self.assertIn(self.B2_REQ_B1_MSG, msg)
        self.assertIn(self.B3_REQ_B12_MSG, msg)
        self.assertNotIn(self.MASK_MSG, msg)

    def test_invalid_due_to_mask_and_bit2_dependency(self):
        # 20 (0b10100): bit4 + bit2, lacking bit1.
        msg = fail_msg(assert_valid_jf_key, "20")
        self.assertIn(self.MASK_MSG, msg)
        self.assertIn(self.B2_REQ_B1_MSG, msg)
        self.assertNotIn(self.B0_MSG, msg)

    def test_invalid_due_to_mask_and_bit3_dependency(self):
        # 24 (0b11000): bit4 + bit3, lacking bits1&2.
        msg = fail_msg(assert_valid_jf_key, "24")
        self.assertIn(self.MASK_MSG, msg)
        self.assertIn(self.B3_REQ_B12_MSG, msg)
        self.assertNotIn(self.B0_MSG, msg)

    def test_invalid_due_to_mask_bit2_and_bit3_dependencies(self):
        # 28 (0b11100): bit4 + bit3 + bit2; missing bit1.
        msg = fail_msg(assert_valid_jf_key, "28")
        self.assertIn(self.MASK_MSG, msg)
        self.assertIn(self.B2_REQ_B1_MSG, msg)
        self.assertIn(self.B3_REQ_B12_MSG, msg)
        self.assertNotIn(self.B0_MSG, msg)

    def test_invalid_due_to_mask_and_bit0_with_others(self):
        # 19 (0b10011): bit4 + (bit0 and bit1) -> mask + bit0-with-others
        msg = fail_msg(assert_valid_jf_key, "19")
        self.assertIn(self.MASK_MSG, msg)
        self.assertIn(self.B0_MSG, msg)
        self.assertNotIn(self.B2_REQ_B1_MSG, msg)
        self.assertNotIn(self.B3_REQ_B12_MSG, msg)

    def test_non_decimal_cannot_combine_with_bit_rules(self):
        # Bit dependency/mask checks require a parsed integer.
        self.skipTest("Multiple failures with DEC are not feasible: bit checks only apply to valid decimal input.")


class TestAssertValidLongDiscriminatorSubtype(unittest.TestCase):
    FMT_MSG = "Must match format '_L<value>._sub.<commissionable-service-type>'"
    DEC_MSG = "Value must be a decimal integer without leading zeroes"
    RNG_MSG = "Value must be within 0-4095 (12-bit range)"

    # Valid values
    VALID_VALUES = [
        "_L0._sub._matterc._udp.local.",       # minimum value
        "_L1._sub._matterc._udp.local.",       # smallest nonzero
        "_L4095._sub._matterc._udp.local.",    # maximum valid 12-bit
        "_L123._sub._matterc._udp.local.",     # midrange
        "_L3840._sub._matterc._udp.local.",    # example from spec
    ]

    def test_valid_values(self):
        for value in self.VALID_VALUES:
            with self.subTest(value=value):
                assert_valid_long_discriminator_subtype(value)

    def test_invalid_due_to_wrong_format(self):
        msg = fail_msg(assert_valid_long_discriminator_subtype, "123._sub._matterc._udp.local.")
        self.assertIn(self.FMT_MSG, msg)

    def test_invalid_due_to_leading_zero(self):
        msg = fail_msg(assert_valid_long_discriminator_subtype, "_L0123._sub._matterc._udp.local.")
        self.assertIn(self.FMT_MSG, msg)   # format forbids leading zero
        self.assertIn(self.DEC_MSG, msg)   # decimal rule forbids leading zero
        self.assertNotIn(self.RNG_MSG, msg)

    def test_invalid_due_to_non_decimal_value(self):
        msg = fail_msg(assert_valid_long_discriminator_subtype, "_LABCD._sub._matterc._udp.local.")
        self.assertIn(self.FMT_MSG, msg)
        self.assertIn(self.DEC_MSG, msg)
        self.assertNotIn(self.RNG_MSG, msg)

    def test_invalid_due_to_value_out_of_range(self):
        msg = fail_msg(assert_valid_long_discriminator_subtype, "_L4096._sub._matterc._udp.local.")
        self.assertNotIn(self.FMT_MSG, msg)
        self.assertNotIn(self.DEC_MSG, msg)
        self.assertIn(self.RNG_MSG, msg)

    def test_invalid_due_to_missing_sub_suffix(self):
        msg = fail_msg(assert_valid_long_discriminator_subtype, "_L123._matterc._udp.local.")
        self.assertIn(self.FMT_MSG, msg)
        self.assertNotIn(self.DEC_MSG, msg)
        self.assertNotIn(self.RNG_MSG, msg)

    def test_invalid_due_to_format_and_range_missing_sub_but_big(self):
        # Missing '._sub', numeric and >4095 → FMT + RNG
        msg = fail_msg(assert_valid_long_discriminator_subtype, "_L5000._matterc._udp.local.")
        self.assertIn(self.FMT_MSG, msg)
        self.assertNotIn(self.DEC_MSG, msg)
        self.assertIn(self.RNG_MSG, msg)

    def test_invalid_due_to_format_and_decimal_rule_leading_zero(self):
        # Leading zero with otherwise valid pieces → FMT + DEC
        msg = fail_msg(assert_valid_long_discriminator_subtype, "_L01._sub._matterc._udp.local.")
        self.assertIn(self.FMT_MSG, msg)
        self.assertIn(self.DEC_MSG, msg)
        self.assertNotIn(self.RNG_MSG, msg)

    def test_invalid_due_to_format_decimal_rule_and_range(self):
        # Leading zero and >4095 and missing '._sub' → FMT + DEC + RNG
        msg = fail_msg(assert_valid_long_discriminator_subtype, "_L04096._matterc._udp.local.")
        self.assertIn(self.FMT_MSG, msg)
        self.assertIn(self.DEC_MSG, msg)
        self.assertIn(self.RNG_MSG, msg)

    def test_impossible_decimal_and_range_without_format(self):
        # DEC+RNG without FMT is not feasible because DEC (leading zero) contradicts the strict format.
        self.skipTest("Multiple failures DEC+RNG without FMT are not feasible: strict format forbids leading zeroes.")


class TestAssertValidOperationalInstanceName(unittest.TestCase):
    HY_MSG = 'Contains exactly one hyphen separating two parts'
    LEN_MSG = 'Each part is exactly 16 characters long'
    HEX_MSG = 'Each part only contains hexadecimal uppercase characters [A-F0-9]'

    # Valid values
    VALID_VALUES = [
        "0000000000000000-0000000000000000",   # all zeros
        "FFFFFFFFFFFFFFFF-FFFFFFFFFFFFFFFF",   # all hex letters
        "ABCDEF1234567890-0123456789ABCDEF",   # mixed hex letters and digits
        "A1B2C3D4E5F60708-90ABCDEF12345678",   # another mixed pattern
        "0000000000000001-0000000000000001",   # leading zeros still valid
        "1234567890ABCDEF-FEDCBA0987654321",   # reversed/varied
    ]

    def test_valid_values(self):
        # All values in VALID_VALUES should pass without raising
        for value in self.VALID_VALUES:
            with self.subTest(value=value):  # Subtest makes it clear which value failed if any
                assert_valid_operational_instance_name(value)

    def test_invalid_due_to_extra_trailing_hyphen(self):
        # Invalid: splits into 3 parts (extra '-') → only hyphen check should fail
        # Length/charset checks are gated off when hyphen count is wrong
        msg = fail_msg(assert_valid_operational_instance_name, "B7322C948581262F-00000000012344321-")
        self.assertIn(self.HY_MSG, msg)
        self.assertNotIn(self.LEN_MSG, msg)
        self.assertNotIn(self.HEX_MSG, msg)

    def test_invalid_due_to_missing_hyphen(self):
        # Invalid: no hyphen → only hyphen check should fail
        msg = fail_msg(assert_valid_operational_instance_name, "B7322C948581262F0000000012344321")
        self.assertIn(self.HY_MSG, msg)
        self.assertNotIn(self.LEN_MSG, msg)
        self.assertNotIn(self.HEX_MSG, msg)

    def test_invalid_due_to_wrong_length(self):
        # Invalid: first part is 15 chars instead of 16
        # Length check fails; charset is not evaluated when length fails
        msg = fail_msg(assert_valid_operational_instance_name, "B7322C948581262-0000000012344321")
        self.assertNotIn(self.HY_MSG, msg)
        self.assertIn(self.LEN_MSG, msg)
        self.assertNotIn(self.HEX_MSG, msg)

    def test_invalid_due_to_non_hex_characters(self):
        # Invalid: both parts are 16 chars but include non-hex characters
        # Only the hex charset check should fail (hyphen/length are OK)
        msg = fail_msg(assert_valid_operational_instance_name, "B7322C948581262G-000000001234432Z")
        self.assertNotIn(self.HY_MSG, msg)
        self.assertNotIn(self.LEN_MSG, msg)
        self.assertIn(self.HEX_MSG, msg)

    def test_invalid_due_to_lowercase_characters(self):
        # Invalid: lowercase letters are not allowed (must be A-F0-9 uppercase)
        # Hyphen and length are fine → only hex charset should fail
        msg = fail_msg(assert_valid_operational_instance_name, "b7322C948581262F-0000000012344321")
        self.assertIn(self.HEX_MSG, msg)
        self.assertNotIn(self.HY_MSG, msg)
        self.assertNotIn(self.LEN_MSG, msg)

    def test_invalid_due_to_empty_string(self):
        # Invalid: empty string → no hyphen present → only hyphen check should fail
        msg = fail_msg(assert_valid_operational_instance_name, "")
        self.assertIn(self.HY_MSG, msg)
        self.assertNotIn(self.LEN_MSG, msg)
        self.assertNotIn(self.HEX_MSG, msg)

    def test_invalid_due_to_wrong_length_in_second_part(self):
        # Invalid: second part has 15 chars → only length check should fail
        msg = fail_msg(assert_valid_operational_instance_name, "AAAAAAAAAAAAAAAA-000000001234432")
        self.assertNotIn(self.HY_MSG, msg)
        self.assertIn(self.LEN_MSG, msg)
        self.assertNotIn(self.HEX_MSG, msg)

    def test_invalid_due_to_length_and_hex_in_first_part(self):
        # First part too short (15) and contains lowercase -> LEN + HEX
        msg = fail_msg(assert_valid_operational_instance_name, "ABCDEF12345aBCD-0000000012344321")
        self.assertNotIn(self.HY_MSG, msg)
        self.assertIn(self.LEN_MSG, msg)
        self.assertIn(self.HEX_MSG, msg)

    def test_invalid_due_to_length_and_hex_in_second_part(self):
        # Second part too short (15) and contains lowercase -> LEN + HEX
        msg = fail_msg(assert_valid_operational_instance_name, "ABCDEF1234567890-0000000012344z1")
        self.assertNotIn(self.HY_MSG, msg)
        self.assertIn(self.LEN_MSG, msg)
        self.assertIn(self.HEX_MSG, msg)

    def test_multiple_failures_not_possible_with_hyphen_error(self):
        # When hyphen count is wrong, length/charset aren't evaluated; only HY applies.
        self.skipTest("Multiple failures involving the hyphen check are not possible due to gating on hyphen count.")


class TestAssertValidPhKey(unittest.TestCase):
    INT_MSG = "Must be a decimal integer without leading zeroes"
    GT0_MSG = "Value must be greater than 0"
    BIT_MSG = "Only bits 0-22 may be set"

    # Valid values
    VALID_VALUES = [
        "1",          # minimum allowed
        "33",         # typical example
        "8388607",    # all 22-bits enabled (1<<23 - 1)
        "524288",     # power of two within 23 bits
        "999999",     # large but still valid under mask
    ]

    def test_valid_values(self):
        # All values in VALID_VALUES should pass
        for value in self.VALID_VALUES:
            with self.subTest(value=value):
                assert_valid_ph_key(value)

    def test_invalid_due_to_leading_zero(self):
        # Leading zero not allowed
        msg = fail_msg(assert_valid_ph_key, "0123")
        self.assertIn(self.INT_MSG, msg)

    def test_invalid_due_to_non_decimal(self):
        # Non-decimal characters
        msg = fail_msg(assert_valid_ph_key, "12A3")
        self.assertIn(self.INT_MSG, msg)

    def test_invalid_due_to_zero_value(self):
        # Value must be > 0
        msg = fail_msg(assert_valid_ph_key, "0")
        self.assertIn(self.GT0_MSG, msg)

    def test_invalid_due_to_out_of_range(self):
        # Only bits 1 through 22 are defined.
        msg = fail_msg(assert_valid_ph_key, str((1 << 23)))
        self.assertIn(self.BIT_MSG, msg)

    def test_invalid_due_to_empty_string(self):
        # Empty string is invalid
        msg = fail_msg(assert_valid_ph_key, "")
        self.assertIn(self.INT_MSG, msg)

    def test_invalid_due_to_leading_zero_and_zero_value(self):
        # "00" is digits-only with leading zeros and equals 0 → INT + GT0
        msg = fail_msg(assert_valid_ph_key, "00")
        self.assertIn(self.INT_MSG, msg)
        self.assertIn(self.GT0_MSG, msg)
        self.assertNotIn(self.BIT_MSG, msg)

    def test_invalid_due_to_leading_zero_and_out_of_range(self):
        # Leading zero present and numeric value beyond range
        msg = fail_msg(assert_valid_ph_key, "08388608")  # 8,388,608 == 1 << 23
        self.assertIn(self.INT_MSG, msg)
        self.assertIn(self.BIT_MSG, msg)
        self.assertNotIn(self.GT0_MSG, msg)

    def test_non_decimal_cannot_combine_with_value_or_mask_checks(self):
        # Value (>0) and mask checks require digits-only parsing.
        self.skipTest("Multiple failures with non-decimal inputs are not feasible: >0 and bit-mask checks require numeric input.")

    def test_gt0_and_bit_cannot_fail_together(self):
        # If BIT fails, the value is necessarily > 0, so GT0 can't also fail.
        self.skipTest("Value >0 and bit-mask violations cannot co-occur: mask violation implies a positive value.")

    def test_invalid_due_to_many_leading_zeros_and_zero_value(self):
        # Multiple leading zeros with value 0 -> INT + GT0, no BIT
        msg = fail_msg(assert_valid_ph_key, "0000")
        self.assertIn(self.INT_MSG, msg)
        self.assertIn(self.GT0_MSG, msg)
        self.assertNotIn(self.BIT_MSG, msg)

    def test_invalid_due_to_leading_zero_and_out_of_range_variant(self):
        # Leading zero plus value beyond 22-bit mask -> INT + BIT, no GT0
        msg = fail_msg(assert_valid_ph_key, "0008388608")  # 1_048_577 > (1<<20) - 1
        self.assertIn(self.INT_MSG, msg)
        self.assertIn(self.BIT_MSG, msg)
        self.assertNotIn(self.GT0_MSG, msg)

    def test_all_three_constraints_cannot_fail_together(self):
        # GT0 fails only when value == 0; BIT fails only when value > (1<<20)-1.
        # These conditions are mutually exclusive, so INT+GT0+BIT cannot co-occur.
        self.skipTest("All-three failure impossible: GT0 (value==0) and BIT (value>1048575) are mutually exclusive.")

    def test_invalid_due_to_many_leading_zeros_and_zero_value_long(self):
        # A longer zero string should still report INT + GT0 and not BIT
        msg = fail_msg(assert_valid_ph_key, "0" * 10)
        self.assertIn(self.INT_MSG, msg)
        self.assertIn(self.GT0_MSG, msg)
        self.assertNotIn(self.BIT_MSG, msg)

    def test_invalid_due_to_leading_zeros_and_large_out_of_range(self):
        # Large out-of-range value with leading zeros -> INT + BIT, no GT0
        msg = fail_msg(assert_valid_ph_key, "0009999999")
        self.assertIn(self.INT_MSG, msg)
        self.assertIn(self.BIT_MSG, msg)
        self.assertNotIn(self.GT0_MSG, msg)


class TestAssertValidPiKey(unittest.TestCase):
    UTF8_MSG = "Must be a valid UTF-8 string"
    LEN_MSG = "UTF-8 encoded length must be ≤ 128 bytes"

    # Valid values
    VALID_VALUES = [
        "10",                              # simple ASCII
        "Pairing instructions",            # plain ASCII
        "Cámara",                          # accented UTF-8
        "设备",                             # UTF-8 multibyte, well under 128 bytes
        "a" * 128,                         # exactly 128 ASCII chars
    ]

    def test_valid_values(self):
        # All values in VALID_VALUES should pass
        for value in self.VALID_VALUES:
            with self.subTest(value=value):
                assert_valid_pi_key(value)

    def test_invalid_due_to_too_long_ascii(self):
        # 129 ASCII chars -> exceeds max length
        msg = fail_msg(assert_valid_pi_key, "a" * 129)
        self.assertIn(self.LEN_MSG, msg)
        self.assertNotIn(self.UTF8_MSG, msg)

    def test_invalid_due_to_too_long_utf8_multibyte(self):
        # '设备' is 3 bytes per char in UTF-8. 65 * 3 = 195 bytes (>128) -> exceeds limit.
        msg = fail_msg(assert_valid_pi_key, "设备" * 65)
        self.assertIn(self.LEN_MSG, msg)
        self.assertNotIn(self.UTF8_MSG, msg)

    def test_invalid_due_to_non_utf8(self):
        # Surrogate that cannot encode in UTF-8
        bad_string = "\udcff"
        msg = fail_msg(assert_valid_pi_key, bad_string)
        self.assertIn(self.UTF8_MSG, msg)

    def test_non_utf8_does_not_report_length(self):
        # When UTF-8 encoding fails, only the UTF8_MSG should appear; LEN_MSG must not.
        msg = fail_msg(assert_valid_pi_key, "\udcff")
        self.assertIn(self.UTF8_MSG, msg)
        self.assertNotIn(self.LEN_MSG, msg)

    def test_multibyte_exact_128_bytes_is_valid(self):
        # Use a true 2-byte UTF-8 character: 'é' (U+00E9) encodes to 0xC3 0xA9.
        # 64 * 2 bytes = 128 bytes exactly -> should be valid.
        value = "é" * 64
        assert_valid_pi_key(value)

    def test_non_utf8_even_when_long_source_string_reports_only_utf8_error(self):
        # Even if the Python string length is large, a non-UTF8 surrogate makes encoding fail first.
        # Expect only UTF8_MSG and no LEN_MSG.
        bad = "\udcff" * 200  # long source, but encoding fails before length is considered
        msg = fail_msg(assert_valid_pi_key, bad)
        self.assertIn(self.UTF8_MSG, msg)
        self.assertNotIn(self.LEN_MSG, msg)


class TestAssertValidPhPiRelationship(unittest.TestCase):
    def test_valid_ph_no_pi_needed(self):
        # PH has no mandatory PI bits set (e.g., bit 0, 1, 2, 3)
        # 7 = 0b0111
        assert_valid_ph_pi_relationship({'PH': '7'})

    def test_valid_ph_with_mandatory_pi_bits(self):
        # PH bit 4 is set, PI is present
        # 16 = 0b10000
        assert_valid_ph_pi_relationship({'PH': '16', 'PI': 'some instruction'})

    def test_invalid_ph_mandatory_pi_bit_set_no_pi(self):
        # PH bit 4 is set, but PI is missing
        msg = fail_msg(assert_valid_ph_pi_relationship, {'PH': '16'})
        self.assertIn("'PI' key must be present", msg)

    def test_invalid_pi_no_ph(self):
        # PI is present, but PH is missing
        msg = fail_msg(assert_valid_ph_pi_relationship, {'PI': 'some instruction'})
        self.assertIn("'PH' key must be present", msg)

    def test_invalid_pi_with_ph_missing_related_bits(self):
        # PI is present, PH is present but has none of bits 4, 8-12, 15-22 set
        # 7 = 0b0111
        msg = fail_msg(assert_valid_ph_pi_relationship, {'PH': '7', 'PI': 'some instruction'})
        self.assertIn("'PH' key must have at least one of bits", msg)

    def test_empty_txt(self):
        # No PH or PI, should pass
        assert_valid_ph_pi_relationship({})


class TestAssertValidProductId(unittest.TestCase):
    INT_MSG = "Must be a decimal integer without leading zeroes"
    RNG_MSG = "Value must be within 0-65535 (16-bit range)"

    # Valid values
    VALID_VALUES = [
        "0",       # minimum
        "1",       # smallest nonzero
        "456",     # typical
        "65535",   # maximum
        "12345",   # valid large value
    ]

    def test_valid_values(self):
        # All valid values should pass
        for value in self.VALID_VALUES:
            with self.subTest(value=value):
                assert_valid_product_id(value)

    def test_invalid_due_to_leading_zero(self):
        # Leading zeros not allowed (except "0")
        msg = fail_msg(assert_valid_product_id, "0456")
        self.assertIn(self.INT_MSG, msg)

    def test_invalid_due_to_non_decimal(self):
        # Non-decimal not allowed
        msg = fail_msg(assert_valid_product_id, "12A3")
        self.assertIn(self.INT_MSG, msg)

    def test_invalid_due_to_out_of_range(self):
        # Value above 16-bit max
        msg = fail_msg(assert_valid_product_id, "65536")
        self.assertIn(self.RNG_MSG, msg)

    def test_invalid_due_to_too_many_digits(self):
        # More than 5 digits not allowed
        msg = fail_msg(assert_valid_product_id, "123456")
        self.assertIn(self.INT_MSG, msg)

    def test_invalid_due_to_empty_string(self):
        # Empty string invalid
        msg = fail_msg(assert_valid_product_id, "")
        self.assertIn(self.INT_MSG, msg)

    def test_non_decimal_and_range_combination_not_possible(self):
        # Inputs with non-digits (e.g., '12A3') fail the syntactic check and skip range evaluation.
        # Thus RNG_MSG cannot appear together with INT_MSG for non-decimal inputs.
        self.skipTest("Non-decimal input cannot also trigger range failure (range check skipped).")

    def test_leading_zero_and_range_combination_not_possible(self):
        # Inputs like '0123' fail the syntactic check due to leading zero and skip range evaluation.
        # Thus RNG_MSG cannot appear together with INT_MSG in this scenario.
        self.skipTest("Leading-zero inputs skip range check; cannot produce both INT_MSG and RNG_MSG.")

    def test_too_many_digits_and_range_combination_not_possible(self):
        # Inputs with >5 digits (e.g., '123456') fail the syntactic check and skip range evaluation.
        # Thus RNG_MSG cannot appear together with INT_MSG here either.
        self.skipTest("Too many digits (>5) skip range check; cannot produce both INT_MSG and RNG_MSG.")


class TestAssertValidRiKey(unittest.TestCase):
    HEX_MSG = "Must only contain uppercase hexadecimal characters [A-F0-9]"
    LEN_MSG = "Length must be between 1 and 100 characters"

    # Valid values
    VALID_VALUES = [
        "A",  # minimum length, valid hex
        "0A1B2C3D",  # typical value
        "FFFFFFFFFFFFFFFF",  # repeated hex chars
        "1234567890ABCDEF",  # mixed digits and letters
        "ABCDEF" * 16 + "AB",  # exactly 100 chars
    ]

    def test_valid_values(self):
        # All values in VALID_VALUES should pass
        for value in self.VALID_VALUES:
            with self.subTest(value=value):
                assert_valid_ri_key(value)

    def test_invalid_due_to_non_hex_characters(self):
        # Non-hex character included
        msg = fail_msg(assert_valid_ri_key, "0A1B2C3G")
        self.assertIn(self.HEX_MSG, msg)
        self.assertNotIn(self.LEN_MSG, msg)

    def test_invalid_due_to_lowercase_characters(self):
        # Lowercase not allowed
        msg = fail_msg(assert_valid_ri_key, "abcDEF123")
        self.assertIn(self.HEX_MSG, msg)

    def test_invalid_due_to_too_long(self):
        # 101 chars
        msg = fail_msg(assert_valid_ri_key, "A" * 101)
        self.assertIn(self.LEN_MSG, msg)
        self.assertNotIn(self.HEX_MSG, msg)

    def test_invalid_due_to_empty_string(self):
        # Empty string fails charset
        msg = fail_msg(assert_valid_ri_key, "")
        self.assertIn(self.HEX_MSG, msg)

    def test_invalid_due_to_hex_and_length_empty(self):
        # Empty string violates both: not hex and length < 1
        msg = fail_msg(assert_valid_ri_key, "")
        self.assertIn(self.HEX_MSG, msg)
        self.assertIn(self.LEN_MSG, msg)

    def test_invalid_due_to_hex_and_length_over_100(self):
        # Length > 100 and contains a lowercase non-hex character
        bad = "A" * 100 + "g"  # 101 chars, includes invalid 'g'
        msg = fail_msg(assert_valid_ri_key, bad)
        self.assertIn(self.HEX_MSG, msg)
        self.assertIn(self.LEN_MSG, msg)

    def test_invalid_due_to_lowercase_only_and_over_100(self):
        # All lowercase hex characters (invalid charset) and length > 100
        bad = "a" * 101
        msg = fail_msg(assert_valid_ri_key, bad)
        self.assertIn(self.HEX_MSG, msg)
        self.assertIn(self.LEN_MSG, msg)

    def test_invalid_due_to_separator_and_over_100(self):
        # Includes a non-hex separator and total length > 100
        bad = ("A" * 50) + "-" + ("B" * 51)  # 102 chars total, '-' breaks hex charset
        msg = fail_msg(assert_valid_ri_key, bad)
        self.assertIn(self.HEX_MSG, msg)
        self.assertIn(self.LEN_MSG, msg)


class TestAssertValidSaiKey(unittest.TestCase):
    INT_MSG = "Must be a decimal integer without leading zeroes"
    RNG_MSG = "Value must be ≤ 3600000 (1 hour in milliseconds)"

    # Valid values
    VALID_VALUES = [
        "0",          # minimum
        "1",          # smallest nonzero
        "1250",       # example mid-range
        "3600000",    # maximum allowed
        "250000",     # large but valid
    ]

    def test_valid_values(self):
        # All valid values should pass
        for value in self.VALID_VALUES:
            with self.subTest(value=value):
                assert_valid_sai_key(value)

    def test_invalid_due_to_leading_zero(self):
        # Leading zero not allowed (except "0")
        msg = fail_msg(assert_valid_sai_key, "0123")
        self.assertIn(self.INT_MSG, msg)

    def test_invalid_due_to_non_decimal(self):
        # Non-decimal not allowed
        msg = fail_msg(assert_valid_sai_key, "12A3")
        self.assertIn(self.INT_MSG, msg)

    def test_invalid_due_to_out_of_range(self):
        # Value above maximum allowed
        msg = fail_msg(assert_valid_sai_key, "3600001")
        self.assertIn(self.RNG_MSG, msg)

    def test_invalid_due_to_empty_string(self):
        # Empty string invalid
        msg = fail_msg(assert_valid_sai_key, "")
        self.assertIn(self.INT_MSG, msg)

    def test_invalid_due_to_leading_zero_and_out_of_range(self):
        # Digits-only so range is evaluated; leading zero violates INT_MSG; numeric value exceeds max -> RNG_MSG
        msg = fail_msg(assert_valid_sai_key, "03600001")
        self.assertIn(self.INT_MSG, msg)
        self.assertIn(self.RNG_MSG, msg)

    def test_invalid_due_to_many_leading_zeros_and_out_of_range(self):
        # Another both-fail variant with multiple leading zeros and clearly > 3_600_000
        msg = fail_msg(assert_valid_sai_key, "0003600001")
        self.assertIn(self.INT_MSG, msg)
        self.assertIn(self.RNG_MSG, msg)

    def test_non_decimal_and_out_of_range_combination_not_possible(self):
        # Range check only runs for digits-only inputs (re.fullmatch(r'\d+')).
        # If the input contains non-digits (e.g., '12A3'), it's impossible to also trigger RNG_MSG.
        self.skipTest("Non-decimal input cannot simultaneously trigger range failure (digits-only required for range).")


class TestAssertValidSatKey(unittest.TestCase):
    INT_MSG = "Must be a decimal integer without leading zeroes"
    RNG_MSG = "Value must be ≤ 65535 (65.535 seconds)"

    # Valid values
    VALID_VALUES = [
        "0",        # minimum
        "1",        # smallest nonzero
        "1250",     # example mid-range
        "65535",    # maximum allowed
        "50000",    # large but valid
    ]

    def test_valid_values(self):
        # All valid values should pass
        for value in self.VALID_VALUES:
            with self.subTest(value=value):
                assert_valid_sat_key(value)

    def test_invalid_due_to_leading_zero(self):
        # Leading zero not allowed (except "0")
        msg = fail_msg(assert_valid_sat_key, "0123")
        self.assertIn(self.INT_MSG, msg)

    def test_invalid_due_to_non_decimal(self):
        # Non-decimal not allowed
        msg = fail_msg(assert_valid_sat_key, "12A3")
        self.assertIn(self.INT_MSG, msg)

    def test_invalid_due_to_out_of_range(self):
        # Value above maximum allowed
        msg = fail_msg(assert_valid_sat_key, "65536")
        self.assertIn(self.RNG_MSG, msg)

    def test_invalid_due_to_empty_string(self):
        # Empty string invalid
        msg = fail_msg(assert_valid_sat_key, "")
        self.assertIn(self.INT_MSG, msg)

    def test_invalid_due_to_leading_zero_and_out_of_range(self):
        # Digits-only so range is evaluated; leading zero violates INT_MSG; numeric value exceeds max -> RNG_MSG
        msg = fail_msg(assert_valid_sat_key, "065536")
        self.assertIn(self.INT_MSG, msg)
        self.assertIn(self.RNG_MSG, msg)

    def test_invalid_due_to_many_leading_zeros_and_out_of_range(self):
        # Multiple leading zeros and value clearly > 65535 -> both failures
        msg = fail_msg(assert_valid_sat_key, "00065536")
        self.assertIn(self.INT_MSG, msg)
        self.assertIn(self.RNG_MSG, msg)

    def test_non_decimal_and_out_of_range_combination_not_possible(self):
        # Range check only runs for digits-only inputs.
        # If the input contains non-digits (e.g., '12A3'), RNG_MSG cannot be triggered.
        self.skipTest("Non-decimal input cannot simultaneously trigger range failure (digits-only required for range).")


class TestAssertValidShortDiscriminatorSubtype(unittest.TestCase):
    FMT_MSG = "Must match format '_S<value>._sub.<commissionable-service-type>'"
    DEC_MSG = "Value must be a decimal integer without leading zeroes"
    RNG_MSG = "Value must be within 0-15 (4-bit range)"

    # Valid values
    VALID_VALUES = [
        "_S0._sub._matterc._udp.local.",    # minimum valid
        "_S1._sub._matterc._udp.local.",    # smallest nonzero
        "_S9._sub._matterc._udp.local.",    # single-digit
        "_S15._sub._matterc._udp.local.",   # maximum 4-bit
    ]

    def test_valid_values(self):
        # All valid values should pass without raising
        for value in self.VALID_VALUES:
            with self.subTest(value=value):
                assert_valid_short_discriminator_subtype(value)

    def test_invalid_due_to_wrong_format(self):
        # Invalid: missing '_S' prefix
        msg = fail_msg(assert_valid_short_discriminator_subtype, "5._sub._matterc._udp.local.")
        self.assertIn(self.FMT_MSG, msg)

    def test_invalid_due_to_leading_zero(self):
        # With the updated validator, the structure is valid, but the value has a leading zero.
        msg = fail_msg(assert_valid_short_discriminator_subtype, "_S05._sub._matterc._udp.local.")
        self.assertIn(self.DEC_MSG, msg)

    def test_invalid_due_to_non_decimal_value(self):
        # Structure is valid; value is non-decimal -> decimal/leading-zero constraint fails.
        msg = fail_msg(assert_valid_short_discriminator_subtype, "_SAB._sub._matterc._udp.local.")
        self.assertIn(self.DEC_MSG, msg)

    def test_invalid_due_to_value_out_of_range(self):
        # Invalid: 16 is outside the 0–15 range
        msg = fail_msg(assert_valid_short_discriminator_subtype, "_S16._sub._matterc._udp.local.")
        self.assertIn(self.RNG_MSG, msg)

    def test_invalid_due_to_missing_sub_suffix(self):
        # Invalid: missing '._sub.<service>'
        msg = fail_msg(assert_valid_short_discriminator_subtype, "_S5._matterc._udp.local.")
        self.assertIn(self.FMT_MSG, msg)

    def test_invalid_due_to_decimal_and_range(self):
        # Correct format; value has a leading zero and is out of range (16)
        msg = fail_msg(assert_valid_short_discriminator_subtype, "_S016._sub._matterc._udp.local.")
        self.assertIn(self.DEC_MSG, msg)
        self.assertIn(self.RNG_MSG, msg)
        self.assertNotIn(self.FMT_MSG, msg)

    def test_invalid_due_to_format_and_decimal(self):
        # Wrong format (missing '._sub.<service>') and decimal rule fails (no extractable value)
        msg = fail_msg(assert_valid_short_discriminator_subtype, "_S05._matterc._udp.local.")
        self.assertIn(self.FMT_MSG, msg)
        self.assertIn(self.DEC_MSG, msg)
        self.assertNotIn(self.RNG_MSG, msg)

    def test_invalid_due_to_format_and_range(self):
        # Wrong service -> format fails, but we can still extract a valid integer and check range.
        msg = fail_msg(assert_valid_short_discriminator_subtype, "_S16._sub._wrongservice")
        self.assertIn(self.FMT_MSG, msg)
        self.assertIn(self.RNG_MSG, msg)
        self.assertNotIn(self.DEC_MSG, msg)

    def test_invalid_due_to_format_decimal_and_range(self):
        # Wrong service (format), leading zero (decimal), and out of range (16)
        msg = fail_msg(assert_valid_short_discriminator_subtype, "_S016._sub._wrongservice")
        self.assertIn(self.FMT_MSG, msg)
        self.assertIn(self.DEC_MSG, msg)
        self.assertIn(self.RNG_MSG, msg)


class TestAssertValidSiiKey(unittest.TestCase):
    INT_MSG = "Must be a decimal integer without leading zeroes"
    RNG_MSG = "Value must be ≤ 3600000 (1 hour in milliseconds)"

    # Valid values
    VALID_VALUES = [
        "0",          # minimum
        "1",          # smallest nonzero
        "5300",       # example mid-range
        "3600000",    # maximum allowed
        "999999",     # under max
    ]

    def test_valid_values(self):
        # All valid values should pass
        for value in self.VALID_VALUES:
            with self.subTest(value=value):
                assert_valid_sii_key(value)

    def test_invalid_due_to_leading_zero(self):
        # Leading zero not allowed (except "0")
        msg = fail_msg(assert_valid_sii_key, "0123")
        self.assertIn(self.INT_MSG, msg)

    def test_invalid_due_to_non_decimal(self):
        # Non-decimal not allowed
        msg = fail_msg(assert_valid_sii_key, "12A3")
        self.assertIn(self.INT_MSG, msg)

    def test_invalid_due_to_out_of_range(self):
        # Value above maximum allowed
        msg = fail_msg(assert_valid_sii_key, "3600001")
        self.assertIn(self.RNG_MSG, msg)

    def test_invalid_due_to_empty_string(self):
        # Empty string invalid
        msg = fail_msg(assert_valid_sii_key, "")
        self.assertIn(self.INT_MSG, msg)

    def test_invalid_due_to_leading_zero_and_out_of_range(self):
        # Digits-only so range check runs; leading zero violates INT; numeric value exceeds max -> RNG
        msg = fail_msg(assert_valid_sii_key, "03600001")
        self.assertIn(self.INT_MSG, msg)
        self.assertIn(self.RNG_MSG, msg)

    def test_invalid_due_to_many_leading_zeros_and_out_of_range(self):
        # Multiple leading zeros and clearly > 3_600_000 -> both failures present
        msg = fail_msg(assert_valid_sii_key, "0003600001")
        self.assertIn(self.INT_MSG, msg)
        self.assertIn(self.RNG_MSG, msg)

    def test_non_decimal_and_out_of_range_combination_not_possible(self):
        # Range check only executes for digits-only inputs; with non-digits, RNG_MSG cannot be emitted.
        self.skipTest("Non-decimal input cannot simultaneously trigger range failure (digits-only required for range).")


class TestAssertValidTKey(unittest.TestCase):
    INT_MSG = "Must be a decimal integer without leading zeroes"
    RANGE_MSG = "Only bits 0-2 may be present (value must fit in 3 bits)"
    BIT0_MSG = "Bit 0 is reserved and must be 0"
    PROV_MSG = "Bits 1 and 2 are provisional and must not be set (strict mode)"

    # Valid values (strict mode)
    VALID_VALUES = [
        "0",   # allowed, all bits clear
    ]

    def _fail_msg(self, value: str, enforce_provisional: bool = True) -> str:
        # Helper: run expecting failure and return assertion message (catch both types)
        try:
            assert_valid_t_key(value, enforce_provisional=enforce_provisional)
        except (AssertionError, signals.TestFailure) as e:
            return str(e)
        self.fail("Expected failure but assertion passed")
        return None

    def test_valid_values_strict(self):
        # In strict mode, only "0" should pass
        for value in self.VALID_VALUES:
            with self.subTest(value=value):
                assert_valid_t_key(value)

    def test_invalid_due_to_non_decimal(self):
        msg = self._fail_msg("A")
        self.assertIn(self.INT_MSG, msg)
        self.assertNotIn(self.RANGE_MSG, msg)
        self.assertNotIn(self.BIT0_MSG, msg)
        self.assertNotIn(self.PROV_MSG, msg)

    def test_invalid_due_to_leading_zero(self):
        msg = self._fail_msg("01")
        self.assertIn(self.INT_MSG, msg)

    def test_invalid_due_to_out_of_range_bit3(self):
        msg = self._fail_msg("8")
        self.assertIn(self.RANGE_MSG, msg)
        self.assertNotIn(self.INT_MSG, msg)  # format is fine

    def test_invalid_due_to_bit0_set(self):
        msg = self._fail_msg("1")
        self.assertIn(self.BIT0_MSG, msg)
        self.assertNotIn(self.INT_MSG, msg)

    def test_invalid_due_to_provisional_bits_in_strict_mode(self):
        for val in ["2", "4", "6"]:
            with self.subTest(val=val):
                msg = self._fail_msg(val)
                self.assertIn(self.PROV_MSG, msg)

    def test_valid_with_provisional_bits_when_relaxed(self):
        assert_valid_t_key("2", enforce_provisional=False)
        assert_valid_t_key("4", enforce_provisional=False)
        assert_valid_t_key("6", enforce_provisional=False)

    def test_invalid_due_to_bit0_even_if_relaxed(self):
        msg = self._fail_msg("1", enforce_provisional=False)
        self.assertIn(self.BIT0_MSG, msg)
        self.assertNotIn(self.PROV_MSG, msg)

    def test_invalid_due_to_out_of_range_even_if_relaxed(self):
        msg = self._fail_msg("9", enforce_provisional=False)
        self.assertIn(self.RANGE_MSG, msg)
        self.assertNotIn(self.PROV_MSG, msg)

    def test_invalid_due_to_int_and_bit0_strict(self):
        # Leading zero breaks INT; numeric value 1 sets bit0
        msg = self._fail_msg("01")
        self.assertIn(self.INT_MSG, msg)
        self.assertIn(self.BIT0_MSG, msg)
        self.assertNotIn(self.RANGE_MSG, msg)
        self.assertNotIn(self.PROV_MSG, msg)

    def test_invalid_due_to_int_and_provisional_strict(self):
        # Leading zero + value 2 sets provisional bit1
        msg = self._fail_msg("02")
        self.assertIn(self.INT_MSG, msg)
        self.assertIn(self.PROV_MSG, msg)
        self.assertNotIn(self.RANGE_MSG, msg)
        self.assertNotIn(self.BIT0_MSG, msg)

    def test_invalid_due_to_int_range_and_provisional_strict(self):
        # "010" -> integer rule fails; 10 (0b1010) sets bit3 (range) and bit1 (provisional)
        msg = self._fail_msg("010")
        self.assertIn(self.INT_MSG, msg)
        self.assertIn(self.RANGE_MSG, msg)
        self.assertIn(self.PROV_MSG, msg)
        self.assertNotIn(self.BIT0_MSG, msg)

    def test_invalid_due_to_range_bit0_and_provisional_strict(self):
        # 11 (0b1011): bit3 -> range, bit0 set, bit1 -> provisional
        msg = self._fail_msg("11")
        self.assertIn(self.RANGE_MSG, msg)
        self.assertIn(self.BIT0_MSG, msg)
        self.assertIn(self.PROV_MSG, msg)
        self.assertNotIn(self.INT_MSG, msg)

    def test_invalid_due_to_bit0_and_provisional_strict(self):
        # 3 (0b0011): within 3-bit range; bit0 set & bit1 (provisional) set
        msg = self._fail_msg("3")
        self.assertIn(self.BIT0_MSG, msg)
        self.assertIn(self.PROV_MSG, msg)
        self.assertNotIn(self.RANGE_MSG, msg)
        self.assertNotIn(self.INT_MSG, msg)

    def test_invalid_due_to_range_and_bit0_relaxed(self):
        # Relaxed: provisional ignored; 9 (0b1001) -> range + bit0
        msg = self._fail_msg("9", enforce_provisional=False)
        self.assertIn(self.RANGE_MSG, msg)
        self.assertIn(self.BIT0_MSG, msg)
        self.assertNotIn(self.PROV_MSG, msg)
        self.assertNotIn(self.INT_MSG, msg)

    def test_invalid_due_to_int_range_and_bit0_relaxed(self):
        # Leading zero + 9 -> INT + RANGE + BIT0 (relaxed ignores provisional)
        msg = self._fail_msg("09", enforce_provisional=False)
        self.assertIn(self.INT_MSG, msg)
        self.assertIn(self.RANGE_MSG, msg)
        self.assertIn(self.BIT0_MSG, msg)
        self.assertNotIn(self.PROV_MSG, msg)


class TestAssertValidVendorId(unittest.TestCase):
    INT_MSG = "Must be a decimal integer without leading zeroes"
    RNG_MSG = "Value must be within 0-65535 (16-bit range)"

    # Valid values
    VALID_VALUES = [
        "0",       # minimum
        "1",       # smallest nonzero
        "123",     # typical
        "65535",   # maximum
        "54321",   # valid large value
    ]

    def test_valid_values(self):
        # All valid values should pass
        for value in self.VALID_VALUES:
            with self.subTest(value=value):
                assert_valid_vendor_id(value)

    def test_invalid_due_to_leading_zero(self):
        # Leading zeros not allowed (except "0")
        msg = fail_msg(assert_valid_vendor_id, "0123")
        self.assertIn(self.INT_MSG, msg)

    def test_invalid_due_to_non_decimal(self):
        # Non-decimal not allowed
        msg = fail_msg(assert_valid_vendor_id, "12A3")
        self.assertIn(self.INT_MSG, msg)

    def test_invalid_due_to_out_of_range(self):
        # Value above 16-bit max
        msg = fail_msg(assert_valid_vendor_id, "65536")
        self.assertIn(self.RNG_MSG, msg)

    def test_invalid_due_to_too_many_digits(self):
        # More than 5 digits not allowed
        msg = fail_msg(assert_valid_vendor_id, "123456")
        self.assertIn(self.INT_MSG, msg)

    def test_invalid_due_to_empty_string(self):
        # Empty string invalid
        msg = fail_msg(assert_valid_vendor_id, "")
        self.assertIn(self.INT_MSG, msg)

    def test_invalid_due_to_leading_zero_and_out_of_range(self):
        # Digits-only so range is evaluated; leading zero violates INT; value exceeds max -> RNG
        msg = fail_msg(assert_valid_vendor_id, "065536")
        self.assertIn(self.INT_MSG, msg)
        self.assertIn(self.RNG_MSG, msg)

    def test_invalid_due_to_many_leading_zeros_and_out_of_range(self):
        # Multiple leading zeros and clearly > 65535 -> both failures
        msg = fail_msg(assert_valid_vendor_id, "00065536")
        self.assertIn(self.INT_MSG, msg)
        self.assertIn(self.RNG_MSG, msg)

    def test_invalid_due_to_too_many_digits_and_out_of_range(self):
        # >5 digits violates INT; numeric value also exceeds 16-bit range -> both failures
        msg = fail_msg(assert_valid_vendor_id, "123456")
        self.assertIn(self.INT_MSG, msg)
        self.assertIn(self.RNG_MSG, msg)

    def test_non_decimal_and_out_of_range_combination_not_possible(self):
        # Range check only runs for digits-only inputs; with non-digits, RNG_MSG cannot be emitted.
        self.skipTest("Non-decimal input cannot simultaneously trigger range failure (digits-only required for range).")

    def test_invalid_due_to_too_many_digits_and_out_of_range_variant(self):
        # Another >5-digits case that also exceeds 16-bit range -> both failures
        msg = fail_msg(assert_valid_vendor_id, "700000")
        self.assertIn(self.INT_MSG, msg)
        self.assertIn(self.RNG_MSG, msg)

    def test_invalid_due_to_many_leading_zeros_and_massive_out_of_range(self):
        # Leading zeros plus a clearly out-of-range value -> both failures
        msg = fail_msg(assert_valid_vendor_id, "000000655360")
        self.assertIn(self.INT_MSG, msg)
        self.assertIn(self.RNG_MSG, msg)


class TestAssertValidVendorSubtype(unittest.TestCase):
    FMT_MSG = "Must match format '_V<value>._sub.<commissionable-service-type>'"
    DEC_MSG = "Value must be a decimal integer without leading zeroes"
    RNG_MSG = "Value must be within 0-65535 (16-bit range)"

    # Valid values
    VALID_VALUES = [
        "_V0._sub._matterc._udp.local.",        # minimum valid
        "_V1._sub._matterc._udp.local.",        # smallest nonzero
        "_V123._sub._matterc._udp.local.",      # typical value
        "_V65535._sub._matterc._udp.local.",    # maximum valid (16-bit)
    ]

    def test_valid_values(self):
        # All valid values should pass without raising
        for value in self.VALID_VALUES:
            with self.subTest(value=value):
                assert_valid_vendor_subtype(value)

    def test_invalid_due_to_wrong_format(self):
        # Invalid: missing '_V' prefix
        msg = fail_msg(assert_valid_vendor_subtype, "123._sub._matterc._udp.local.")
        self.assertIn(self.FMT_MSG, msg)

    def test_invalid_due_to_leading_zero(self):
        # Invalid: leading zero not allowed except for zero itself
        msg = fail_msg(assert_valid_vendor_subtype, "_V0123._sub._matterc._udp.local.")
        self.assertIn(self.FMT_MSG, msg)

    def test_invalid_due_to_non_decimal_value(self):
        # Invalid: contains letters instead of digits
        msg = fail_msg(assert_valid_vendor_subtype, "_V12AB._sub._matterc._udp.local.")
        self.assertIn(self.FMT_MSG, msg)

    def test_invalid_due_to_value_out_of_range(self):
        # Invalid: 65536 is outside the 16-bit range
        msg = fail_msg(assert_valid_vendor_subtype, "_V65536._sub._matterc._udp.local.")
        self.assertIn(self.RNG_MSG, msg)

    def test_invalid_due_to_missing_sub_suffix(self):
        # Invalid: missing '._sub.<service>'
        msg = fail_msg(assert_valid_vendor_subtype, "_V123._matterc._udp.local.")
        self.assertIn(self.FMT_MSG, msg)

    def test_invalid_due_to_format_and_decimal(self):
        # Leading zero should now produce both FMT (strict pattern) and DEC (value rule)
        msg = fail_msg(assert_valid_vendor_subtype, "_V0123._sub._matterc._udp.local.")
        self.assertIn(self.FMT_MSG, msg)
        self.assertIn(self.DEC_MSG, msg)
        self.assertNotIn(self.RNG_MSG, msg)

    def test_invalid_due_to_format_and_range(self):
        # Wrong service but numeric value is out of range
        msg = fail_msg(assert_valid_vendor_subtype, "_V70000._sub._wrongservice")
        self.assertIn(self.FMT_MSG, msg)
        self.assertIn(self.RNG_MSG, msg)
        self.assertNotIn(self.DEC_MSG, msg)

    def test_invalid_due_to_format_decimal_and_range(self):
        # Leading zero AND wrong service AND out of range -> all three
        msg = fail_msg(assert_valid_vendor_subtype, "_V065536._sub._wrongservice")
        self.assertIn(self.FMT_MSG, msg)
        self.assertIn(self.DEC_MSG, msg)
        self.assertIn(self.RNG_MSG, msg)

    def test_invalid_due_to_decimal_only_within_format(self):
        # Wrong value token (non-decimal) inside otherwise correct structure
        msg = fail_msg(assert_valid_vendor_subtype, "_V12AB._sub._matterc._udp.local.")
        self.assertIn(self.FMT_MSG, msg)  # strict pattern fails
        self.assertIn(self.DEC_MSG, msg)  # decimal rule also fails
        self.assertNotIn(self.RNG_MSG, msg)

    def test_invalid_due_to_range_only(self):
        # Correct format & decimal rules satisfied; only range should fail
        msg = fail_msg(assert_valid_vendor_subtype, "_V65536._sub._matterc._udp.local.")
        self.assertIn(self.RNG_MSG, msg)
        # We do not enforce absence of others here because the validator may include them,
        # but at minimum RNG must be present.


class TestAssertValidVpKey(unittest.TestCase):
    VID_MSG = "Must contain at least a Vendor ID"
    SEP_MSG = "If Product ID is present, there must be exactly one '+' separator"
    VEN_MSG = "Vendor ID must be a valid decimal integer"
    PROD_MSG = "Product ID (if present) must be a valid decimal integer"

    # Valid values
    VALID_VALUES = [
        "123",        # Vendor ID only
        "0",          # Vendor ID only, minimum value
        "65535",      # Vendor ID only, upper bound
        "123+456",    # Vendor + Product
        "1+65535",    # Vendor with max 16-bit product
        "42+42",      # Vendor and product same
    ]

    def test_valid_values(self):
        for value in self.VALID_VALUES:
            with self.subTest(value=value):
                assert_valid_vp_key(value)

    def test_invalid_due_to_multiple_separators(self):
        msg = fail_msg(assert_valid_vp_key, "123+456+789")
        self.assertIn(self.SEP_MSG, msg)

    def test_invalid_due_to_non_integer_vendor_id(self):
        msg = fail_msg(assert_valid_vp_key, "abc+123")
        self.assertIn(self.VEN_MSG, msg)
        self.assertNotIn(self.PROD_MSG, msg)

    def test_invalid_due_to_non_integer_product_id(self):
        msg = fail_msg(assert_valid_vp_key, "123+abc")
        self.assertIn(self.PROD_MSG, msg)

    def test_invalid_due_to_empty_vendor_id(self):
        msg = fail_msg(assert_valid_vp_key, "+123")
        self.assertIn(self.VEN_MSG, msg)

    def test_invalid_due_to_empty_product_id(self):
        msg = fail_msg(assert_valid_vp_key, "123+")
        self.assertIn(self.PROD_MSG, msg)

    def test_invalid_due_to_empty_string(self):
        msg = fail_msg(assert_valid_vp_key, "")
        self.assertIn(self.VEN_MSG, msg)

    def test_invalid_due_to_vendor_and_product_non_decimal(self):
        msg = fail_msg(assert_valid_vp_key, "abc+def")
        self.assertIn(self.VEN_MSG, msg)
        self.assertIn(self.PROD_MSG, msg)

    def test_invalid_due_to_vendor_and_product_with_leading_zeros(self):
        msg = fail_msg(assert_valid_vp_key, "0123+0456")
        self.assertIn(self.VEN_MSG, msg)
        self.assertIn(self.PROD_MSG, msg)

    def test_invalid_due_to_vendor_and_product_out_of_range(self):
        msg = fail_msg(assert_valid_vp_key, "65536+65536")
        self.assertIn(self.VEN_MSG, msg)
        self.assertIn(self.PROD_MSG, msg)

    def test_invalid_due_to_missing_vendor_and_missing_product(self):
        msg = fail_msg(assert_valid_vp_key, "+")
        self.assertIn(self.VEN_MSG, msg)
        self.assertIn(self.PROD_MSG, msg)

    def test_invalid_due_to_vendor_non_decimal_and_product_out_of_range(self):
        msg = fail_msg(assert_valid_vp_key, "abc+65536")
        self.assertIn(self.VEN_MSG, msg)
        self.assertIn(self.PROD_MSG, msg)


if __name__ == "__main__":
    unittest.main()
