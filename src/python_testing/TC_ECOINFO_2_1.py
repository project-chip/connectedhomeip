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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: examples/fabric-admin/scripts/fabric-sync-app.py
#     app-args: --app-admin=${FABRIC_ADMIN_APP} --app-bridge=${FABRIC_BRIDGE_APP} --stdin-pipe=dut-fsa-stdin --discriminator=1234
#     app-ready-pattern: "Successfully opened pairing window on the device"
#     script-args: >
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --string-arg th_server_app_path:${ALL_CLUSTERS_APP} dut_fsa_stdin_pipe:dut-fsa-stdin
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factoryreset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio
import logging
import os
import random
import tempfile

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.interaction_model import Status
from chip.tlv import uint
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main, type_matches
from mobly import asserts
from TC_MCORE_FS_1_1 import AppServer


class TC_ECOINFO_2_1(MatterBaseTest):

    @async_test_body
    async def setup_class(self):
        super().setup_class()

        self.th_server = None
        self.storage = None

        if self.is_pics_sdk_ci_only:
            await self._setup_ci_prerequisites()

    def teardown_class(self):
        if self.th_server is not None:
            self.th_server.terminate()
        if self.storage is not None:
            self.storage.cleanup()
        super().teardown_class()

    async def _setup_ci_prerequisites(self):
        asserts.assert_true(self.is_pics_sdk_ci_only, "This method is only for PICS SDK CI")

        th_server_app = self.user_params.get("th_server_app_path", None)
        if not th_server_app:
            asserts.fail("CI setup requires a TH_SERVER app. Specify app path with --string-arg th_server_app_path:<path_to_app>")
        if not os.path.exists(th_server_app):
            asserts.fail(f"The path {th_server_app} does not exist")

        # Get the named pipe path for the DUT_FSA app input from the user params.
        dut_fsa_stdin_pipe = self.user_params.get("dut_fsa_stdin_pipe")
        if not dut_fsa_stdin_pipe:
            asserts.fail("CI setup requires --string-arg dut_fsa_stdin_pipe:<path_to_pipe>")
        self.dut_fsa_stdin = open(dut_fsa_stdin_pipe, "w")

        # Create a temporary storage directory for keeping KVS files.
        self.storage = tempfile.TemporaryDirectory(prefix=self.__class__.__name__)
        logging.info("Temporary storage directory: %s", self.storage.name)

        self.th_server_port = 5544
        self.th_server_discriminator = random.randint(0, 4095)
        self.th_server_passcode = 20202021

        # Start the server app.
        self.th_server = AppServer(
            th_server_app,
            storage_dir=self.storage.name,
            port=self.th_server_port,
            discriminator=self.th_server_discriminator,
            passcode=self.th_server_passcode)
        self.th_server.start()

        # Add some server to the DUT_FSA's Aggregator/Bridge.
        self.dut_fsa_stdin.write(f"pairing onnetwork 2 {self.th_server_passcode}\n")
        self.dut_fsa_stdin.flush()
        # Wait for the commissioning to complete.
        await asyncio.sleep(5)

    def _validate_device_directory(self, current_fabric_index, device_directory):
        for device in device_directory:
            if current_fabric_index != device.fabricIndex:
                # Fabric sensitive field still exist in python, just that they have default values
                asserts.assert_equal(device.deviceName, None, "Unexpected value in deviceName")
                asserts.assert_equal(device.deviceNameLastEdit, None, "Unexpected value in deviceNameLastEdit")
                asserts.assert_equal(device.bridgedEndpoint, 0, "Unexpected value in bridgedEndpoint")
                asserts.assert_equal(device.originalEndpoint, 0, "Unexpected value in originalEndpoint")
                asserts.assert_true(type_matches(device.deviceTypes, list), "DeviceTypes should be a list")
                asserts.assert_equal(len(device.deviceTypes), 0, "DeviceTypes list should be empty")
                asserts.assert_true(type_matches(device.uniqueLocationIDs, list), "UniqueLocationIds should be a list")
                asserts.assert_equal(len(device.uniqueLocationIDs), 0, "uniqueLocationIDs list should be empty")
                asserts.assert_equal(device.uniqueLocationIDsLastEdit, 0, "Unexpected value in uniqueLocationIDsLastEdit")
                continue

            if device.deviceName is not None:
                asserts.assert_true(type_matches(device.deviceName, str), "DeviceName should be a string")
                asserts.assert_less_equal(len(device.deviceName), 64, "DeviceName should be <= 64")
                asserts.assert_true(type_matches(device.deviceNameLastEdit, uint), "DeviceNameLastEdit should be a uint")
                asserts.assert_greater(device.deviceNameLastEdit, 0, "DeviceNameLastEdit must be greater than 0")
            else:
                asserts.assert_true(device.deviceNameLastEdit is None,
                                    "DeviceNameLastEdit should not be provided when there is no DeviceName")

            asserts.assert_true(type_matches(device.bridgedEndpoint, uint), "BridgedEndpoint should be a uint")
            asserts.assert_greater_equal(device.bridgedEndpoint, 0, "BridgedEndpoint >= 0")
            asserts.assert_less_equal(device.bridgedEndpoint, 0xffff_ffff,
                                      "BridgedEndpoint less than or equal to Invalid Endpoint value")

            asserts.assert_true(type_matches(device.originalEndpoint, uint), "OriginalEndpoint should be a uint")
            asserts.assert_greater_equal(device.originalEndpoint, 0, "OriginalEndpoint >= 0")
            asserts.assert_less(device.originalEndpoint, 0xffff_ffff,
                                "OriginalEndpoint less than or equal to Invalid Endpoint value")

            asserts.assert_true(type_matches(device.deviceTypes, list), "DeviceTypes should be a list")
            asserts.assert_greater_equal(len(device.deviceTypes), 1, "DeviceTypes list must contains at least one entry")
            for device_type in device.deviceTypes:
                asserts.assert_true(type_matches(device_type.deviceType, uint), "DeviceType should be a uint")
                # TODO what other validation can we do here to device_type.deviceType
                asserts.assert_true(type_matches(device_type.revision, uint), "device type's revision should be a uint")
                asserts.assert_greater_equal(device_type.revision, 1, "device type's revision must >= 1")

            asserts.assert_true(type_matches(device.uniqueLocationIDs, list), "UniqueLocationIds should be a list")
            num_of_unique_location_ids = len(device.uniqueLocationIDs)
            asserts.assert_less_equal(num_of_unique_location_ids, 64, "UniqueLocationIds list should be <= 64")
            for location_id in device.uniqueLocationIDs:
                asserts.assert_true(type_matches(location_id, str), "UniqueLocationId should be a string")
                location_id_string_length = len(location_id)
                asserts.assert_greater_equal(location_id_string_length, 1,
                                             "UniqueLocationId must contain at least one character")
                asserts.assert_less_equal(location_id_string_length, 64, "UniqueLocationId should be <= 64")

            asserts.assert_true(type_matches(device.uniqueLocationIDsLastEdit, uint),
                                "UniqueLocationIdsLastEdit should be a uint")
            if num_of_unique_location_ids:
                asserts.assert_greater(device.uniqueLocationIDsLastEdit, 0, "UniqueLocationIdsLastEdit must be non-zero")

    def _validate_location_directory(self, current_fabric_index, location_directory):
        for location in location_directory:
            if current_fabric_index != location.fabricIndex:
                # Fabric sensitive field still exist in python, just that they have default values
                asserts.assert_equal(location.uniqueLocationID, "", "Unexpected value in uniqueLocationID")
                asserts.assert_equal(location.locationDescriptor.locationName, "",
                                     "Unexpected value in locationDescriptor.locationName")
                asserts.assert_equal(location.locationDescriptor.floorNumber, NullValue,
                                     "Unexpected value in locationDescriptor.floorNumber")
                asserts.assert_equal(location.locationDescriptor.areaType, NullValue,
                                     "Unexpected value in locationDescriptor.areaType")
                asserts.assert_equal(location.locationDescriptorLastEdit, 0, "Unexpected value in locationDescriptorLastEdit")
                continue

            asserts.assert_true(type_matches(location.uniqueLocationID, str), "UniqueLocationId should be a string")
            location_id_string_length = len(location.uniqueLocationID)
            asserts.assert_greater_equal(location_id_string_length, 1,
                                         "UniqueLocationId must contain at least one character")
            asserts.assert_less_equal(location_id_string_length, 64, "UniqueLocationId should be <= 64")

            asserts.assert_true(type_matches(location.locationDescriptor.locationName, str),
                                "LocationName should be a string")
            asserts.assert_less_equal(len(location.locationDescriptor.locationName), 64, "LocationName should be <= 64")

            if location.locationDescriptor.floorNumber is not NullValue:
                asserts.assert_true(type_matches(location.locationDescriptor.floorNumber, int),
                                    "FloorNumber should be an int")
                # TODO check in range of int16.

            if location.locationDescriptor.areaType is not NullValue:
                # TODO check areaType is valid.
                pass

            asserts.assert_true(type_matches(location.locationDescriptorLastEdit, uint),
                                "UniqueLocationIdsLastEdit should be a uint")
            asserts.assert_greater(location.locationDescriptorLastEdit, 0, "LocationDescriptorLastEdit must be non-zero")

    def steps_TC_ECOINFO_2_1(self) -> list[TestStep]:
        return [
            TestStep(0, "Commission DUT if not done", is_commissioning=True),
            TestStep(1, "Identify endpoints with Ecosystem Information Cluster"),
            TestStep(2, "Reading DeviceDirectory Attribute"),
            TestStep(3, "Reading LocationDirectory Attribute"),
            TestStep(4, "Try Writing to DeviceDirectory Attribute"),
            TestStep(5, "Try Writing to LocationDirectory Attribute"),
            TestStep(6, "Repeating steps 2 to 5 for each endpoint identified in step 1"),
        ]

    @async_test_body
    async def test_TC_ECOINFO_2_1(self):
        dev_ctrl = self.default_controller
        dut_node_id = self.dut_node_id

        # Commissioning - done
        self.step(0)

        if not self.is_pics_sdk_ci_only:
            self.wait_for_user_input(
                "Paused test to allow for manufacturer to satisfy precondition where "
                "one or more bridged devices of a supported type is connected to DUT")

        current_fabric_index = await self.read_single_attribute_check_success(cluster=Clusters.OperationalCredentials, attribute=Clusters.OperationalCredentials.Attributes.CurrentFabricIndex)
        self.step(1)
        endpoint_wild_card_read = await dev_ctrl.ReadAttribute(dut_node_id, [(Clusters.EcosystemInformation.Attributes.ClusterRevision)])
        list_of_endpoints = list(endpoint_wild_card_read.keys())
        asserts.assert_greater(len(list_of_endpoints), 0, "Expecting at least one endpoint with Ecosystem Information Cluster")

        for idx, cluster_endpoint in enumerate(list_of_endpoints):
            if idx == 0:
                self.step(2)
            device_directory = await self.read_single_attribute(
                dev_ctrl,
                dut_node_id,
                endpoint=cluster_endpoint,
                attribute=Clusters.EcosystemInformation.Attributes.DeviceDirectory,
                fabricFiltered=False)

            self._validate_device_directory(current_fabric_index, device_directory)

            if idx == 0:
                self.step(3)
            location_directory = await self.read_single_attribute(
                dev_ctrl,
                dut_node_id,
                endpoint=cluster_endpoint,
                attribute=Clusters.EcosystemInformation.Attributes.LocationDirectory,
                fabricFiltered=False)

            self._validate_location_directory(current_fabric_index, location_directory)

            if idx == 0:
                self.step(4)
            result = await dev_ctrl.WriteAttribute(dut_node_id, [(cluster_endpoint, Clusters.EcosystemInformation.Attributes.DeviceDirectory([]))])
            asserts.assert_equal(len(result), 1, "Expecting only one result from trying to write to DeviceDirectory Attribute")
            asserts.assert_equal(result[0].Status, Status.UnsupportedWrite, "Expecting Status of UnsupportedWrite")

            if idx == 0:
                self.step(5)
            result = await dev_ctrl.WriteAttribute(dut_node_id, [(cluster_endpoint, Clusters.EcosystemInformation.Attributes.DeviceDirectory([]))])
            asserts.assert_equal(len(result), 1, "Expecting only one result from trying to write to LocationDirectory Attribute")
            asserts.assert_equal(result[0].Status, Status.UnsupportedWrite, "Expecting Status of UnsupportedWrite")

            if idx == 0:
                self.step(6)


if __name__ == "__main__":
    default_matter_test_main()
