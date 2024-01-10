#
#    Copyright (c) 2023 Project CHIP Authors
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


import logging
import queue
import time

import chip.clusters as Clusters
from chip.clusters import ClusterObjects as ClusterObjects
from chip.clusters.Attribute import EventReadResult, SubscriptionTransaction
from chip.clusters.Types import NullValue
from chip.interaction_model import InteractionModelError, Status
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

logger = logging.getLogger(__name__)


class EventChangeCallback:
    def __init__(self, expected_cluster: ClusterObjects):
        self._q = queue.Queue()
        self._expected_cluster = expected_cluster

    async def start(self, dev_ctrl, nodeid):
        self._subscription = await dev_ctrl.ReadEvent(nodeid,
                                                      events=[(1, self._expected_cluster, 1)], reportInterval=(1, 5),
                                                      fabricFiltered=False, keepSubscriptions=True, autoResubscribe=False)
        self._subscription.SetEventUpdateCallback(self.__call__)

    def __call__(self, res: EventReadResult, transaction: SubscriptionTransaction):
        if res.Status == Status.Success and res.Header.ClusterId == self._expected_cluster.id:
            logging.info(
                f'Got subscription report for event on cluster {self._expected_cluster}: {res.Data}')
            self._q.put(res)

    def WaitForEventReport(self, expected_event: ClusterObjects.ClusterEvent):
        try:
            res = self._q.get(block=True, timeout=10)
        except queue.Empty:
            asserts.fail("Failed to receive a report for the event {}".format(expected_event))

        asserts.assert_equal(res.Header.ClusterId, expected_event.cluster_id, "Expected cluster ID not found in event report")
        asserts.assert_equal(res.Header.EventId, expected_event.event_id, "Expected event ID not found in event report")
        return res.Data


