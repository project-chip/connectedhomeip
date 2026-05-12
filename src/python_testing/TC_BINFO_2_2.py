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

'''
Purpose
Verify that the DUT generates the StartUp event on boot with the correct SoftwareVersion,
and, if supported, generates the Leave event with the correct fabric index when a fabric
is removed.

Test Plan
https://github.com/CHIP-Specifications/chip-test-plans/blob/master/src/cluster/basic_information.adoc#tc-binfo-2-2

'''

import logging

from mobly import asserts

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

cluster = Clusters.BasicInformation
attributes = cluster.Attributes
events = cluster.Events


class TC_BINFO_2_2(MatterBaseTest):

    def teardown_class(self):
        if hasattr(self, 'TH2') and self.TH2 is not None:
            self.TH2.Shutdown()
            self.TH2 = None

        super().teardown_class()

    def desc_TC_BINFO_2_2(self) -> str:
        return "[TC-BINFO-2.2] Events [DUT-Server]"

    def steps_TC_BINFO_2_2(self) -> list[TestStep]:
        return [
            TestStep("precondition", "Commissioning, already done.", is_commissioning=True),
            TestStep(1, "TH reads from the DUT the SoftwareVersion attribute.",
                     "Store value for later use."),
            TestStep(2, "TH reads the StartUp event from the DUT", """
                        If no StartUp event is present, reboot DUT and read StartUp event again.
                        - Verify that the StartUp event is present.
                        - Verify that the softwareVersion field from the StartUp event matches the SoftwareVersion attribute value read in step 1."""),
            TestStep(3, """Setup a second controller TH2 on its own fabric.
                           TH1 opens an Enhanced Commissioning Window on the DUT.
                           TH2 commissions the DUT on a new fabric.""",
                        "TH2 successfully commissions the DUT on a new fabric."),
            TestStep(4, "TH2 reads CurrentFabricIndex.", "Store value as th2_fabric_index."),
            TestStep(5, """TH1 sends RemoveFabric command with th2_fabric_index and reads the Leave event from the DUT.""", """
                        - If the Leave event is present: verify that the fabricIndex field matches th2_fabric_index, and verify that BINFO.S.E02 is declared in PICS.
                        - If the Leave event is absent: verify that BINFO.S.E02 is not declared in PICS."""),
        ]

    def pics_TC_BINFO_2_2(self) -> list[str]:
        return ["BINFO.S"]

    @async_test_body
    async def test_TC_BINFO_2_2(self):
        self.TH1 = self.default_controller

        # *** PRECONDITION ***
        # Commissioning, already done.
        self.step("precondition")

        # *** STEP 1 ***
        # TH reads from the DUT the SoftwareVersion attribute.
        # Store value for later use.
        self.step(1)
        software_version_from_attribute = await self.read_single_attribute_check_success(
            cluster=cluster,
            attribute=attributes.SoftwareVersion,
            endpoint=0
        )
        log.info(f"SoftwareVersion: {software_version_from_attribute}")

        # *** STEP 2 ***
        # TH reads the StartUp event from the DUT.
        # If no StartUp event is present, DUT is rebooted and StartUp event is read again.
        self.step(2)
        # Read StartUp event
        startup_events = await self.TH1.ReadEvent(
            nodeId=self.dut_node_id,
            events=[(0, events.StartUp, 0)]
        )
        log.info(f"StartUp events found: {len(startup_events)}")

        if not startup_events:
            # Reboot DUT
            log.info("StartUp events not found, DUT reboot is necessary.")
            await self.request_device_reboot()

            # Read StartUp event again after reboot
            startup_events = await self.TH1.ReadEvent(
                nodeId=self.dut_node_id,
                events=[(0, events.StartUp, 0)]
            )
            log.info(f"StartUp events found after reboot: {len(startup_events)}")

        # Verify that the StartUp event is present
        asserts.assert_true(startup_events, "StartUp event not present before or after reboot.")

        # Verify that the softwareVersion field from the StartUp event matches the SoftwareVersion attribute value read in step 1.
        software_version_from_startup_event = startup_events[-1].Data.softwareVersion
        asserts.assert_equal(software_version_from_startup_event, software_version_from_attribute,
                             f"StartUp event softwareVersion {software_version_from_startup_event} does not match SoftwareVersion attribute {software_version_from_attribute}")

        # *** STEP 3 ***
        # Setup a second controller TH2 on its own fabric.
        # TH1 opens an Enhanced Commissioning Window on the DUT.
        # TH2 commissions the DUT on a new fabric.
        self.step(3)

        th2_ca = self.certificate_authority_manager.NewCertificateAuthority()
        th2_fabric_admin = th2_ca.NewFabricAdmin(vendorId=0xFFF1, fabricId=self.TH1.fabricId + 1)
        self.TH2 = th2_fabric_admin.NewController(nodeId=2, useTestCommissioner=True)

        ecw = await self.open_commissioning_window(dev_ctrl=self.TH1, node_id=self.dut_node_id)

        th2_dut_node_id = self.dut_node_id + 1
        await self.TH2.CommissionOnNetwork(
            nodeId=th2_dut_node_id,
            setupPinCode=ecw.commissioningParameters.setupPinCode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=ecw.randomDiscriminator
        )

        # *** STEP 4 ***
        # TH2 reads CurrentFabricIndex.
        self.step(4)
        th2_fabric_index = await self.read_single_attribute_check_success(
            dev_ctrl=self.TH2,
            node_id=th2_dut_node_id,
            cluster=Clusters.OperationalCredentials,
            attribute=Clusters.OperationalCredentials.Attributes.CurrentFabricIndex,
            endpoint=0
        )

        # *** STEP 5 ***
        # TH1 sends RemoveFabric command with th2_fabric_index
        # and reads the Leave event from the DUT.
        self.step(5)

        # Send RemoveFabric command with th2_fabric_index
        await self.send_single_cmd(
            cmd=Clusters.OperationalCredentials.Commands.RemoveFabric(fabricIndex=th2_fabric_index),
            endpoint=0
        )

        # Read Leave event
        leave_events = await self.TH1.ReadEvent(
            nodeId=self.dut_node_id,
            events=[(0, events.Leave, 0)]
        )

        if leave_events:
            leave_event_fabric_index = leave_events[-1].Data.fabricIndex

            # Verify that the fabricIndex field from the Leave event matches th2_fabric_index
            asserts.assert_equal(leave_event_fabric_index, th2_fabric_index,
                                 f"Leave event fabricIndex {leave_event_fabric_index} does not match removed fabric index {th2_fabric_index}")

            # Verify that BINFO.S.E02 is declared in PICS
            asserts.assert_true(self.check_pics("BINFO.S.E02"),
                                "DUT sent a Leave event but BINFO.S.E02 is not declared in PICS")
        else:
            # If no Leave event is present, verify that BINFO.S.E02 is not declared in PICS
            asserts.assert_false(self.check_pics("BINFO.S.E02"),
                                 "BINFO.S.E02 is declared in PICS but no Leave event was observed after RemoveFabric")


if __name__ == "__main__":
    default_matter_test_main()
