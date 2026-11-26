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

# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_DEVICES_APP}
#     app-args: >
#       --device on-off-sensor:1
#       --device on-off-sensor:2
#       --discriminator 1234 --KVS kvs1
#       --app-pipe /tmp/on-off-sensor-pipe
#       --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network --discriminator 1234 --passcode 20202021
#       --endpoint 1 --app-pipe /tmp/on-off-sensor-pipe
#       --string-arg peer_app_path:${ALL_DEVICES_APP}
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio
import tempfile
from contextlib import ExitStack
from pathlib import Path

from mobly import asserts

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.interaction_model import Status
from matter.testing.apps import AppServerSubprocess
from matter.testing.decorators import async_test_body
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.matter_testing import MatterTestCommissionedDevice
from matter.testing.runner import default_matter_test_main


class TestOnOffSensor(MatterTestCommissionedDevice):
    """Exercises the sensor's mandatory clients against a separately commissioned light."""

    @async_test_body
    async def test_bound_commands(self) -> None:
        """Verify Binding isolation and observable On/Off and Identify effects on a peer."""
        peer_app = self.user_params.get("peer_app_path")
        asserts.assert_true(peer_app and Path(peer_app).is_file(), "Supply --string-arg peer_app_path:<all-devices-app>")
        endpoint = self.get_endpoint()
        peer_node = self.dut_node_id + 1
        peer_endpoint = 1
        discriminator = 1235
        passcode = 20202021
        with ExitStack() as cleanup:
            storage = cleanup.enter_context(tempfile.TemporaryDirectory(prefix="on-off-sensor-peer-"))
            peer = AppServerSubprocess(peer_app, storage, discriminator, passcode, port=5543,
                                       extra_args=["--device", "on-off-light:1"])
            cleanup.callback(peer.terminate)
            peer.start(expected_output="Server initialization complete", timeout=30)
            await self.default_controller.CommissionOnNetwork(
                nodeId=peer_node, setupPinCode=passcode,
                filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=discriminator)

            self.step(1, "Allow the sensor's operational node to operate and identify the peer light")
            acl = await self.read_single_attribute_check_success(
                Clusters.AccessControl, Clusters.AccessControl.Attributes.Acl, node_id=peer_node, endpoint=0)
            acl.append(Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kManage,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[self.dut_node_id],
                targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(endpoint=peer_endpoint)]))
            result = await self.default_controller.WriteAttribute(
                peer_node, [(0, Clusters.AccessControl.Attributes.Acl(acl))])
            asserts.assert_equal(result[0].Status, Status.Success, "Peer ACL write failed")

            self.step(2, "Bind the sensor's On/Off and Identify clients to the peer")
            targets = [Clusters.Binding.Structs.TargetStruct(node=peer_node, endpoint=peer_endpoint, cluster=cluster.id)
                       for cluster in (Clusters.OnOff, Clusters.Identify)]
            await self.write_single_attribute(Clusters.Binding.Attributes.Binding(targets), endpoint_id=endpoint)
            binding = await self.read_single_attribute_check_success(
                Clusters.Binding, Clusters.Binding.Attributes.Binding, endpoint=endpoint)
            asserts.assert_equal([(b.node, b.endpoint, b.cluster) for b in binding],
                                 [(b.node, b.endpoint, b.cluster) for b in targets], "Binding targets differ")
            other_endpoint = 2 if endpoint == 1 else 1
            other_binding = await self.read_single_attribute_check_success(
                Clusters.Binding, Clusters.Binding.Attributes.Binding, endpoint=other_endpoint)
            asserts.assert_equal(other_binding, [], f"Sensor {endpoint}'s bindings leaked into sensor {other_endpoint}")

            self.step(3, "Trigger On, Off, and Toggle and verify peer state reports")
            await self.default_controller.SendCommand(peer_node, peer_endpoint, Clusters.OnOff.Commands.Off())
            onoff = AttributeSubscriptionHandler(Clusters.OnOff, Clusters.OnOff.Attributes.OnOff)
            await onoff.start(self.default_controller, peer_node, peer_endpoint, max_interval_sec=30)
            cleanup.callback(onoff.cancel)
            for action, expected in [("SensorOn", True), ("SensorOff", False), ("SensorToggle", True),
                                     ("SensorToggle", False)]:
                onoff.reset()
                self.write_to_app_pipe({"Name": action, "EndpointId": endpoint})
                report = await asyncio.to_thread(onoff.wait_for_attribute_report, 10)
                asserts.assert_equal(report.endpoint_id, peer_endpoint, "Report came from the wrong peer endpoint")
                asserts.assert_equal(report.value, expected, f"{action} did not update the peer")

            self.step(4, "Trigger Identify start and stop and verify peer reports")
            identify = AttributeSubscriptionHandler(Clusters.Identify, Clusters.Identify.Attributes.IdentifyTime)
            await identify.start(self.default_controller, peer_node, peer_endpoint, max_interval_sec=30)
            cleanup.callback(identify.cancel)
            for seconds in (60, 0):
                identify.reset()
                self.write_to_app_pipe({"Name": "SensorIdentify", "EndpointId": endpoint, "IdentifyTime": seconds})
                report = await asyncio.to_thread(identify.wait_for_attribute_report, 10)
                # IdentifyTime can tick before the report is encoded; match TC_I_2_4's two-second allowance.
                asserts.assert_greater_equal(report.value, max(0, seconds - 2), "IdentifyTime report is too small")
                asserts.assert_less_equal(report.value, seconds, "IdentifyTime report is too large")

            self.step(5, "Remove bindings and verify the list is empty")
            await self.write_single_attribute(Clusters.Binding.Attributes.Binding([]), endpoint_id=endpoint)
            binding = await self.read_single_attribute_check_success(
                Clusters.Binding, Clusters.Binding.Attributes.Binding, endpoint=endpoint)
            asserts.assert_equal(binding, [], "Binding removal failed")


if __name__ == "__main__":
    default_matter_test_main()
