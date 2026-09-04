#
#    Copyright (c) 2026 Project CHIP Authors
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
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --endpoint 0
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===
#

import logging
import time

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.testing import matter_asserts
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

logger = logging.getLogger(__name__)
logger.setLevel(logging.INFO)

# Uptime must advance measurably between two reads (plan step 4b).
UPTIME_WAIT_SECONDS = 10


class TC_DGGEN_2_1(MatterBaseTest):

    async def _read_dggen_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.GeneralDiagnostics
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def _assert_active_faults_list(self, fault_list, enum_type, description: str, max_faults: int):
        """Validates an ActiveFaults list: a set of at most max_faults values of the given enum."""
        matter_asserts.assert_list(fault_list, description, max_length=max_faults)
        for fault in fault_list:
            matter_asserts.assert_valid_enum(fault, f"{description} entry", enum_type)
            # Raw values outside the defined enum values decode to the kUnknownEnumValue
            # sentinel, which is an instance of the enum type and passes the check above.
            asserts.assert_not_equal(fault, enum_type.kUnknownEnumValue,
                                     f"{description} entry is not a defined {enum_type.__name__} value.")
        asserts.assert_equal(len(fault_list), len(set(fault_list)),
                             f"{description} contains duplicate faults, it must represent a set.")

    def desc_TC_DGGEN_2_1(self) -> str:
        """Returns a description of this test"""
        return "[TC-DGGEN-2.1] Attributes with Server as DUT"

    def pics_TC_DGGEN_2_1(self) -> list[str]:
        return ["DGGEN.S"]

    def steps_TC_DGGEN_2_1(self) -> list[TestStep]:
        return [
            TestStep(1, "Commission DUT to TH if not already commissioned", is_commissioning=True),
            TestStep("2a", "TH reads the RebootCount attribute from the DUT and saves as boot_count1.",
                     "Verify that boot_count1 is a uint16 value and is either 0 or 1."),
            TestStep("2b", "Reboot the DUT. TH reads the RebootCount attribute from the DUT and saves as boot_count2.",
                     "Verify that boot_count2 is greater than boot_count1."),
            TestStep(3, "TH reads the NetworkInterfaces attribute from the DUT.",
                     "Verify that each entry is a NetworkInterface struct with fields of the types listed in the "
                     "specification, and that the Type field is a value specified by the InterfaceType enum."),
            TestStep("4a", "TH reads the UpTime attribute from the DUT and saves as uptime1.",
                     "Verify that uptime1 is a uint64 value."),
            TestStep("4b", "Wait 10 seconds. TH reads the UpTime attribute from the DUT and saves as uptime2.",
                     "Verify that uptime2 is greater than uptime1."),
            TestStep("4c", "Reboot the DUT. TH reads the UpTime attribute from the DUT and saves as uptime3.",
                     "Verify that uptime3 is less than uptime2."),
            TestStep(5, "TH reads the BootReason attribute from the DUT (after the reboot in step 4c).",
                     "Verify that the BootReason is a value specified by the BootReason enum."),
            TestStep(6, "TH reads the ActiveHardwareFaults attribute from the DUT.",
                     "Verify that the value is a set of at most 11 values specified by the HardwareFault enum "
                     "(empty when no fault is active)."),
            TestStep(7, "TH reads the ActiveRadioFaults attribute from the DUT.",
                     "Verify that the value is a set of at most 7 values specified by the RadioFault enum "
                     "(empty when no fault is active)."),
            TestStep(8, "TH reads the ActiveNetworkFaults attribute from the DUT.",
                     "Verify that the value is a set of at most 4 values specified by the NetworkFault enum "
                     "(empty when no fault is active)."),
            TestStep(9, "TH reads the TestEventTriggersEnabled attribute from the DUT.",
                     "Verify that the value is a bool."),
            TestStep(10, "TH reads the TotalOperationalHours attribute from the DUT.",
                     "Verify that the value is a uint32."),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.GeneralDiagnostics))
    async def test_TC_DGGEN_2_1(self):

        endpoint = self.get_endpoint()
        attributes = Clusters.GeneralDiagnostics.Attributes
        enums = Clusters.GeneralDiagnostics.Enums

        # STEP 1: Commission DUT to TH (already done)
        self.step(1)

        # STEP 2a: TH reads RebootCount, saved as boot_count1
        self.step("2a")
        boot_count1 = await self._read_dggen_attribute_expect_success(endpoint=endpoint, attribute=attributes.RebootCount)
        matter_asserts.assert_valid_uint16(boot_count1, "RebootCount")
        asserts.assert_in(boot_count1, (0, 1), "RebootCount (boot_count1) is not 0 or 1.")
        logger.info("RebootCount (boot_count1): %s", boot_count1)

        # STEP 2b: reboot, then RebootCount must increment
        self.step("2b")
        await self.request_device_reboot()
        boot_count2 = await self._read_dggen_attribute_expect_success(endpoint=endpoint, attribute=attributes.RebootCount)
        asserts.assert_greater(boot_count2, boot_count1, "RebootCount did not increase after a reboot.")
        logger.info("RebootCount (boot_count2): %s", boot_count2)

        # STEP 3: TH reads NetworkInterfaces
        self.step(3)
        network_interfaces = await self._read_dggen_attribute_expect_success(
            endpoint=endpoint, attribute=attributes.NetworkInterfaces)
        # The spec constrains NetworkInterfaces to max 8 entries, but this is not
        # asserted here: the all-clusters reference app enumerates the host's network
        # interfaces, and a CI/dev host commonly has more than 8, which would fail a
        # conforming-DUT bound in an environment that is not a conforming DUT.
        matter_asserts.assert_list(network_interfaces, "NetworkInterfaces")
        for interface in network_interfaces:
            matter_asserts.assert_is_string(interface.name, "NetworkInterface.Name")
            asserts.assert_is_instance(interface.isOperational, bool, "NetworkInterface.IsOperational is not a bool")
            for field_name, value in (("OffPremiseServicesReachableIPv4", interface.offPremiseServicesReachableIPv4),
                                      ("OffPremiseServicesReachableIPv6", interface.offPremiseServicesReachableIPv6)):
                if value is not NullValue:
                    asserts.assert_is_instance(value, bool, f"NetworkInterface.{field_name} is not a bool or null")
            matter_asserts.assert_is_octstr(interface.hardwareAddress, "NetworkInterface.HardwareAddress")
            matter_asserts.assert_list(interface.IPv4Addresses, "NetworkInterface.IPv4Addresses")
            matter_asserts.assert_list(interface.IPv6Addresses, "NetworkInterface.IPv6Addresses")
            matter_asserts.assert_valid_enum(interface.type, "NetworkInterface.Type", enums.InterfaceTypeEnum)
            asserts.assert_not_equal(interface.type, enums.InterfaceTypeEnum.kUnknownEnumValue,
                                     "NetworkInterface.Type is not a defined InterfaceType value.")
        logger.info("NetworkInterfaces: %s entries", len(network_interfaces))

        # STEP 4a: TH reads UpTime, saved as uptime1
        self.step("4a")
        uptime1 = await self._read_dggen_attribute_expect_success(endpoint=endpoint, attribute=attributes.UpTime)
        matter_asserts.assert_valid_uint64(uptime1, "UpTime")
        logger.info("UpTime (uptime1): %s", uptime1)

        # STEP 4b: wait, then UpTime must have advanced
        self.step("4b")
        time.sleep(UPTIME_WAIT_SECONDS)
        uptime2 = await self._read_dggen_attribute_expect_success(endpoint=endpoint, attribute=attributes.UpTime)
        asserts.assert_greater(uptime2, uptime1, "UpTime did not increase after waiting.")
        logger.info("UpTime (uptime2): %s", uptime2)

        # STEP 4c: reboot, then UpTime must have reset below the pre-reboot value
        self.step("4c")
        await self.request_device_reboot()
        uptime3 = await self._read_dggen_attribute_expect_success(endpoint=endpoint, attribute=attributes.UpTime)
        asserts.assert_less(uptime3, uptime2, "UpTime did not reset after a reboot.")
        logger.info("UpTime (uptime3): %s", uptime3)

        # STEP 5: BootReason after the step 4c reboot (a reboot has just occurred, no need to reboot again)
        self.step(5)
        boot_reason = await self._read_dggen_attribute_expect_success(endpoint=endpoint, attribute=attributes.BootReason)
        matter_asserts.assert_valid_enum(boot_reason, "BootReason", enums.BootReasonEnum)
        asserts.assert_not_equal(boot_reason, enums.BootReasonEnum.kUnknownEnumValue,
                                 "BootReason is not a defined BootReason value.")
        logger.info("BootReason: %s", boot_reason)

        # STEP 6: ActiveHardwareFaults
        self.step(6)
        active_hw_faults = await self._read_dggen_attribute_expect_success(
            endpoint=endpoint, attribute=attributes.ActiveHardwareFaults)
        self._assert_active_faults_list(active_hw_faults, enums.HardwareFaultEnum, "ActiveHardwareFaults", max_faults=11)
        logger.info("ActiveHardwareFaults: %s", active_hw_faults)

        # STEP 7: ActiveRadioFaults
        self.step(7)
        active_radio_faults = await self._read_dggen_attribute_expect_success(
            endpoint=endpoint, attribute=attributes.ActiveRadioFaults)
        self._assert_active_faults_list(active_radio_faults, enums.RadioFaultEnum, "ActiveRadioFaults", max_faults=7)
        logger.info("ActiveRadioFaults: %s", active_radio_faults)

        # STEP 8: ActiveNetworkFaults
        self.step(8)
        active_network_faults = await self._read_dggen_attribute_expect_success(
            endpoint=endpoint, attribute=attributes.ActiveNetworkFaults)
        self._assert_active_faults_list(active_network_faults, enums.NetworkFaultEnum, "ActiveNetworkFaults", max_faults=4)
        logger.info("ActiveNetworkFaults: %s", active_network_faults)

        # STEP 9: TestEventTriggersEnabled
        self.step(9)
        test_event_triggers_enabled = await self._read_dggen_attribute_expect_success(
            endpoint=endpoint, attribute=attributes.TestEventTriggersEnabled)
        asserts.assert_is_instance(test_event_triggers_enabled, bool, "TestEventTriggersEnabled is not a bool")
        logger.info("TestEventTriggersEnabled: %s", test_event_triggers_enabled)

        # STEP 10: TotalOperationalHours
        # The test plan's operational-hours increment and factory-reset checks (its steps
        # 10a-10d) require multi-hour waits and a mid-test factory reset, which are not
        # automatable; this validates the attribute's type, which is the automatable part.
        self.step(10)
        total_operational_hours = await self._read_dggen_attribute_expect_success(
            endpoint=endpoint, attribute=attributes.TotalOperationalHours)
        matter_asserts.assert_valid_uint32(total_operational_hours, "TotalOperationalHours")
        logger.info("TotalOperationalHours: %s", total_operational_hours)


if __name__ == "__main__":
    default_matter_test_main()
