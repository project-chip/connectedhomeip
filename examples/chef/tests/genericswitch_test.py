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

import logging

from actions_service import actions_service_pb2
from mobly import asserts
from pw_hdlc import rpc
from pw_system.device_connection import create_device_serial_or_socket_connection

import matter.clusters as Clusters
from matter.testing.decorators import async_test_body
from matter.testing.event_attribute_reporting import EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest, TestStep
from matter.testing.runner import default_matter_test_main

logger = logging.getLogger(__name__)


class TC_GENERICSWITCH(MatterBaseTest):
    """Tests for chef rootnode_genericswitch_9866e35d0b device."""

    # Switch endpoint that supports triple press
    _SWITCH_TRIPLE_PRESS_ENDPOINT = 1
    _SWITCH_TRIPLE_PRESS_FEATURE_MAP = 30
    _SWITCH_TRIPLE_PRESS_TAG_LIST = [
        Clusters.Objects.Globals.Structs.SemanticTagStruct(
            mfgCode=Clusters.Types.NullValue, namespaceID=8, tag=2, label=None),
        Clusters.Objects.Globals.Structs.SemanticTagStruct(
            mfgCode=Clusters.Types.NullValue, namespaceID=7, tag=1, label=None)
    ]
    _SWITCH_TRIPLE_PRESS_NUMBER_OF_POSITIONS = 2

    # Switch endpoint that supports single press only
    _SWITCH_SINGLE_PRESS_ENDPOINT = 2
    _SWITCH_SINGLE_PRESS_FEATURE_MAP = 2
    _SWITCH_SINGLE_PRESS_TAG_LIST = [
        Clusters.Objects.Globals.Structs.SemanticTagStruct(
            mfgCode=Clusters.Types.NullValue, namespaceID=8, tag=3, label=None),
        Clusters.Objects.Globals.Structs.SemanticTagStruct(
            mfgCode=Clusters.Types.NullValue, namespaceID=7, tag=2, label=None)
    ]
    _SWITCH_SINGLE_PRESS_NUMBER_OF_POSITIONS = 2

    _PW_RPC_SOCKET_ADDR = "0.0.0.0:33000"
    _PW_RPC_BAUD_RATE = 115200

    async def _read_switch_feature_map(self, endpoint):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=Clusters.Objects.Switch,
            attribute=Clusters.Objects.Switch.Attributes.FeatureMap
        )

    async def _read_switch_number_of_positions(self, endpoint):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=Clusters.Objects.Switch,
            attribute=Clusters.Objects.Switch.Attributes.NumberOfPositions
        )

    async def _read_descriptor_semantic_tags(self, endpoint):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=Clusters.Objects.Descriptor,
            attribute=Clusters.Objects.Descriptor.Attributes.TagList
        )

    def _inject_switch_events(self, device, endpoint, actions: list):
        result = device.rpcs.chip.rpc.Actions.Set(
            endpoint_id=endpoint,
            cluster_id=Clusters.Objects.Switch.id,
            actions=actions,
        )
        asserts.assert_true(result.status.ok(), msg="PwRPC status not ok.")

    def desc_TC_GENERICSWITCH(self) -> str:
        return "[TC_GENERICSWITCH] chef genericswitch functionality test."

    def steps_TC_GENERICSWITCH(self):
        return [
            TestStep(
                1, "[TC_GENERICSWITCH] Commissioning already done.", is_commissioning=True),
            TestStep(
                2, "[TC_GENERICSWITCH] Triple press endpoint feature map."),
            TestStep(3, "[TC_GENERICSWITCH] Triple press endpoint tag list."),
            TestStep(
                4, "[TC_GENERICSWITCH] Triple press endpoint number of positions."),
            TestStep(
                5, "[TC_GENERICSWITCH] Single press endpoint feature map."),
            TestStep(6, "[TC_GENERICSWITCH] Single press endpoint tag list."),
            TestStep(
                7, "[TC_GENERICSWITCH] Single press endpoint number of positions."),
            TestStep(
                8, "[TC_GENERICSWITCH] Test switch press events via RPC injection.")
        ]

    @async_test_body
    async def test_TC_GENERICSWITCH(self):
        """Run all steps."""

        self.step(1)
        # Commissioning already done.

        self.step(2)
        feature_map = await self._read_switch_feature_map(self._SWITCH_TRIPLE_PRESS_ENDPOINT)
        asserts.assert_equal(
            feature_map, self._SWITCH_TRIPLE_PRESS_FEATURE_MAP)

        self.step(3)
        tag_list = await self._read_descriptor_semantic_tags(self._SWITCH_TRIPLE_PRESS_ENDPOINT)
        asserts.assert_equal(tag_list, self._SWITCH_TRIPLE_PRESS_TAG_LIST)

        self.step(4)
        number_of_positions = await self._read_switch_number_of_positions(self._SWITCH_TRIPLE_PRESS_ENDPOINT)
        asserts.assert_equal(
            number_of_positions,
            self._SWITCH_TRIPLE_PRESS_NUMBER_OF_POSITIONS
        )

        self.step(5)
        feature_map = await self._read_switch_feature_map(self._SWITCH_SINGLE_PRESS_ENDPOINT)
        asserts.assert_equal(
            feature_map, self._SWITCH_SINGLE_PRESS_FEATURE_MAP)

        self.step(6)
        tag_list = await self._read_descriptor_semantic_tags(self._SWITCH_SINGLE_PRESS_ENDPOINT)
        asserts.assert_equal(tag_list, self._SWITCH_SINGLE_PRESS_TAG_LIST)

        self.step(7)
        number_of_positions = await self._read_switch_number_of_positions(self._SWITCH_SINGLE_PRESS_ENDPOINT)
        asserts.assert_equal(
            number_of_positions,
            self._SWITCH_SINGLE_PRESS_NUMBER_OF_POSITIONS
        )

        self.step(8)
        device_connection = create_device_serial_or_socket_connection(
            device="",
            baudrate=self._PW_RPC_BAUD_RATE,
            token_databases=[],
            socket_addr=self._PW_RPC_SOCKET_ADDR,
            compiled_protos=[actions_service_pb2],
            rpc_logging=True,
            channel_id=rpc.DEFAULT_CHANNEL_ID,
            hdlc_encoding=True,
            device_tracing=False,
        )

        with device_connection as device:
            logger.info("Started Pw socket connection.")

            logger.info("Testing events on Endpoint %d",
                        self._SWITCH_TRIPLE_PRESS_ENDPOINT)
            events_callback_1 = EventSubscriptionHandler(
                expected_cluster=Clusters.Objects.Switch)
            await events_callback_1.start(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=self._SWITCH_TRIPLE_PRESS_ENDPOINT,
            )
            self._inject_switch_events(  # A sequence of events simulating double press.
                device,
                self._SWITCH_TRIPLE_PRESS_ENDPOINT,
                actions=[
                    actions_service_pb2.Action(
                        type=actions_service_pb2.ActionType.EMIT_EVENT,
                        delayMs=0,
                        actionId=Clusters.Objects.Switch.Events.InitialPress.event_id,
                        arg1=1,  # New Position = 1
                    ),
                    actions_service_pb2.Action(
                        type=actions_service_pb2.ActionType.EMIT_EVENT,
                        delayMs=10,
                        actionId=Clusters.Objects.Switch.Events.ShortRelease.event_id,
                        arg1=1,  # Previous Position = 1
                    ),
                    actions_service_pb2.Action(
                        type=actions_service_pb2.ActionType.EMIT_EVENT,
                        delayMs=10,
                        actionId=Clusters.Objects.Switch.Events.MultiPressOngoing.event_id,
                        arg1=1,  # New Position = 1
                        arg2=2,  # number of presses
                    ),
                    actions_service_pb2.Action(
                        type=actions_service_pb2.ActionType.EMIT_EVENT,
                        delayMs=10,
                        actionId=Clusters.Objects.Switch.Events.ShortRelease.event_id,
                        arg1=1,  # Previous Position = 1
                    ),
                    actions_service_pb2.Action(
                        type=actions_service_pb2.ActionType.EMIT_EVENT,
                        delayMs=100,
                        actionId=Clusters.Objects.Switch.Events.MultiPressComplete.event_id,
                        arg1=1,  # Previous Position = 1
                        arg2=2,  # number of presses
                    ),
                ]
            )
            logger.info("Injected multi press events.")
            events_callback_1.wait_for_event_report(
                Clusters.Objects.Switch.Events.InitialPress)
            events_callback_1.wait_for_event_report(
                Clusters.Objects.Switch.Events.ShortRelease)
            events_callback_1.wait_for_event_report(
                Clusters.Objects.Switch.Events.MultiPressOngoing)
            events_callback_1.wait_for_event_report(
                Clusters.Objects.Switch.Events.ShortRelease)
            events_callback_1.wait_for_event_report(
                Clusters.Objects.Switch.Events.MultiPressComplete)
            logger.info("Multi press events verified.")
            events_callback_1.reset()
            events_callback_1.cancel()

            logger.info("Testing events on Endpoint %d",
                        self._SWITCH_SINGLE_PRESS_ENDPOINT)
            events_callback_2 = EventSubscriptionHandler(
                expected_cluster=Clusters.Objects.Switch)
            await events_callback_2.start(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=self._SWITCH_SINGLE_PRESS_ENDPOINT,
            )
            self._inject_switch_events(  # An initial press.
                device,
                self._SWITCH_SINGLE_PRESS_ENDPOINT,
                actions=[
                    actions_service_pb2.Action(
                        type=actions_service_pb2.ActionType.EMIT_EVENT,
                        delayMs=0,
                        actionId=Clusters.Objects.Switch.Events.InitialPress.event_id,
                        arg1=1,  # New Position = 1
                    ),
                ]
            )
            logger.info("Injected initial press.")
            events_callback_2.wait_for_event_report(
                Clusters.Objects.Switch.Events.InitialPress)
            logger.info("Initial press events verified.")
            events_callback_2.reset()
            events_callback_2.cancel()


if __name__ == "__main__":
    default_matter_test_main()