class TC_EEVSE_2_5(MatterBaseTest):
    async def read_evse_attribute_expect_success(self, endpoint, attribute):
        full_attr = getattr(Clusters.EnergyEvse.Attributes, attribute)
        cluster = Clusters.Objects.EnergyEvse
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=full_attr)

    async def check_evse_attribute(self, attribute, expected_value):
        value = await self.read_evse_attribute_expect_success(endpoint=1, attribute=attribute)
        asserts.assert_equal(value, expected_value,
                             f"Unexpected '{attribute}' value - expected {expected_value}, was {value}")

    async def send_enable_charge_command(self, endpoint: int = 0, charge_until: int = None, timedRequestTimeoutMs: int = 3000,
                                         min_charge: int = None, max_charge: int = None, expected_status: Status = Status.Success):
        try:
            await self.send_single_cmd(cmd=Clusters.EnergyEvse.Commands.EnableCharging(
                chargingEnabledUntil=charge_until,
                minimumChargeCurrent=min_charge,
                maximumChargeCurrent=max_charge),
                endpoint=1,
                timedRequestTimeoutMs=timedRequestTimeoutMs)

        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")

    async def send_start_diagnostics_command(self, endpoint: int = 0, timedRequestTimeoutMs: int = 3000,
                                             expected_status: Status = Status.Success):
        try:
            await self.send_single_cmd(cmd=Clusters.EnergyEvse.Commands.StartDiagnostics(),
                                       endpoint=1,
                                       timedRequestTimeoutMs=timedRequestTimeoutMs)

        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")

    def desc_TC_EEVSE_2_5(self) -> str:
        """Returns a description of this test"""
        return "5.1.XXX. [TC-EEVSE-2.4] Fault test functionality with DUT as Server"

    def pics_TC_EEVSE_2_5(self):
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        # In this case - there is no feature flags needed to run this test case
        return None

    def steps_TC_EEVSE_2_5(self) -> list[TestStep]:
        steps = [
            TestStep("1", "Commissioning, already done", is_commissioning=True),
            TestStep("2", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster. Verify that TestEventTriggersEnabled attribute has a value of 1 (True)"),
            TestStep("3", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER for Basic Functionality Test Event"),
            TestStep("3a", "After a few seconds TH reads from the DUT the State attribute. Verify value is 0x00 (NotPluggedIn)"),
            TestStep("3b", "TH reads from the DUT the SupplyState attribute. Verify value is 0x00 (Disabled)"),
            TestStep("3c", "TH reads from the DUT the FaultState attribute. Verify value is 0x00 (NoError)"),
            TestStep("4", "TH sends command EnableCharging with ChargingEnabledUntil=Null, minimumChargeCurrent=6000, maximumChargeCurrent=60000"),
            TestStep("4a", "TH reads from the DUT the State attribute. Verify value is 0x00 (NotPluggedIn)"),
            TestStep("4b", "TH reads from the DUT the SupplyState attribute. Verify value is 0x00 (ChargingEnabled)"),
            TestStep("5", "TH sends command StartDiagnostics"),
            TestStep("5a", "TH reads from the DUT the State attribute. Verify value is 0x00 (NotPluggedIn)"),
            TestStep("5b", "TH reads from the DUT the SupplyState attribute. Verify value is 0x04 (DisabledDiagnostics)"),
            TestStep("6", "A few seconds later TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER for EVSE Diagnostics Complete Event"),
            TestStep("6a", "TH reads from the DUT the State attribute. Verify value is 0x00 (NotPluggedIn)"),
            TestStep("6b", "TH reads from the DUT the SupplyState attribute. Verify value is 0x04 (Disabled)"),
            TestStep("7", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER for Basic Functionality Test Event Clear."),
        ]

        return steps

    async def send_test_event_triggers(self, enableKey=bytes([b for b in range(16)]), eventTrigger=0x0099000000000000):
        try:
            await self.send_single_cmd(endpoint=0,
                                       cmd=Clusters.GeneralDiagnostics.Commands.TestEventTrigger(
                                           enableKey,
                                           eventTrigger)
                                       )

        except InteractionModelError as e:
            asserts.fail(f"Unexpected error returned - {e.status}")

    # TC_EEVSE_2_5 tests steps
    async def check_test_event_triggers_enabled(self):
        full_attr = Clusters.GeneralDiagnostics.Attributes.TestEventTriggersEnabled
        cluster = Clusters.Objects.GeneralDiagnostics
        test_event_enabled = await self.read_single_attribute_check_success(endpoint=0, cluster=cluster, attribute=full_attr)
        asserts.assert_equal(test_event_enabled, True, "TestEventTriggersEnabled is False")

    async def send_test_event_trigger_basic(self):
        await self.send_test_event_triggers(eventTrigger=0x0099000000000000)

    async def send_test_event_trigger_basic_clear(self):
        await self.send_test_event_triggers(eventTrigger=0x0099000000000001)

    async def send_test_event_trigger_evse_diagnostics_complete(self):
        await self.send_test_event_triggers(eventTrigger=0x0099000000000020)

    @async_test_body
    async def test_TC_EEVSE_2_5(self):
        self.step("1")
        # Commission DUT - already done

        # Subscribe to Events and when they are sent push them to a queue for checking later
        events_callback = EventChangeCallback(Clusters.EnergyEvse)
        await events_callback.start(self.default_controller, self.dut_node_id)

        self.step("2")
        await self.check_test_event_triggers_enabled()

        self.step("3")
        await self.send_test_event_trigger_basic()

        # After a few seconds...
        time.sleep(1)

        self.step("3a")
        await self.check_evse_attribute("State", Clusters.EnergyEvse.Enums.StateEnum.kNotPluggedIn)

        self.step("3b")
        await self.check_evse_attribute("SupplyState", Clusters.EnergyEvse.Enums.SupplyStateEnum.kDisabled)

        self.step("3c")
        await self.check_evse_attribute("FaultState", Clusters.EnergyEvse.Enums.FaultStateEnum.kNoError)

        self.step("4")
        charge_until = NullValue
        min_charge_current = 6000
        max_charge_current = 60000
        await self.send_enable_charge_command(endpoint=1, charge_until=charge_until, min_charge=min_charge_current, max_charge=max_charge_current)

        self.step("4a")
        await self.check_evse_attribute("State", Clusters.EnergyEvse.Enums.StateEnum.kNotPluggedIn)

        self.step("4b")
        await self.check_evse_attribute("SupplyState", Clusters.EnergyEvse.Enums.SupplyStateEnum.kChargingEnabled)

        self.step("5")
        await self.send_start_diagnostics_command(endpoint=1)

        self.step("5a")
        await self.check_evse_attribute("State", Clusters.EnergyEvse.Enums.StateEnum.kNotPluggedIn)

        self.step("5b")
        await self.check_evse_attribute("SupplyState", Clusters.EnergyEvse.Enums.SupplyStateEnum.kDisabledDiagnostics)

        self.step("6")
        await self.send_test_event_trigger_evse_diagnostics_complete()

        self.step("6a")
        await self.check_evse_attribute("State", Clusters.EnergyEvse.Enums.StateEnum.kNotPluggedIn)

        self.step("6b")
        # It should go to disabled after a diagnostics session
        await self.check_evse_attribute("SupplyState", Clusters.EnergyEvse.Enums.SupplyStateEnum.kDisabled)

        self.step("7")
        await self.send_test_event_trigger_basic_clear()


if __name__ == "__main__":
    default_matter_test_main()
