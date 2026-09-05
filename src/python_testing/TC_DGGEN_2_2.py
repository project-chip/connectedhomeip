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
#     app-args: --discriminator 1234 --KVS kvs1 --app-pipe /tmp/dggen_2_2_fifo --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --endpoint 0
#       --app-pipe /tmp/dggen_2_2_fifo
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===
#

import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.testing import matter_asserts
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.event_attribute_reporting import EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

logger = logging.getLogger(__name__)
logger.setLevel(logging.INFO)


class TC_DGGEN_2_2(MatterBaseTest):

    def _induce_fault_event(self, pipe_command: str, prompt: str):
        """Triggers a fault change on the DUT: named pipe on the CI app, user action on a real DUT."""
        if self.is_pics_sdk_ci_only:
            self.write_to_app_pipe({"Name": pipe_command})
        else:
            self.wait_for_user_input(prompt_msg=prompt)

    def _assert_valid_fault_lists(self, event_data, enum_type, event_name: str, max_faults: int):
        """Validates the current and previous fault lists of a fault change event."""
        for fault_list, field_name in ((event_data.current, "current"), (event_data.previous, "previous")):
            matter_asserts.assert_list(fault_list, f"{event_name} {field_name}", max_length=max_faults)
            for fault in fault_list:
                matter_asserts.assert_valid_enum(fault, f"{event_name} {field_name} entry", enum_type)
                # Raw values outside the defined enum values decode to the kUnknownEnumValue
                # sentinel, which is an instance of the enum type and passes the check above.
                asserts.assert_not_equal(fault, enum_type.kUnknownEnumValue,
                                         f"{event_name} {field_name} entry is not a defined {enum_type.__name__} value.")
            asserts.assert_equal(len(fault_list), len(set(fault_list)),
                                 f"{event_name} {field_name} contains duplicate faults, it must represent a set.")
        asserts.assert_not_equal(set(event_data.current), set(event_data.previous),
                                 f"{event_name} current and previous fault sets are identical, no change represented.")
        logger.info("%s: previous=%s current=%s", event_name, event_data.previous, event_data.current)

    def desc_TC_DGGEN_2_2(self) -> str:
        """Returns a description of this test"""
        return "[TC-DGGEN-2.2] Event Functionality with Server as DUT"

    def pics_TC_DGGEN_2_2(self) -> list[str]:
        return ["DGGEN.S"]

    def steps_TC_DGGEN_2_2(self) -> list[TestStep]:
        return [
            TestStep(1, "Commission DUT to TH if not already commissioned", is_commissioning=True),
            TestStep(2, "TH subscribes to the GeneralDiagnostics cluster events on the DUT. "
                     "The DUT is triggered to induce a change in its set of detected hardware faults. "
                     "TH waits for the HardwareFaultChange event.",
                     "Verify TH receives the HardwareFaultChange event, that the current and previous "
                     "fields are each a set of at most 11 values specified by the HardwareFault enum, "
                     "and that the two sets differ."),
            TestStep(3, "The DUT is triggered to induce a change in its set of detected radio faults. "
                     "TH waits for the RadioFaultChange event.",
                     "Verify TH receives the RadioFaultChange event, that the current and previous "
                     "fields are each a set of at most 7 values specified by the RadioFault enum, "
                     "and that the two sets differ."),
            TestStep(4, "The DUT is triggered to induce a change in its set of detected network faults. "
                     "TH waits for the NetworkFaultChange event.",
                     "Verify TH receives the NetworkFaultChange event, that the current and previous "
                     "fields are each a set of at most 4 values specified by the NetworkFault enum, "
                     "and that the two sets differ."),
            TestStep(5, "Reboot the DUT with a normal reboot process. TH reads the BootReason event from the DUT.",
                     "Verify a new BootReason event is emitted after the reboot and its data field "
                     "contains a value specified by the BootReason enum."),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.GeneralDiagnostics))
    async def test_TC_DGGEN_2_2(self):

        endpoint = self.get_endpoint()
        events = Clusters.GeneralDiagnostics.Events
        enums = Clusters.GeneralDiagnostics.Enums

        # STEP 1: Commission DUT to TH (already done)
        self.step(1)

        # Subscribe for GeneralDiagnostics events, used by steps 2 to 4. Inducing radio
        # or network faults on a real DUT can drop the Matter link, so let the
        # subscription re-establish itself.
        events_callback = EventSubscriptionHandler(expected_cluster=Clusters.GeneralDiagnostics)
        await events_callback.start(self.default_controller, self.dut_node_id, endpoint, autoResubscribe=True)

        # STEP 2: DUT induces a hardware faults change, TH waits for the HardwareFaultChange event
        self.step(2)
        if self.pics_guard(self.check_pics("DGGEN.S.E00")):
            self._induce_fault_event(
                "HardwareFaultChange",
                "Induce a change in the set of hardware faults detected by the DUT "
                "(for example, disconnect a power source). Press Enter when done.\n")
            event_data = events_callback.wait_for_event_report(events.HardwareFaultChange)
            self._assert_valid_fault_lists(event_data, enums.HardwareFaultEnum, "HardwareFaultChange", max_faults=11)

        # STEP 3: DUT induces a radio faults change, TH waits for the RadioFaultChange event
        self.step(3)
        if self.pics_guard(self.check_pics("DGGEN.S.E01")):
            self._induce_fault_event(
                "RadioFaultChange",
                "Induce a change in the set of radio faults detected by the DUT "
                "(for example, shield the DUT radio). Press Enter when done.\n")
            event_data = events_callback.wait_for_event_report(events.RadioFaultChange)
            self._assert_valid_fault_lists(event_data, enums.RadioFaultEnum, "RadioFaultChange", max_faults=7)

        # STEP 4: DUT induces a network faults change, TH waits for the NetworkFaultChange event
        self.step(4)
        if self.pics_guard(self.check_pics("DGGEN.S.E02")):
            self._induce_fault_event(
                "NetworkFaultChange",
                "Induce a change in the set of network faults detected by the DUT "
                "(for example, disconnect the network medium). Press Enter when done.\n")
            event_data = events_callback.wait_for_event_report(events.NetworkFaultChange)
            self._assert_valid_fault_lists(event_data, enums.NetworkFaultEnum, "NetworkFaultChange", max_faults=4)

        # STEP 5: Reboot the DUT, TH reads the BootReason event
        self.step(5)
        if self.pics_guard(self.check_pics("DGGEN.S.E03")):
            # Event numbers are monotonic and persist across reboots: record the
            # newest BootReason event number before the reboot, so the check below
            # proves this reboot emitted a new event.
            events_before = await self.default_controller.ReadEvent(
                nodeId=self.dut_node_id, events=[(endpoint, events.BootReason, 0)])
            # -1 sentinel: event numbers start at 0, so an empty pre-reboot result
            # must not exclude a first-ever event numbered 0.
            last_event_number_before = max(
                (e.Header.EventNumber for e in events_before), default=-1)

            await self.request_device_reboot()

            boot_reason_events = await self.default_controller.ReadEvent(
                nodeId=self.dut_node_id, events=[(endpoint, events.BootReason, 0)])
            new_events = [e for e in boot_reason_events if e.Header.EventNumber > last_event_number_before]
            asserts.assert_true(new_events, "No new BootReason event emitted after the reboot.")
            latest_event = max(new_events, key=lambda e: e.Header.EventNumber)
            matter_asserts.assert_valid_enum(latest_event.Data.bootReason, "BootReason", enums.BootReasonEnum)
            # Raw values outside the defined enum values decode to the kUnknownEnumValue
            # sentinel, which is an instance of the enum type and passes the check above.
            asserts.assert_not_equal(latest_event.Data.bootReason, enums.BootReasonEnum.kUnknownEnumValue,
                                     "BootReason is not a defined BootReasonEnum value.")
            logger.info("BootReason (after reboot): %s", latest_event.Data.bootReason)


if __name__ == "__main__":
    default_matter_test_main()
