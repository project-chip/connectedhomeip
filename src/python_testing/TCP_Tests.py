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
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===
#
import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from chip.interaction_model import InteractionModelError
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TCP_Tests(MatterBaseTest):
    async def send_arm_cmd(self, payloadCapability: ChipDeviceCtrl.TransportPayloadCapability) -> None:
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=900, breadcrumb=1)
        await self.send_single_cmd(cmd=cmd, endpoint=0, payloadCapability=payloadCapability)

    @async_test_body
    async def teardown_test(self):
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=0, breadcrumb=0)
        await self.send_single_cmd(cmd=cmd, endpoint=0,
                                   payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)

    def pics_TC_SC_8_1(self):
        return ['MCORE.SC.TCP']

    def steps_TC_SC_8_1(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH initiates a CASE session establishment with DUT, requesting a session supporting large payloads."),
            TestStep(3, "Verifying that a session is set up with an underlying TCP connection established with DUT."),
        ]
        return steps

    # TCP Connection Establishment
    @async_test_body
    async def test_TC_SC_8_1(self):

        self.step(1)
        try:
            self.step(2)
            device = await self.default_controller.GetConnectedDevice(nodeid=self.dut_node_id, allowPASE=False, timeoutMs=1000,
                                                                      payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
        except TimeoutError:
            asserts.fail("Unable to establish a CASE session over TCP to the device")

        self.step(3)
        asserts.assert_equal(device.isSessionOverTCPConnection, True, "Session does not have associated TCP connection")
        asserts.assert_equal(device.isActiveSession, True, "Large Payload Session should be active over TCP connection")

    def pics_TC_SC_8_2(self):
        return ['MCORE.SC.TCP']

    def steps_TC_SC_8_2(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH initiates a CASE session establishment with DUT, requesting a session supporting large payloads."),
            TestStep(3, "Verifying that the session established with DUT allows large payloads."),
        ]
        return steps

    # Large Payload Session Establishment
    @async_test_body
    async def test_TC_SC_8_2(self):

        self.step(1)
        try:
            self.step(2)
            device = await self.default_controller.GetConnectedDevice(nodeid=self.dut_node_id, allowPASE=False, timeoutMs=1000,
                                                                      payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
        except TimeoutError:
            asserts.fail("Unable to establish a CASE session over TCP to the device")

        self.step(3)
        asserts.assert_equal(device.sessionAllowsLargePayload, True, "Session does not have associated TCP connection")

    def pics_TC_SC_8_3(self):
        return ['MCORE.SC.TCP']

    def steps_TC_SC_8_3(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH initiates a CASE session establishment with DUT, requesting a session supporting large payloads."),
            TestStep(3, "Verifying that a session is set up with an underlying TCP connection established with DUT."),
            TestStep(4, "TH closes the TCP connection with DUT"),
            TestStep(5, "Verifying that the secure session with DUT is inactive."),
        ]
        return steps

    # Session Inactive After TCP Disconnect
    @async_test_body
    async def test_TC_SC_8_3(self):

        self.step(1)
        try:
            self.step(2)
            device = await self.default_controller.GetConnectedDevice(nodeid=self.dut_node_id, allowPASE=False, timeoutMs=1000,
                                                                      payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
        except TimeoutError:
            asserts.fail("Unable to establish a CASE session over TCP to the device")

        self.step(3)
        asserts.assert_equal(device.isSessionOverTCPConnection, True, "Session does not have associated TCP connection")

        self.step(4)
        device.closeTCPConnectionWithPeer()

        self.step(5)
        asserts.assert_equal(device.isActiveSession, False,
                             "Large Payload Session should not be active after TCP connection closure")

    def pics_TC_SC_8_4(self):
        return ['MCORE.SC.TCP']

    def steps_TC_SC_8_4(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH initiates a CASE session establishment with DUT, requesting a session supporting large payloads."),
            TestStep(3, "Verifying that a session is set up with an underlying TCP connection established with DUT."),
            TestStep(4, "TH closes the TCP connection with DUT"),
            TestStep(5, "Verifyng that the secure session with DUT is inactive."),
            TestStep(6, "TH re-initiates CASE session establishment over TCP with DUT"),
            TestStep(7, "Verifying that a session is set up with an underlying TCP connection established with DUT."),
            TestStep(8, "Verifying that the large-payload secure session with DUT is active."),
        ]
        return steps

    # TCP Connect, Disconnect, Then Connect Again
    @async_test_body
    async def test_TC_SC_8_4(self):

        self.step(1)
        try:
            self.step(2)
            device = await self.default_controller.GetConnectedDevice(nodeid=self.dut_node_id, allowPASE=False, timeoutMs=1000,
                                                                      payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
        except TimeoutError:
            asserts.fail("Unable to establish a CASE session over TCP to the device")

        self.step(3)
        asserts.assert_equal(device.isSessionOverTCPConnection, True, "Session does not have associated TCP connection")

        self.step(4)
        device.closeTCPConnectionWithPeer()

        self.step(5)
        asserts.assert_equal(device.isActiveSession, False,
                             "Large Payload Session should not be active after TCP connection closure")

        # Connect again
        try:
            self.step(6)
            device = await self.default_controller.GetConnectedDevice(nodeid=self.dut_node_id, allowPASE=False, timeoutMs=1000,
                                                                      payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
        except TimeoutError:
            asserts.fail("Unable to establish a CASE session over TCP to the device")

        self.step(7)
        asserts.assert_equal(device.isSessionOverTCPConnection, True, "Session does not have associated TCP connection")

        self.step(8)
        asserts.assert_equal(device.isActiveSession, True, "Large Payload Session should be active over TCP connection")

    def pics_TC_SC_8_5(self):
        return ['MCORE.SC.TCP']

    def steps_TC_SC_8_5(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH initiates a CASE session establishment with DUT, requesting a session supporting large payloads."),
            TestStep(3, "Verifying that a session is set up with an underlying TCP connection established with DUT."),
            TestStep(4, "Verifying that the large-payload secure session with DUT is active."),
            TestStep(5, "TH initiates an InvokeCommandRequest with DUT over the established session."),
            TestStep(6, "Verifying successful invocation with DUT over the established session without any error."),
        ]
        return steps

    @async_test_body
    async def test_TC_SC_8_5(self):

        self.step(1)
        try:
            self.step(2)
            device = await self.default_controller.GetConnectedDevice(nodeid=self.dut_node_id, allowPASE=False, timeoutMs=1000,
                                                                      payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
        except TimeoutError:
            asserts.fail("Unable to establish a CASE session over TCP to the device")

        self.step(3)
        asserts.assert_equal(device.isSessionOverTCPConnection, True, "Session does not have associated TCP connection")

        self.step(4)
        asserts.assert_equal(device.isActiveSession, True, "Large Payload Session should be active over TCP connection")
        asserts.assert_equal(device.sessionAllowsLargePayload, True, "Session does not have associated TCP connection")

        try:
            self.step(5)
            await self.send_arm_cmd(ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
        except InteractionModelError:
            asserts.fail("Unexpected error returned by DUT")
        self.step(6)

    def pics_TC_SC_8_6(self):
        return ['MCORE.SC.TCP']

    def steps_TC_SC_8_6(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH initiates a CASE session establishment with DUT, requesting a session supporting large payloads."),
            TestStep(3, "Verifying that a session is set up with an underlying TCP connection established with DUT."),
            TestStep(4, "Verifying that the large-payload secure session with DUT is active."),
            TestStep(5, "TH initiates a Read of all attributes of all clusters of DUT."),
            TestStep(6, "Verifying wildcard read was successful with DUT over the established session without any error."),
        ]
        return steps

    # WildCard Read Over TCP Session
    @async_test_body
    async def test_TC_SC_8_6(self):

        self.step(1)
        try:
            self.step(2)
            device = await self.default_controller.GetConnectedDevice(nodeid=self.dut_node_id, allowPASE=False, timeoutMs=1000,
                                                                      payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
        except TimeoutError:
            asserts.fail("Unable to establish a CASE session over TCP to the device")

        self.step(3)
        asserts.assert_equal(device.isSessionOverTCPConnection, True, "Session does not have associated TCP connection")

        self.step(4)
        asserts.assert_equal(device.isActiveSession, True, "Large Payload Session should be active over TCP connection")
        asserts.assert_equal(device.sessionAllowsLargePayload, True, "Session does not have associated TCP connection")

        try:
            self.step(5)
            await self.default_controller.Read(self.dut_node_id, [()], payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
        except InteractionModelError:
            asserts.fail("Unexpected error returned by DUT")
        self.step(6)

    def pics_TC_SC_8_7(self):
        return ['MCORE.SC.TCP']

    def steps_TC_SC_8_7(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH initiates a CASE session establishment with DUT, requesting a session supporting large payloads."),
            TestStep(3, "Verifying that a session is set up with an underlying TCP connection established with DUT."),
            TestStep(4, "Verifying that the large-payload secure session with DUT is active."),
            TestStep(5, "TH initiates a regularly-sized InvokeCommandRequest with DUT, specifying that either a MRP or TCP-based session is usable."),
            TestStep(6, "Verifying successful invocation with DUT over the established TCP-based session without any error."),
        ]
        return steps

    # Use TCP Session If Available For MRP Interaction
    @async_test_body
    async def test_TC_SC_8_7(self):

        self.step(1)

        try:
            self.step(2)
            device = await self.default_controller.GetConnectedDevice(nodeid=self.dut_node_id, allowPASE=False, timeoutMs=1000,
                                                                      payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
        except TimeoutError:
            asserts.fail("Unable to establish a CASE session over TCP to the device")

        self.step(3)
        asserts.assert_equal(device.isSessionOverTCPConnection, True, "Session does not have associated TCP connection")

        self.step(4)
        asserts.assert_equal(device.isActiveSession, True, "Large Payload Session should be active over TCP connection")
        asserts.assert_equal(device.sessionAllowsLargePayload, True, "Session does not have associated TCP connection")

        try:
            self.step(5)
            self.send_arm_cmd(ChipDeviceCtrl.TransportPayloadCapability.MRP_OR_TCP_PAYLOAD)
        except InteractionModelError:
            asserts.fail("Unexpected error returned by DUT")
        self.step(6)


if __name__ == "__main__":
    default_matter_test_main()
