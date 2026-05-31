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

from attributes_service import attributes_service_pb2
from mobly import asserts
from pw_hdlc import rpc
from pw_system.device_connection import create_device_serial_or_socket_connection

import matter.clusters as Clusters
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest, TestStep
from matter.testing.runner import default_matter_test_main

logger = logging.getLogger(__name__)


class TC_BooleanStateConfiguration(MatterBaseTest):
    """Tests for BooleanStateConfiguration cluster."""

    _PW_RPC_SOCKET_ADDR = "0.0.0.0:33000"
    _PW_RPC_BAUD_RATE = 115200

    def desc_TC_BooleanStateConfiguration(self) -> str:
        return "[TC_BooleanStateConfiguration] BooleanStateConfiguration cluster functionality test."

    def steps_TC_BooleanStateConfiguration(self):
        return [
            TestStep(1, "Commissioning already done.", is_commissioning=True),
            TestStep(2, "Read supported sensitivity levels."),
            TestStep(3, "Set current sensitivity level to min (0) and verify."),
            TestStep(4, "Set current sensitivity level to max and verify."),
            TestStep(5, "Read alarms supported."),
            TestStep(6, "Enable all supported alarms."),
            TestStep(7, "Activate all supported alarms via PwRPC."),
            TestStep(8, "Verify alarms are active."),
            TestStep(9, "Disable all alarms and verify alarms are inactive.")
        ]

    async def _read_bsc_attribute(self, endpoint, attribute):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=Clusters.Objects.BooleanStateConfiguration,
            attribute=attribute
        )

    async def _write_bsc_attribute(self, endpoint, attribute, value):
        return await self.write_single_attribute(
            attribute_value=attribute(value=value),
            endpoint_id=endpoint
        )

    def _write_alarms_active_pwrpc(self, device, endpoint, value: int):
        result = device.rpcs.chip.rpc.Attributes.Write(
            data=attributes_service_pb2.AttributeData(data_uint8=value),
            metadata=attributes_service_pb2.AttributeMetadata(
                endpoint=endpoint,
                cluster=Clusters.Objects.BooleanStateConfiguration.id,
                attribute_id=Clusters.Objects.BooleanStateConfiguration.Attributes.AlarmsActive.attribute_id,
                type=attributes_service_pb2.AttributeType.ZCL_BITMAP8_ATTRIBUTE_TYPE
            )
        )
        asserts.assert_true(result.status.ok(), msg="PwRPC status not ok.")

    @async_test_body
    async def test_TC_BooleanStateConfiguration(self):
        endpoint = self.user_params.get("bsc_endpoint", self.matter_test_config.endpoint)
        if endpoint is None:
            endpoint = 1

        logger.info(f"Testing BooleanStateConfiguration on endpoint {endpoint}")

        self.step(1)
        # Commissioning already done.

        self.step(2)
        supported_sensitivity_levels = await self._read_bsc_attribute(
            endpoint, Clusters.Objects.BooleanStateConfiguration.Attributes.SupportedSensitivityLevels)
        logger.info(f"SupportedSensitivityLevels: {supported_sensitivity_levels}")

        self.step(3)
        await self._write_bsc_attribute(
            endpoint, Clusters.Objects.BooleanStateConfiguration.Attributes.CurrentSensitivityLevel, 0)
        current_sensitivity_level = await self._read_bsc_attribute(
            endpoint, Clusters.Objects.BooleanStateConfiguration.Attributes.CurrentSensitivityLevel)
        asserts.assert_equal(current_sensitivity_level, 0, "CurrentSensitivityLevel should be 0")

        self.step(4)
        max_sensitivity_level = supported_sensitivity_levels - 1
        await self._write_bsc_attribute(
            endpoint, Clusters.Objects.BooleanStateConfiguration.Attributes.CurrentSensitivityLevel, max_sensitivity_level)
        current_sensitivity_level = await self._read_bsc_attribute(
            endpoint, Clusters.Objects.BooleanStateConfiguration.Attributes.CurrentSensitivityLevel)
        asserts.assert_equal(current_sensitivity_level, max_sensitivity_level,
                             f"CurrentSensitivityLevel should be {max_sensitivity_level}")

        self.step(5)
        alarms_supported = await self._read_bsc_attribute(
            endpoint, Clusters.Objects.BooleanStateConfiguration.Attributes.AlarmsSupported)
        logger.info(f"AlarmsSupported: {alarms_supported}")

        self.step(6)
        await self.send_single_cmd(
            cmd=Clusters.Objects.BooleanStateConfiguration.Commands.EnableDisableAlarm(
                alarmsToEnableDisable=alarms_supported
            ),
            endpoint=endpoint
        )

        # Do the steps below in a Pw socket connection.
        device_connection = create_device_serial_or_socket_connection(
            device="",
            baudrate=self._PW_RPC_BAUD_RATE,
            token_databases=[],
            socket_addr=self._PW_RPC_SOCKET_ADDR,
            compiled_protos=[attributes_service_pb2],
            rpc_logging=True,
            channel_id=rpc.DEFAULT_CHANNEL_ID,
            hdlc_encoding=True,
            device_tracing=False,
        )

        with device_connection as device:
            self.step(7)
            # Send a Pigweed request to activate all supported alarms
            self._write_alarms_active_pwrpc(device, endpoint, alarms_supported)

            self.step(8)
            # Read and check alarms are active is alarms_supported
            alarms_active = await self._read_bsc_attribute(
                endpoint, Clusters.Objects.BooleanStateConfiguration.Attributes.AlarmsActive)
            asserts.assert_equal(alarms_active, alarms_supported,
                                 f"AlarmsActive should be {alarms_supported}")

            self.step(9)
            # Send EnableDisableAlarm with bitmap equal to 0 (disable all alarms)
            await self.send_single_cmd(
                cmd=Clusters.Objects.BooleanStateConfiguration.Commands.EnableDisableAlarm(
                    alarmsToEnableDisable=0
                ),
                endpoint=endpoint
            )

            # Then read and verify AlarmsActive is 0
            alarms_active = await self._read_bsc_attribute(
                endpoint, Clusters.Objects.BooleanStateConfiguration.Attributes.AlarmsActive)
            asserts.assert_equal(alarms_active, 0, "AlarmsActive should be 0 after disabling all alarms")


if __name__ == "__main__":
    default_matter_test_main()
