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

"""
# Setup env
source ./scripts/bootstrap.sh -p linux
scripts/run_in_build_env.sh './scripts/build_python.sh --install_virtual_env out/python_env -pw true'

# Build app
scripts/run_in_build_env.sh "./scripts/build/build_examples.py \
    --target linux-x64-all-devices-ipv6only-no-ble-no-wifi-rpc-asan-clang-test build"

# Run test
scripts/run_in_python_env.sh out/python_env \
    './scripts/tests/run_python_test.py \
    --app out/linux-x64-all-devices-ipv6only-no-ble-no-wifi-rpc-asan-clang-test/all-devices-app \
    --app-args "\
        --device contact-sensor:1 \
        --device water-leak-detector:2" \
    --factory-reset \
    --script examples/all-devices-app/all-devices-common/devices/boolean-state-sensor/test.py \
    --script-args "\
        --commissioning-method on-network \
        --discriminator 3840 \
        --passcode 20202021\
        " \
    --app-stdin-pipe /tmp/app_stdin.txt'

"""

import logging

from attributes_service import attributes_service_pb2
from mobly import asserts
from pw_hdlc import rpc
from pw_system.device_connection import create_device_serial_or_socket_connection

import matter.clusters as Clusters
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import default_matter_test_main

logger = logging.getLogger(__name__)


class BooleanStateSensorCommissioningTest(MatterBaseTest):
    """Simple test that commissions and reads the Boolean State Sensor device."""

    async def read_boolean_state_value(self, endpoint: int):
        """Helper method to read the StateValue attribute from the BooleanState cluster on a given endpoint."""
        return await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=Clusters.BooleanState,
            attribute=Clusters.BooleanState.Attributes.StateValue
        )

    def write_boolean_state_value_pwrpc(self, device, endpoint: int, value: bool):
        """Helper method to write the StateValue attribute on a given endpoint via Pigweed RPC."""
        result = device.rpcs.chip.rpc.Attributes.Write(
            data=attributes_service_pb2.AttributeData(data_bool=value),
            metadata=attributes_service_pb2.AttributeMetadata(
                endpoint=endpoint,
                cluster=Clusters.Objects.BooleanState.id,
                attribute_id=Clusters.Objects.BooleanState.Attributes.StateValue.attribute_id,
                type=attributes_service_pb2.AttributeType.ZCL_BOOLEAN_ATTRIBUTE_TYPE
            )
        )
        asserts.assert_true(result.status.ok(), msg=f"PwRPC write failed for endpoint {endpoint}.")

    @async_test_body
    async def test_TC_BOOL_1_1(self):
        "[TC_BOOL_1_1] Boolean State Sensor Commissioning and State Read Test"

        self.step(1, "Commissioning, already done", is_commissioning=True)
        logger.info("Successfully completed commissioning step.")

        # Step 2: Read Boolean State Value on Endpoint 1 and Endpoint 2
        self.step(2, "Read Boolean State Value on Endpoint 1 and Endpoint 2")

        # Read Endpoint 1
        initial_val_ep1 = await self.read_boolean_state_value(endpoint=1)
        logger.info("Initial Boolean State Value on Endpoint 1: %s", initial_val_ep1)

        # Read Endpoint 2
        initial_val_ep2 = await self.read_boolean_state_value(endpoint=2)
        logger.info("Initial Boolean State Value on Endpoint 2: %s", initial_val_ep2)

        # Step 3: Toggle and assert state values on Endpoint 1 and Endpoint 2 independently via PwRPC
        self.step(3, "Toggle and assert state values on Endpoint 1 and Endpoint 2 independently via PwRPC")

        # Establish PwRPC connection
        logger.info("Establishing Pigweed RPC connection...")
        device_connection = create_device_serial_or_socket_connection(
            device="",
            baudrate=115200,
            token_databases=[],
            socket_addr="127.0.0.1:33000",
            compiled_protos=[attributes_service_pb2],
            rpc_logging=True,
            channel_id=rpc.DEFAULT_CHANNEL_ID,
            hdlc_encoding=True,
            device_tracing=False,
        )

        with device_connection as device:
            # 1. Toggle Endpoint 1's state value and assert Endpoint 1's state changed while Endpoint 2's state remained intact.
            new_val_ep1 = not initial_val_ep1
            logger.info("Toggling Endpoint 1 from %s to %s via PwRPC...", initial_val_ep1, new_val_ep1)
            self.write_boolean_state_value_pwrpc(device, endpoint=1, value=new_val_ep1)

            # Assert Endpoint 1 changed
            current_val_ep1 = await self.read_boolean_state_value(endpoint=1)
            asserts.assert_equal(current_val_ep1, new_val_ep1, "Endpoint 1 state did not toggle successfully!")

            # Assert Endpoint 2 remained intact
            current_val_ep2 = await self.read_boolean_state_value(endpoint=2)
            asserts.assert_equal(current_val_ep2, initial_val_ep2, "Endpoint 2 state was unexpectedly modified!")

            # 2. Toggle Endpoint 2's state value and assert Endpoint 2's state changed while Endpoint 1's state remained intact.
            new_val_ep2 = not initial_val_ep2
            logger.info("Toggling Endpoint 2 from %s to %s via PwRPC...", initial_val_ep2, new_val_ep2)
            self.write_boolean_state_value_pwrpc(device, endpoint=2, value=new_val_ep2)

            # Assert Endpoint 2 changed
            current_val_ep2 = await self.read_boolean_state_value(endpoint=2)
            asserts.assert_equal(current_val_ep2, new_val_ep2, "Endpoint 2 state did not toggle successfully!")

            # Assert Endpoint 1 remained intact
            current_val_ep1 = await self.read_boolean_state_value(endpoint=1)
            asserts.assert_equal(current_val_ep1, new_val_ep1, "Endpoint 1 state was unexpectedly modified!")

            logger.info("Successfully toggled and verified independent state changes on endpoints 1 and 2!")


if __name__ == "__main__":
    default_matter_test_main()
