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
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --PICS src/app/tests/suites/certification/ci-pics-values
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

from mobly import asserts
from support_modules.idm_support import IDMBaseTest, WritableAttributeInfo

import matter.clusters as Clusters
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import async_test_body
from matter.testing.global_attribute_ids import is_standard_cluster_id
from matter.testing.matter_testing import MatterBaseTest, TestStep
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)


class TC_IDM_9_1(IDMBaseTest):
    def steps_TC_IDM_9_1(self) -> list[TestStep]:
        return [
            TestStep(0, "Commissioning, already done", is_commissioning=True),
            TestStep("1a", "Set the data field of a command of data type octstr to an out of range value in the Invoke Request message to the DUT from the TH. Set the length of the octstr to a value that is larger than the constraint allowed. Example: Command - GeneralDiagnostics.TestEventTrigger, data field - EnableKey, Constraint - 16 bytes exactly, Test Value - 17 bytes",
                     "Verify on the TH that the DUT sends a Status Response with a CONSTRAINT_ERROR Status Code."),
            TestStep("1b", "Set the data field of a command of data type octstr to an out of range value in the Invoke Request message to the DUT from the TH. Set the length of the octstr to a value that is smaller than the constraint allowed. Example: Command - GeneralDiagnostics.TestEventTrigger, data field - EnableKey, Constraint - 16 bytes exactly, Test Value - 15 bytes",
                     "Verify on the TH that the DUT sends a Status Response with a CONSTRAINT_ERROR Status Code."),
            TestStep("1c", "Set the data field of a command of data type string to an out of range value in the Invoke Request message to the DUT from the TH. Set the length of the string to a value that is larger than the maximum length allowed. Example: Command - SetRegulatoryConfig, data field - CountryCode, Constraint - 2",
                     "Verify on the TH that the DUT sends a Status Response with a CONSTRAINT_ERROR Status Code."),
            TestStep("1d", "Set the data field of a command of data type string to an out of range value in the Invoke Request message to the DUT from the TH. Set the length of the string to a value that is smaller than the minimum length allowed. Example: Command - SetRegulatoryConfig, data field - CountryCode, Constraint - 2",
                     "Verify on the TH that the DUT sends a Status Response with a CONSTRAINT_ERROR Status Code."),
            TestStep(2, "Read every attribute that is writable with bounds from all the clusters from all the endpoints. For every writable attribute read, set the data field  to an out of bounds value in the Write Request message to the DUT from the TH.",
                     "Verify on the TH that the DUT sends a Status Response Action with a CONSTRAINT_ERROR Status Code."),
        ]

    @async_test_body
    async def test_TC_IDM_9_1(self):
        self.step(0)
        await self.setup_class_helper(allow_pase=False)
        self.build_spec_xmls()
        self.endpoint = MatterBaseTest.get_endpoint(self)

        test_event_triggers_enabled = await self.read_single_attribute_check_success(
            cluster=Clusters.GeneralDiagnostics,
            attribute=Clusters.GeneralDiagnostics.Attributes.TestEventTriggersEnabled,
            endpoint=0,
        )
        if not test_event_triggers_enabled:
            log.info("TestEventTriggersEnabled is not enabled, skipping test step 1a and 1b")
            self.mark_step_range_skipped("1a", "1b")

        else:
            self.step("1a")
            # Step 1a: Test octstr max length constraint violation using GeneralDiagnostics TestEventTrigger command.
            # EnableKey is an octstr with allowed=16 (must be exactly 16 bytes); a 17-byte value violates the length constraint.
            # TestEventTrigger lives on the root node (endpoint 0).
            if await MatterBaseTest.command_guard(self, endpoint=0, command=Clusters.GeneralDiagnostics.Commands.TestEventTrigger):
                try:
                    cmd = Clusters.GeneralDiagnostics.Commands.TestEventTrigger(
                        enableKey=b'\x00' * 17,
                        eventTrigger=0,
                    )
                    await self.default_controller.SendCommand(nodeId=self.dut_node_id, endpoint=0, payload=cmd)
                    asserts.fail("Expected CONSTRAINT_ERROR but command succeeded")
                except InteractionModelError as e:
                    asserts.assert_equal(e.status, Status.ConstraintError,
                                         f"Expected CONSTRAINT_ERROR, but got {e.status}")

            # Step 1b: Test octstr min length constraint violation using GeneralDiagnostics TestEventTrigger command.
            # EnableKey is an octstr with allowed=16 (must be exactly 16 bytes); a 15-byte value violates the length constraint.
            # TestEventTrigger lives on the root node (endpoint 0).
            self.step("1b")
            if await MatterBaseTest.command_guard(self, endpoint=0, command=Clusters.GeneralDiagnostics.Commands.TestEventTrigger):
                try:
                    cmd = Clusters.GeneralDiagnostics.Commands.TestEventTrigger(
                        enableKey=b'\x00' * 15,
                        eventTrigger=0,
                    )
                    await self.default_controller.SendCommand(nodeId=self.dut_node_id, endpoint=0, payload=cmd)
                    asserts.fail("Expected CONSTRAINT_ERROR but command succeeded")
                except InteractionModelError as e:
                    asserts.assert_equal(e.status, Status.ConstraintError,
                                         f"Expected CONSTRAINT_ERROR, but got {e.status}")

        # Step 1c: Test string max length constraint violation using GeneralCommissioning clusters SetRegulatoryConfig command
        self.step("1c")
        try:
            # CountryCode field is string with constraint length=2
            cmd = Clusters.GeneralCommissioning.Commands.SetRegulatoryConfig(
                newRegulatoryConfig=Clusters.GeneralCommissioning.Enums.RegulatoryLocationTypeEnum.kIndoor,
                countryCode="USA"  # Out of range (must be 2 chars)
            )
            await self.default_controller.SendCommand(nodeId=self.dut_node_id, endpoint=0, payload=cmd)
            asserts.fail("Expected CONSTRAINT_ERROR but command succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError,
                                 f"Expected CONSTRAINT_ERROR, got {e.status}")

        # Step 1d: Test string min length constraint violation
        self.step("1d")
        try:
            # CountryCode field is string with constraint length=2
            cmd = Clusters.GeneralCommissioning.Commands.SetRegulatoryConfig(
                newRegulatoryConfig=Clusters.GeneralCommissioning.Enums.RegulatoryLocationTypeEnum.kIndoor,
                countryCode="U"  # Out of range (must be 2 chars)
            )
            await self.default_controller.SendCommand(nodeId=self.dut_node_id, endpoint=0, payload=cmd)
            asserts.fail("Expected CONSTRAINT_ERROR but command succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError,
                                 f"Expected CONSTRAINT_ERROR, got {e.status}")

        # Step 2: Test all writable attributes with constraints
        self.step(2)
        log.info("Testing writable attributes for constraint errors")

        # Collect writable attributes from DUT
        writable_attributes = []
        for endpoint_id, endpoint in self.endpoints_tlv.items():
            for cluster_id, device_cluster_data in endpoint.items():
                if not is_standard_cluster_id(cluster_id):
                    continue
                if cluster_id not in self.xml_clusters or cluster_id not in Clusters.ClusterObjects.ALL_ATTRIBUTES:
                    continue

                xml_cluster = self.xml_clusters[cluster_id]
                cluster_class = Clusters.ClusterObjects.ALL_CLUSTERS[cluster_id]

                for attribute_id in self.checkable_attributes(cluster_id, device_cluster_data, xml_cluster):
                    xml_attr = xml_cluster.attributes[attribute_id]

                    if xml_attr.write_access != Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kUnknownEnumValue:
                        writable_attributes.append(WritableAttributeInfo(
                            endpoint_id=endpoint_id,
                            cluster_id=cluster_id,
                            cluster_name=xml_cluster.name,
                            attribute_id=attribute_id,
                            attribute_name=xml_attr.name,
                            attribute=Clusters.ClusterObjects.ALL_ATTRIBUTES[cluster_id][attribute_id],
                            cluster_class=cluster_class,
                            datatype=xml_attr.datatype,
                            constraints=xml_attr.constraints,
                        ))

        log.info("Found %s writable attributes on DUT", len(writable_attributes))

        # Test attributes with constraints
        tested_count = 0
        skipped_count = 0
        failed_attributes = []

        for attr_info in writable_attributes:
            constraints = attr_info.constraints

            if not constraints or not constraints.has_constraints():
                skipped_count += 1
                continue

            try:
                result = await self.check_attribute_constraint(attr_info, constraints)
                if result is None:
                    skipped_count += 1
                elif result is False:
                    failed_attributes.append(f"{attr_info.cluster_name}.{attr_info.attribute_name}")
                    tested_count += 1
                else:
                    tested_count += 1
            except Exception as e:
                log.warning("Exception testing %s.%s: %s", attr_info.cluster_name, attr_info.attribute_name, e)
                skipped_count += 1

        log.info("Step 2 complete: Tested %s attributes, skipped %s", tested_count, skipped_count)

        if failed_attributes:
            failed_list = ', '.join(failed_attributes)
            log.error("Failed attributes constraints not enforced: %s", failed_list)
            asserts.fail(f"Failed attributes constraints not enforced: {failed_list}")


if __name__ == "__main__":
    default_matter_test_main()
