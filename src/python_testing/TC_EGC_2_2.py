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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ENERGY_GATEWAY_APP}
#     app-args: >
#       --discriminator 1234
#       --KVS kvs1
#       --trace-to json:${TRACE_APP}.json
#       --enable-key 000102030405060708090a0b0c0d0e0f
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --hex-arg enableKey:000102030405060708090a0b0c0d0e0f
#       --endpoint 1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.testing.event_attribute_reporting import EventChangeCallback
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_cluster, run_if_endpoint_matches
from mobly import asserts
from TC_EGCTestBase import ElectricalGridConditionsTestBaseHelper

cluster = Clusters.ElectricalGridConditions


class TC_EGC_2_2(ElectricalGridConditionsTestBaseHelper, MatterBaseTest):
    """Implementation of test case TC_EGC_2_2."""

    def desc_TC_EGC_2_2(self) -> str:
        """Returns a description of this test"""
        return "[TC-EGC-2.2] Primary Functionality with DUT as Server"

    def pics_TC_EGC_2_2(self) -> list[str]:
        """This function returns a list of PICS for this test case that must be True for the test to be run"""
        return ["EGC.S"]

    def steps_TC_EGC_2_2(self) -> list[TestStep]:
        steps = [
            TestStep("1", "Commission DUT to TH (can be skipped if done in a preceding test).",
                     is_commissioning=True),
            TestStep("2", "Set up a subscription to all ElectricalGridConditions cluster events"),
            TestStep("3", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster",
                     "Value has to be 1 (True)"),
            TestStep("4", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EGC.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EGC.TESTEVENTTRIGGER for Current Conditions Update Test Event",
                     """Verify DUT responds w/ status SUCCESS(0x00) and event EGC.S.E0000(CurrentConditionsChanged) sent.
                     Store the event's CurrentConditions field as NewCurrentConditions."""),
            TestStep("4a", "TH reads from the DUT the CurrentConditions attribute.",
                     """Verify that the DUT response contains a ElectricalGridConditionsStruct value.
                        Verify that the value matches the NewCurrentConditions from step 4."""),
        ]
        return steps

    @run_if_endpoint_matches(has_cluster(Clusters.ElectricalGridConditions))
    async def test_TC_EGC_2_2(self):
        endpoint = self.get_endpoint()
        attributes = cluster.Attributes

        self.step("1")
        # Commission DUT - already done

        self.step("2")
        events_callback = EventChangeCallback(cluster)
        await events_callback.start(self.default_controller,
                                    self.dut_node_id,
                                    endpoint)

        self.step("3")
        # TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster
        await self.check_test_event_triggers_enabled()

        self.step("4")
        # TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EGC.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EGC.TESTEVENTTRIGGER for Current Conditions Update Test Event",
        # Verify DUT responds w/ status SUCCESS(0x00) and event EGC.S.E0000(CurrentConditionsChanged) sent.
        #  Store the event's CurrentConditions field as NewCurrentConditions.
        await self.send_test_event_trigger_current_conditions_update()

        event_data = events_callback.wait_for_event_report(
            Clusters.ElectricalGridConditions.Events.CurrentConditionsChanged)

        newCurrentConditions = event_data.currentConditions
        if newCurrentConditions is not NullValue:
            asserts.assert_true(isinstance(
                newCurrentConditions, cluster.Structs.ElectricalGridConditionsStruct), "val must be of type ElectricalGridConditionsStruct")

            # The other aspects of this verification are handled by the helper
            self.check_ElectricalGridConditionsStruct(cluster=cluster,
                                                      struct=newCurrentConditions)

        self.step("4a")
        # TH reads from the DUT the CurrentConditions attribute.
        #  Verify that the DUT response contains a ElectricalGridConditionsStruct value.
        #  Verify that the value matches the NewCurrentConditions from step 4.
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster,
                                                             attribute=attributes.CurrentConditions)
        if val is not NullValue:
            asserts.assert_true(isinstance(
                val, cluster.Structs.ElectricalGridConditionsStruct), "val must be of type ElectricalGridConditionsStruct")
            self.check_ElectricalGridConditionsStruct(cluster=cluster, struct=val)

        asserts.assert_equal(val, newCurrentConditions, "CurrentConditions is not equal to NewCurrentConditions")


if __name__ == "__main__":
    default_matter_test_main()
