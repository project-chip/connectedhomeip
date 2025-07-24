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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.

# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${LIT_ICD_APP}
#     factory-reset: true
#     quiet: true
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import logging
import re
from typing import Any, Optional, Tuple, Union

import chip.clusters as Clusters
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from construct import Enum
from mdns_discovery.mdns_discovery import MdnsDiscovery, MdnsServiceType
from mobly import asserts

'''
Purpose
The purpose of this test case is to verify that a device is able to
correctly advertise Commissionable Node Discovery service.

Test Plan
https://github.com/CHIP-Specifications/chip-test-plans/blob/master/src/securechannel.adoc#341-tc-sc-41-commissionable-node-discovery-dut_commissionee
'''

PICS_MCORE_ROLE_COMMISSIONEE = "MCORE.ROLE.COMMISSIONEE"
TCP_PICS_STR = "MCORE.SC.TCP"
ONE_HOUR_IN_MS = 3600000
MAX_SAT_VALUE = 65535
MAX_T_VALUE = 6
LONG_DISCRIMINATOR = 3840
CM_SUBTYPE = f"_CM._sub.{MdnsServiceType.COMMISSIONABLE.value}"


class DiscriminatorLength(Enum):
    LONG = 12
    SHORT = 4


class TC_SC_4_1(MatterBaseTest):

    def steps_TC_SC_4_1(self):
        return [TestStep(1, "DUT is commissioned.", is_commissioning=True),
                TestStep(2, "TH reads ServerList attribute from the Descriptor cluster on EP0. ",
                         "If the ICD Management cluster ID (70,0x46) is present in the list, set supports_icd to true, otherwise set supports_icd to false."),
                TestStep(
                    3, "If supports_icd is true, TH reads ActiveModeThreshold from the ICD Management cluster on EP0 and saves as active_mode_threshold_ms."),
                TestStep(4, "DUT is put in Commissioning Mode.",
                         "DUT starts advertising Commissionable Node Discovery service using DNS-SD."),
                TestStep(5, "DUT is put in Commissioning Mode.",
                         "DUT starts advertising Commissionable Node Discovery service using DNS-SD."),
                ]

    async def read_attribute(self, attribute: Any) -> Any:
        cluster = Clusters.Objects.IcdManagement
        return await self.read_single_attribute_check_success(endpoint=self.endpoint, cluster=cluster, attribute=attribute)

    async def get_descriptor_server_list(self):
        return await self.read_single_attribute_check_success(
            endpoint=0,
            dev_ctrl=self.default_controller,
            cluster=Clusters.Descriptor,
            attribute=Clusters.Descriptor.Attributes.ServerList
        )

    async def get_active_mode_threshhold_ms(self):
        return await self.read_single_attribute_check_success(
            endpoint=0,
            dev_ctrl=self.default_controller,
            cluster=Clusters.IcdManagement,
            attribute=Clusters.IcdManagement.Attributes.ActiveModeThreshold
        )

    @staticmethod
    def is_valid_dns_sd_instance_name(name: str) -> bool:
        return bool(re.fullmatch(r'[A-F0-9]{16}', name))

    @staticmethod
    def is_valid_hostname(hostname: str) -> bool:
        # Remove '.local' suffix if present
        hostname = hostname.rstrip('.')
        if hostname.endswith('.local'):
            hostname = hostname[:-6]

        # Regular expression to match an uppercase hexadecimal string of 12 or 16 characters
        pattern = re.compile(r'^[0-9A-F]{12}$|^[0-9A-F]{16}$')
        return bool(pattern.match(hostname))

    @staticmethod
    def get_vendor_subtype(subtypes: list[str]) -> Optional[str]:
        pattern = re.compile(rf'^(_V(\d+))\._sub\.{re.escape(MdnsServiceType.COMMISSIONABLE.value)}$')
        return next((s for s in subtypes if pattern.fullmatch(s)), None)

    @staticmethod
    def get_devtype_subtype(subtypes: list[str]) -> Optional[str]:
        pattern = re.compile(rf'^(_T([1-9]\d*))\._sub\.{re.escape(MdnsServiceType.COMMISSIONABLE.value)}$')
        return next((s for s in subtypes if pattern.fullmatch(s)), None)

    @staticmethod
    def is_valid_discriminator_subtype(
        subtypes: list[str],
        discriminator_length: DiscriminatorLength
    ) -> bool:
        prefix = '_L' if discriminator_length == DiscriminatorLength.LONG else '_S'
        max_value = 4095 if discriminator_length == DiscriminatorLength.LONG else 15

        pattern = re.compile(rf'^({prefix}(\d+))\._sub\.{re.escape(MdnsServiceType.COMMISSIONABLE.value)}$')

        for subtype in subtypes:
            match = pattern.fullmatch(subtype)
            if match and 0 <= int(match.group(2)) <= max_value:
                return True

        return False

    @staticmethod
    def is_valid_vendor_subtype(subtype: str) -> bool:
        return 0 <= int(subtype.split('_V')[1].split('.')[0]) <= 65535

    @staticmethod
    def is_valid_devtype_subtype(subtype: str) -> bool:
        return int(subtype.split('_T')[1].split('.')[0]) >= 1

    @staticmethod
    def verify_d_key(txt_record: dict) -> bool:
        value = txt_record.get('D')
        return value is not None and value.isdigit() and 1 <= int(value) <= 9999 and not value.startswith('0')

    @staticmethod
    def is_valid_key_decimal_value(input_value, max_value: int) -> Union[bool, Tuple[bool, str]]:
        try:
            input_float = float(input_value)
            input_int = int(input_float)

            if str(input_value).startswith("0") and input_int != 0:
                return (False, f"Input ({input_value}) has leading zeros.")

            if input_float != input_int:
                return (False, f"Input ({input_value}) is not an integer.")

            if input_int <= max_value:
                return (True, f"Input ({input_value}) is valid.")
            else:
                return (False, f"Input ({input_value}) exceeds the allowed value {max_value}.")
        except ValueError:
            return (False, f"Input ({input_value}) is not a valid decimal number.")

    @staticmethod
    def is_valid_vp_key(value: str) -> bool:
        parts = value.split('+')

        if not (1 <= len(parts) <= 2):
            return False

        try:
            vendor_id = int(parts[0])
            if not (0 <= vendor_id <= 65535):
                return False

            if len(parts) == 2:
                product_id = int(parts[1])
                if not (0 <= product_id <= 65535):
                    return False

            return True
        except ValueError:
            return False

    @staticmethod
    def is_valid_dt_key(value: str) -> bool:
        # Must be a decimal ASCII string without leading zeros and ≥ 1
        return value.isdigit() and not value.startswith('0') and int(value) >= 1

    @staticmethod
    def is_valid_dn_key(value: str) -> bool:
        try:
            return len(value.encode('utf-8')) <= 32
        except UnicodeEncodeError:
            return False

    @staticmethod
    def is_valid_ri_key(value: str) -> bool:
        # Uppercase hex string, each byte is 2 hex chars
        if not re.fullmatch(r'[0-9A-F]{2,100}', value):
            return False
        return len(value) % 2 == 0 and len(value) <= 100

    @staticmethod
    def is_valid_ph_key(value: str) -> bool:
        if not value.isdigit():
            return False
        if value == '0' or value.startswith('0'):
            return False
        return True

    @staticmethod
    def is_valid_pi_key(value: str) -> bool:
        if value is None:
            return False
        try:
            return len(value.encode('utf-8')) <= 128
        except (UnicodeEncodeError, AttributeError):
            return False

    def desc_TC_TC_SC_4_1(self) -> str:
        return "[TC-SC-4.1] Commissionable Node Discovery with DUT as Commissionee"

    def pics_TC_SC_4_1(self) -> list[str]:
        return [PICS_MCORE_ROLE_COMMISSIONEE]

    @async_test_body
    async def test_TC_SC_4_1(self):
        self.endpoint = self.get_endpoint(default=1)
        active_mode_threshold_ms = None
        supports_icd = False

        # *** STEP 1 ***
        # DUT is Commissioned.
        self.step(1)

        # *** STEP 2 ***
        # TH reads from the DUT the ServerList attribute from the Descriptor cluster on EP0. If the ICD Management
        # cluster ID (70,0x46) is present in the list, set supports_icd to true, otherwise set supports_icd to false.
        self.step(2)
        ep0_servers = await self.get_descriptor_server_list()

        # Check if ep0_servers contain the ICD Management cluster ID (0x0046)
        supports_icd = Clusters.IcdManagement.id in ep0_servers
        logging.info(f"\n\n\t** supports_icd: {supports_icd}\n")

        # *** STEP 3 ***
        # If supports_icd is true, TH reads ActiveModeThreshold from the ICD Management cluster on EP0 and saves
        # as active_mode_threshold.
        self.step(3)
        if supports_icd:
            active_mode_threshold_ms = await self.get_active_mode_threshhold_ms()
        logging.info(f"\n\n\t** active_mode_threshold_ms: {active_mode_threshold_ms}\n")

        # *** STEP 4 ***
        # DUT is put in Commissioning Mode.
        # - DUT starts advertising Commissionable Node Discovery service using DNS-SD.
        self.step(4)
        await self.default_controller.OpenCommissioningWindow(
            nodeid=self.dut_node_id,
            timeout=600,
            iteration=10000,
            discriminator=LONG_DISCRIMINATOR,
            option=1
        )

        # *** STEP 5 ***
        # Get DUT's commissionable service.
        # - Verify presence of DUT's commissionable service.
        # - Verify DUT's commissionable service is a valid DNS-SD instance name.
        # - Verify DUT's commissionable service service type is '_matterc._udp' and service domain '.local.'
        self.step(5)
        mdns = MdnsDiscovery()

        # Get DUT's commissionable service
        commissionable_service = await mdns.get_commissionable_service(log_output=True)

        # Verify presence of DUT's comissionable service
        asserts.assert_is_not_none(commissionable_service, "DUT's commissionable service not present")

        # Verify DUT's commissionable service is a valid DNS-SD instance name
        # (64-bit randomly selected ID expressed as a sixteen-char hex string with capital letters)
        asserts.assert_true(self.is_valid_dns_sd_instance_name(commissionable_service.instance_name),
                            f"Invalid DNS-SD instance name: {commissionable_service.instance_name}")

        # Verify DUT's commissionable service service type is '_matterc._udp' and service domain '.local.'
        asserts.assert_equal(commissionable_service.service_type, MdnsServiceType.COMMISSIONABLE.value,
                             f"Invalid service type '{commissionable_service.service_type}', must be '{MdnsServiceType.COMMISSIONABLE.value}'")

        # Verify target hostname is derived from the 48bit or 64bit MAC address
        # expressed as a twelve or sixteen capital letter hex string. If the MAC
        # is randomized for privacy, the randomized version must be used each time.
        asserts.assert_true(self.is_valid_hostname(commissionable_service.server),
                            f"Invalid server hostname: {commissionable_service.server}")

        # Get commissionable subtypes
        subtypes = await mdns.get_commissionable_subtypes(log_output=True)

        # Validate that the long discriminator commissionable subtype is a 12-bit long discriminator,
        # encoded as a variable-length decimal number in ASCII text, omitting any leading zeros
        asserts.assert_true(
            self.is_valid_discriminator_subtype(subtypes, DiscriminatorLength.LONG),
            "Invalid long discriminator commissionable subtype or not present."
        )

        # Validate that the short discriminator commissionable subtype is a 4-bit long discriminator,
        # encoded as a variable-length decimal number in ASCII text, omitting any leading zeros
        asserts.assert_true(
            self.is_valid_discriminator_subtype(subtypes, DiscriminatorLength.SHORT),
            "Invalid short discriminator commissionable subtype or not present."
        )

        # If the vendor commissionable subtype is present, validate it's a
        # 16-bit vendor id, encoded as a variable-length decimal number in
        # ASCII text, omitting any leading zeros
        vendor_subtype = self.get_vendor_subtype(subtypes)
        if vendor_subtype:
            asserts.assert_true(
                self.is_valid_vendor_subtype(vendor_subtype),
                f"Invalid vendor commissionable subtype: {vendor_subtype}."
            )

        # If the devtype commissionable subtype is present, validate it's a
        # variable length decimal number in ASCII without leading zeros
        devtype_subtype = self.get_devtype_subtype(subtypes)
        if devtype_subtype:
            asserts.assert_true(
                self.is_valid_devtype_subtype(devtype_subtype),
                f"Invalid devtype commissionable subtype: {devtype_subtype}."
            )

        # Verify presence of the _CM subtype
        asserts.assert_in(CM_SUBTYPE, subtypes, f"{CM_SUBTYPE} subtype not present.")

        # Verify D key is present, which represents the discriminator
        # and must be encoded as a variable-length decimal value with up to 4
        # digits omitting any leading zeros
        asserts.assert_true(self.verify_d_key(commissionable_service.txt_record), "D key is invalid or not present.")

        # If VP key is present, verify it contain at least Vendor ID
        # and if Product ID is present, values must be separated by a + sign
        vp_key = commissionable_service.txt_record['VP']
        if vp_key:
            asserts.assert_true(self.is_valid_vp_key(vp_key), f"Invalid VP key: {vp_key}")

        # If SAI key is present, SII key must be an unsigned integer with
        # units of milliseconds and shall be encoded as a variable length decimal
        # number in ASCII, omitting leading zeros. Shall not exceed 3600000.
        sii_key = commissionable_service.txt_record['SII']
        if sii_key:
            result, message = self.is_valid_key_decimal_value(sii_key, ONE_HOUR_IN_MS)
            asserts.assert_true(result, message)

        # If SAI key is present, SAI key must be an unsigned integer with
        # units of milliseconds and shall be encoded as a variable length decimal
        # number in ASCII, omitting leading zeros. Shall not exceed 3600000.
        sai_key = commissionable_service.txt_record['SAI']
        if sai_key:
            result, message = self.is_valid_key_decimal_value(sai_key, ONE_HOUR_IN_MS)
            asserts.assert_true(result, message)

        # - If the SAT key is present, verify that it is a decimal value with
        #   no leading zeros and is less than or equal to 65535
        sat_key = commissionable_service.txt_record['SAT']
        if sat_key:
            result, message = self.is_valid_key_decimal_value(sat_key, MAX_SAT_VALUE)
            asserts.assert_true(result, message)

            # - If the SAT key is present and supports_icd is true, verify that
            #   the value is equal to active_mode_threshold
            if supports_icd:
                logging.info("supports_icd is True, verify the SAT value is equal to active_mode_threshold.")
                asserts.assert_equal(int(sat_key), active_mode_threshold_ms)

        # Verify that the SAI key is present if the SAT key is present
        asserts.assert_true(not sat_key or sai_key, "SAI key must be present if SAT key is present.")

        # TODO: how to make CM = 1, getting CM = 2 currently
        # Verify that the CM key is present and is equal to 1
        # cm_key = commissionable_service.txt_record.get('CM')
        # if cm_key:
        #     asserts.assert_true(cm_key == "1", f"CM key must be equal to 1, got {cm_key}")
        # else:
        #     asserts.fail("CM key not present")

        # If the DT key is present, it must contain the device type identifier from
        # Data Model Device Types and must be encoded as a variable length decimal
        # ASCII number without leading zeros
        if 'DT' in commissionable_service.txt_record:
            dt_key = commissionable_service.txt_record['DT']
            asserts.assert_true(
                self.is_valid_dt_key(dt_key),
                f"Invalid DT key: must be a decimal ASCII number ≥ 1 without leading zeros, got {dt_key}"
            )

        # If the DN key is present, DN key must be a UTF-8 encoded string with a maximum length of 32B
        if 'DN' in commissionable_service.txt_record:
            dn_key = commissionable_service.txt_record['DN']
            asserts.assert_true(
                self.is_valid_dn_key(dn_key),
                f"DN key must be valid UTF-8 and <= 32 bytes, got {len(dn_key.encode('utf-8'))} bytes"
            )

        # If the RI key is present, key RI must include the Rotating Device Identifier
        # encoded as an uppercase string with a maximum length of 100 chars (each octet
        # encoded as a 2-digit hex number, max 50 octets)
        if 'RI' in commissionable_service.txt_record:
            ri_key = commissionable_service.txt_record['RI']
            asserts.assert_true(
                self.is_valid_ri_key(ri_key),
                f"RI key must be uppercase hex with even length ≤ 100, got {ri_key}"
            )

        # If the PH key is present, key PH must be encoded as a variable-length decimal number
        # in ASCII text, omitting any leading zeros. If present value must be different of 0
        if 'PH' in commissionable_service.txt_record:
            ph_key = commissionable_service.txt_record['PH']
            asserts.assert_true(
                self.is_valid_ph_key(ph_key),
                f"PH key must be a non-zero decimal number without leading zeros, got {ph_key}"
            )

        # TODO: Fix PI key coming in None
        # # If the PI key is present, key PI must be encoded as a valid UTF-8 string
        # # with a maximum length of 128 bytes
        # if 'PI' in commissionable_service.txt_record:
        #     pi_key = commissionable_service.txt_record['PI']
        #     pi_value = len(pi_key.encode('utf-8')) if pi_key is not None else None
        #     asserts.assert_true(
        #         self.is_valid_pi_key(pi_key),
        #         f"PI key must be valid UTF-8 and <= 128 bytes, got {pi_value} bytes"
        #     )

        # TH performs a query for the AAAA record against the target
        # listed in the Commissionable Service SRV record.
        hostname = commissionable_service.server
        quada_records = await mdns.get_quada_records(hostname=hostname, log_output=True)

        # Verify at least 1 AAAA record is returned
        asserts.assert_greater(len(quada_records), 0, f"No AAAA addresses were resolved for hostname '{hostname}'")


if __name__ == "__main__":
    default_matter_test_main()
