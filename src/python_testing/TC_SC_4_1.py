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

from asyncio import sleep
import ipaddress
import logging
import re
from typing import Any

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

class DiscriminatorLength(Enum):
    LONG = 12
    SHORT = 4

class TC_SC_4_1(MatterBaseTest):

    def steps_TC_SC_4_1(self):
        return [TestStep(1, "DUT is commissioned.", is_commissioning=True),
                TestStep(2, "TH reads ServerList attribute from the Descriptor cluster on EP0. ",
                         "If the ICD Management cluster ID (70,0x46) is present in the list, set supports_icd to true, otherwise set supports_icd to false."),
                TestStep(3, "If supports_icd is true, TH reads ActiveModeThreshold from the ICD Management cluster on EP0 and saves as active_mode_threshold_ms."),
                TestStep(4, "DUT is put in Commissioning Mode.", "DUT starts advertising Commissionable Node Discovery service using DNS-SD."),
                TestStep(5, "DUT is put in Commissioning Mode.", "DUT starts advertising Commissionable Node Discovery service using DNS-SD."),
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
        """
        Validates that the DNS-SD instance name is a 64-bit randomly selected ID,
        expressed as a 16-character hexadecimal string using capital letters.

        Args:
            name (str): The DNS-SD instance name to validate.

        Returns:
            bool: True if valid, False otherwise.
        """
        return bool(re.fullmatch(r'[A-F0-9]{16}', name))

    @staticmethod
    def is_valid_hostname(hostname: str) -> bool:
        """ Verifies that the hostname is derived from a 48-bit or 64-bit MAC address,
        expressed as a 12-character or 16-character uppercase hexadecimal string.
        Args:
            hostname (str): The hostname to verify.
        Returns:
            bool: True if the hostname is valid, False otherwise.
        """
        # Remove '.local' suffix if present
        hostname = hostname.rstrip('.')
        if hostname.endswith('.local'):
            hostname = hostname[:-6]

        # Regular expression to match an uppercase hexadecimal string of 12 or 16 characters
        pattern = re.compile(r'^[0-9A-F]{12}$|^[0-9A-F]{16}$')
        return bool(pattern.match(hostname))

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
    def is_valid_vendor_subtype(subtypes: list[str]) -> bool:
        pattern = re.compile(rf'^(_V(\d+))\._sub\.{re.escape(MdnsServiceType.COMMISSIONABLE.value)}$')

        for subtype in subtypes:
            match = pattern.fullmatch(subtype)
            if match and 0 <= int(match.group(2)) <= 65535:
                return True

        return False

    def desc_TC_TC_SC_4_1(self) -> str:
        return "[TC-SC-4.1] Commissionable Node Discovery with DUT as Commissionee"

    def pics_TC_SC_4_1(self) -> list[str]:
        return [PICS_MCORE_ROLE_COMMISSIONEE]

    @async_test_body
    async def test_TC_SC_4_1(self):
        cluster = Clusters.Objects.IcdManagement
        attr = cluster.Attributes
        long_discriminator = 3840
        short_discriminator = 15
        self.endpoint = self.get_endpoint(default=1)
        self.endpoint = 1
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
            discriminator=long_discriminator,
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
        commissionable_service = await mdns.get_commissionable_service(discovery_timeout_sec=3, log_output=True)

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
        asserts.assert_true(self.is_valid_hostname(commissionable_service.server), f"Invalid server hostname: {commissionable_service.server}")

        # Get commissionable subtypes
        sub_types = await mdns.get_commissionable_subtypes(
            log_output=True,
        )

        # Validate that the long discriminator commissionable subtype is a 12-bit long discriminator,
        # encoded as a variable-length decimal number in ASCII text, omitting any leading zeros
        asserts.assert_true(
            self.is_valid_discriminator_subtype(sub_types, DiscriminatorLength.LONG),
            "Invalid long discriminator commissionable subtype or not present."
        )

        # Validate that the short discriminator commissionable subtype is a 4-bit long discriminator,
        # encoded as a variable-length decimal number in ASCII text, omitting any leading zeros
        asserts.assert_true(
            self.is_valid_discriminator_subtype(sub_types, DiscriminatorLength.SHORT),
            "Invalid short discriminator commissionable subtype or not present."
        )

        # Validate that the vendor commissionable subtype is a 16-bit vendor id, encoded
        # as a variable-length decimal number in ASCII text, omitting any leading zeros
        asserts.assert_true(
            self.is_valid_vendor_subtype(sub_types),
            "Invalid vendor commissionable subtype or not present."
        )

if __name__ == "__main__":
    default_matter_test_main()
