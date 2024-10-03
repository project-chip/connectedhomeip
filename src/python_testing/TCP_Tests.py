#
#    Copyright (c) 2024 Project CHIP Authors
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
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factoryreset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===
#
import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from chip.interaction_model import InteractionModelError
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts


class TCP_Tests(MatterBaseTest):

    # TCP Connection Establishment
    @async_test_body
    async def test_TC_SC_8_1(self):

        try:
            device = await self.default_controller.GetConnectedDevice(nodeid=self.dut_node_id, allowPASE=False, timeoutMs=1000,
                                                                      payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
        except TimeoutError:
            asserts.fail("Unable to establish a CASE session over TCP to the device")
        asserts.assert_equal(device.isSessionOverTCPConnection, True, "Session does not have associated TCP connection")
        asserts.assert_equal(device.isActiveSession, True, "Large Payload Session should be active over TCP connection")

    # Large Payload Session Establishment
    @async_test_body
    async def test_TC_SC_8_2(self):

        try:
            device = await self.default_controller.GetConnectedDevice(nodeid=self.dut_node_id, allowPASE=False, timeoutMs=1000,
                                                                      payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
        except TimeoutError:
            asserts.fail("Unable to establish a CASE session over TCP to the device")
        asserts.assert_equal(device.sessionAllowsLargePayload, True, "Session does not have associated TCP connection")

    # Session Inactive After TCP Disconnect
    @async_test_body
    async def test_TC_SC_8_3(self):

        try:
            device = await self.default_controller.GetConnectedDevice(nodeid=self.dut_node_id, allowPASE=False, timeoutMs=1000,
                                                                      payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
        except TimeoutError:
            asserts.fail("Unable to establish a CASE session over TCP to the device")
        asserts.assert_equal(device.isSessionOverTCPConnection, True, "Session does not have associated TCP connection")

        device.closeTCPConnectionWithPeer()
        asserts.assert_equal(device.isActiveSession, False,
                             "Large Payload Session should not be active after TCP connection closure")

    # TCP Connect, Disconnect, Then Connect Again
    @async_test_body
    async def test_TC_SC_8_4(self):

        try:
            device = await self.default_controller.GetConnectedDevice(nodeid=self.dut_node_id, allowPASE=False, timeoutMs=1000,
                                                                      payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
        except TimeoutError:
            asserts.fail("Unable to establish a CASE session over TCP to the device")
        asserts.assert_equal(device.isSessionOverTCPConnection, True, "Session does not have associated TCP connection")

        device.closeTCPConnectionWithPeer()
        asserts.assert_equal(device.isActiveSession, False,
                             "Large Payload Session should not be active after TCP connection closure")

        # Connect again
        try:
            device = await self.default_controller.GetConnectedDevice(nodeid=self.dut_node_id, allowPASE=False, timeoutMs=1000,
                                                                      payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
        except TimeoutError:
            asserts.fail("Unable to establish a CASE session over TCP to the device")
        asserts.assert_equal(device.isSessionOverTCPConnection, True, "Session does not have associated TCP connection")
        asserts.assert_equal(device.isActiveSession, True, "Large Payload Session should be active over TCP connection")

    # OnOff Cluster Toggle Command Over TCP Session
    @async_test_body
    async def test_TC_SC_8_5(self):

        try:
            device = await self.default_controller.GetConnectedDevice(nodeid=self.dut_node_id, allowPASE=False, timeoutMs=1000,
                                                                      payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
        except TimeoutError:
            asserts.fail("Unable to establish a CASE session over TCP to the device")
        asserts.assert_equal(device.isSessionOverTCPConnection, True, "Session does not have associated TCP connection")
        asserts.assert_equal(device.isActiveSession, True, "Large Payload Session should be active over TCP connection")
        asserts.assert_equal(device.sessionAllowsLargePayload, True, "Session does not have associated TCP connection")

        commands = Clusters.Objects.OnOff.Commands
        try:
            await self.send_single_cmd(cmd=commands.Toggle(), endpoint=1,
                                       payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
        except InteractionModelError:
            asserts.fail("Unexpected error returned by DUT")

    # WildCard Read Over TCP Session
    @async_test_body
    async def test_TC_SC_8_6(self):

        try:
            device = await self.default_controller.GetConnectedDevice(nodeid=self.dut_node_id, allowPASE=False, timeoutMs=1000,
                                                                      payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
        except TimeoutError:
            asserts.fail("Unable to establish a CASE session over TCP to the device")
        asserts.assert_equal(device.isSessionOverTCPConnection, True, "Session does not have associated TCP connection")
        asserts.assert_equal(device.isActiveSession, True, "Large Payload Session should be active over TCP connection")
        asserts.assert_equal(device.sessionAllowsLargePayload, True, "Session does not have associated TCP connection")

        try:
            await self.default_controller.Read(self.dut_node_id, [()], payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
        except InteractionModelError:
            asserts.fail("Unexpected error returned by DUT")

    # Use TCP Session If Available For MRP Interaction
    @async_test_body
    async def test_TC_SC_8_7(self):

        try:
            device = await self.default_controller.GetConnectedDevice(nodeid=self.dut_node_id, allowPASE=False, timeoutMs=1000,
                                                                      payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
        except TimeoutError:
            asserts.fail("Unable to establish a CASE session over TCP to the device")
        asserts.assert_equal(device.isSessionOverTCPConnection, True, "Session does not have associated TCP connection")
        asserts.assert_equal(device.isActiveSession, True, "Large Payload Session should be active over TCP connection")
        asserts.assert_equal(device.sessionAllowsLargePayload, True, "Session does not have associated TCP connection")

        commands = Clusters.Objects.OnOff.Commands
        try:
            await self.send_single_cmd(cmd=commands.Toggle(), endpoint=1,
                                       payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.MRP_OR_TCP_PAYLOAD)
        except InteractionModelError:
            asserts.fail("Unexpected error returned by DUT")


if __name__ == "__main__":
    default_matter_test_main()
