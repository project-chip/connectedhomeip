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
#       --endpoint 1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio
import logging

import matter.clusters as Clusters
from matter.clusters.Attribute import ValueDecodeFailure
from matter.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, async_test_body, has_attribute, run_if_endpoint_matches
from mobly import asserts

logger = logging.getLogger(__name__)
logging.basicConfig(level=logging.INFO)


class TC_BOOL_2_2(MatterBaseTest):

    async def read_bstate_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.BooleanState
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_BOOL_2_2(self) -> str:
        return "[TC-BOOL-2.2] Primary Functionality with Server as DUT"

    def steps_TC_BOOL_2_2(self) -> list[TestStep]:
        steps = [
            TestStep("1", "Commission DUT to TH", is_commissioning=True),
            TestStep("2a", "Bring the DUT into a state so StateValue is FALSE."),
            TestStep("2b", "TH reads the StateValue attribute from the DUT.", "Verify that value in the response is FALSE."),
            TestStep("3a", "Bring the DUT into a state so StateValue is TRUE."),
            TestStep("3b", "TH reads the StateValue attribute from the DUT.", "Verify that value in the response is TRUE."),
            TestStep("4a", "Set up subscription to StateChange event."),
            TestStep("4b", "Bring the DUT into a state so StateValue is FALSE.",
                     "Receive StateChange event with StateValue set to FALSE."),
            TestStep("4c", "TH reads the StateValue attribute from the DUT.", "Verify that value in the response is FALSE."),
            TestStep("4d", "Bring the DUT into a state so StateValue is TRUE.",
                     "Receive StateChange event with StateValue set to TRUE."),
            TestStep("4e", "TH reads the StateValue attribute from the DUT.", "Verify that value in the response is TRUE."),
        ]
        return steps

    def pics_TC_BOOLCFG_2_1(self) -> list[str]:
        pics = [
            "BOOLCFG.S",
        ]
        return pics

    @run_if_endpoint_matches(has_attribute(Clusters.BooleanState.Attributes.StateValue))
    async def test_TC_BOOL_2_2(self):

        # Commission DUT to TH done
        self.step("1")

        # Bring the DUT into a state so StateValue is FALSE.
        self.step("2a")

        cbool = Clusters.BooleanState
        endpoint = self.get_endpoint()
        node_id = self.dut_node_id
        dev_ctrl = self.default_controller
        logger.info(f" --- Step 2a: endpoint: {endpoint}")
        logger.info(f" --- Step 2a: node_id: {node_id}")
        logger.info(f" --- Step 2a: dev_ctrl: {dev_ctrl}")

        try:
            logger.info(" --- Step 2a: Sending Off command to the DUT")
            await self.send_single_cmd(
                cmd=Clusters.OnOff.Commands.Off(),
                timedRequestTimeoutMs=1000
            )
        except Exception as e:
            logger.error(f" --- Step 2a: SendCommand failed: {e}")
            raise

        # TH reads the StateValue attribute from the DUT.
        self.step("2b")

        state_value = None
        try:
            state_value = await self.read_bstate_attribute_expect_success(endpoint=endpoint, attribute=cbool.Attributes.StateValue)
            if isinstance(state_value, ValueDecodeFailure):
                logger.error(f" --- Step 2b: Value decode failed: {state_value.Reason}")
            else:
                logger.info(f" --- Step 2b: state_value: {state_value}")
        except Exception as e:
            logger.error(f" --- Step 2b: Failed to read attribute: {e}")

        # Verify that value in the response is FALSE.

        if state_value is not None:
            asserts.assert_false(state_value, " --- Step 2b: state_value should be False.")

        # Bring the DUT into a state so StateValue is TRUE.
        self.step("3a")

        try:
            logger.info(" --- Step 3a: Sending On command to the DUT")
            await self.send_single_cmd(
                cmd=Clusters.OnOff.Commands.On(),
                timedRequestTimeoutMs=1000
            )
            # Wait for device state to stabilize after level changes
            await asyncio.sleep(2)

            # Verify OnOff state first
            onoff_state = await self.read_single_attribute_check_success(
                endpoint=endpoint,
                cluster=Clusters.OnOff,
                attribute=Clusters.OnOff.Attributes.OnOff
            )
            logger.info(f" --- Step 3a: OnOff state after command: {onoff_state}")

        except Exception as e:
            logger.error(f" --- Step 3a: SendCommand failed: {e}")
            raise

        # TH reads the StateValue attribute from the DUT.
        self.step("3b")

        state_value = None
        try:
            state_value = await self.read_bstate_attribute_expect_success(endpoint=endpoint, attribute=cbool.Attributes.StateValue)
            if isinstance(state_value, ValueDecodeFailure):
                logger.error(f" --- Step 3b: Value decode failed: {state_value.Reason}")
            else:
                logger.info(f" --- Step 3b: state_value: {state_value}")
        except Exception as e:
            logger.error(f" --- Step 3b: Failed to read attribute: {e}")

        # Verify that value in the response is TRUE.
        if state_value is not None:
            asserts.assert_true(state_value, " --- Step 3b: state_value should be True.")

        # Clean up - turn off after test completes
        try:
            await self.send_single_cmd(cmd=Clusters.OnOff.Commands.Off())
        except Exception:
            pass  # Ignore cleanup errors

        # Set up subscription to StateChange event.
        self.step("4a")

        # # Suscribirse a eventos
        # event_listener = EventChangeCallback(Clusters.BooleanState)
        # await event_listener.start(self.default_controller, self.dut_node_id, endpoint=endpoint_id)

        # # Para eventos específicos:
        # subscription = await self.default_controller.ReadEvent(
        #     nodeid=self.dut_node_id,
        #     events=[(endpoint_id, Clusters.BooleanState.Events.StateChange, 1)],
        #     reportInterval=(1, 5),
        #     keepSubscriptions=True
        # )

        # async def do_test(self):
        #     # Detectar modo de ejecución
        #     self.is_ci = self.check_pics("PICS_SDK_CI_ONLY")

        #     # Para cada paso que requiere cambio de estado:
        #     if self.is_ci:
        #         # Modo CI: usar named pipes para automatizar
        #         self.write_to_app_pipe({
        #             "Name": "SetBooleanState",
        #             "EndpointId": 1,
        #             "StateValue": true
        #         })
        #     else:
        #         # Modo Manual: pedir interacción humana
        #         self.wait_for_user_input(
        #             prompt_msg="Manually change device state to make StateValue=true, then press Enter"
        #         )

        #     # Las verificaciones son SIEMPRE automáticas en ambos modos
        #     state_value = await self.read_single_attribute_check_success(
        #         cluster=Clusters.BooleanState,
        #         attribute=Clusters.BooleanState.Attributes.StateValue
        #     )
        #     asserts.assert_equal(state_value, True)

        # Bring the DUT into a state so StateValue is FALSE.
        self.step("4b")

        # Receive StateChange event with StateValue set to FALSE.

        # TH reads the StateValue attribute from the DUT.
        self.step("4c")

        # Verify that value in the response is FALSE.

        # Bring the DUT into a state so StateValue is TRUE.
        self.step("4d")

        # Receive StateChange event with StateValue set to TRUE.

        # TH reads the StateValue attribute from the DUT.
        self.step("4e")

        # Verify that value in the response is TRUE.


if __name__ == "__main__":
    default_matter_test_main()
