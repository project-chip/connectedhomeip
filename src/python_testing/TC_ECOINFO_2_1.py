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

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.interaction_model import Status
from chip.tlv import uint
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main, type_matches
from mobly import asserts


class TC_ECOINFO_2_1(MatterBaseTest):

    def _validate_device_directory(self, is_removed_on_null, device_directory):
        num_of_devices = len(device_directory)
        if is_removed_on_null:
            asserts.assert_less_equal(num_of_devices, 256, "Too many device entries")
            for device in device_directory:
                # TODO do fabric index check first
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
        else:
            asserts.assert_equal(num_of_devices, 0, "Device was removed, there should be no devices in DeviceDirectory")

    def _validate_location_directory(self, is_removed_on_null, location_directory):
        num_of_locations = len(location_directory)
        if is_removed_on_null:
            asserts.assert_less_equal(num_of_locations, 64, "Too many location entries")
            for location in location_directory:
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

        else:
            asserts.assert_equal(num_of_locations, 0, "Device was removed, there should be no location in LocationDirectory")

    def steps_TC_ECOINFO_2_1(self) -> list[TestStep]:
        steps = [TestStep(1, "Identify endpoints with Ecosystem Information Cluster", is_commissioning=True),
                 TestStep(2, "Reading RemovedOn Attribute"),
                 TestStep(3, "Reading DeviceDirectory Attribute"),
                 TestStep(4, "Reading LocationDirectory Attribute"),
                 TestStep(5, "Try Writing to RemovedOn Attribute"),
                 TestStep(6, "Try Writing to DeviceDirectory Attribute"),
                 TestStep(7, "Try Writing to LocationDirectory Attribute"),
                 TestStep(8, "Repeating steps 2 to 7 for each endpoint identified in step 1")]
        return steps

    @async_test_body
    async def test_TC_ECOINFO_2_1(self):
        dev_ctrl = self.default_controller
        dut_node_id = self.dut_node_id

        self.print_step(0, "Commissioning, already done")

        self.step(1)
        endpoint_wild_card_read = await dev_ctrl.ReadAttribute(dut_node_id, [(Clusters.EcosystemInformation.Attributes.ClusterRevision)])
        list_of_endpoints = list(endpoint_wild_card_read.keys())

        for idx, cluster_endpoint in enumerate(list_of_endpoints):
            if idx == 0:
                self.step(2)
            removed_on = await self.read_single_attribute(
                dev_ctrl,
                dut_node_id,
                endpoint=cluster_endpoint,
                attribute=Clusters.EcosystemInformation.Attributes.RemovedOn)

            is_removed_on_null = removed_on is NullValue
            if not is_removed_on_null:
                asserts.assert_true(type_matches(removed_on, uint))
                asserts.assert_greater(removed_on, 0, "RemovedOn must be greater than 0", "RemovedOn should be a uint")

            if idx == 0:
                self.step(3)
            device_directory = await self.read_single_attribute(
                dev_ctrl,
                dut_node_id,
                endpoint=cluster_endpoint,
                attribute=Clusters.EcosystemInformation.Attributes.DeviceDirectory,
                fabricFiltered=False)

            self._validate_device_directory(is_removed_on_null, device_directory)

            if idx == 0:
                self.step(4)
            location_directory = await self.read_single_attribute(
                dev_ctrl,
                dut_node_id,
                endpoint=cluster_endpoint,
                attribute=Clusters.EcosystemInformation.Attributes.LocationDirectory,
                fabricFiltered=False)

            self._validate_location_directory(is_removed_on_null, location_directory)

            if idx == 0:
                self.step(5)
            result = await dev_ctrl.WriteAttribute(dut_node_id, [(cluster_endpoint, Clusters.EcosystemInformation.Attributes.RemovedOn(2))])
            asserts.assert_equal(len(result), 1, "Expecting only one result from trying to write to RemovedOn Attribute")
            asserts.assert_equal(result[0].Status, Status.UnsupportedWrite, "Expecting Status of UnsupportedWrite")

            if idx == 0:
                self.step(6)
            result = await dev_ctrl.WriteAttribute(dut_node_id, [(cluster_endpoint, Clusters.EcosystemInformation.Attributes.DeviceDirectory([]))])
            asserts.assert_equal(len(result), 1, "Expecting only one result from trying to write to DeviceDirectory Attribute")
            asserts.assert_equal(result[0].Status, Status.UnsupportedWrite, "Expecting Status of UnsupportedWrite")

            if idx == 0:
                self.step(7)
            result = await dev_ctrl.WriteAttribute(dut_node_id, [(cluster_endpoint, Clusters.EcosystemInformation.Attributes.DeviceDirectory([]))])
            asserts.assert_equal(len(result), 1, "Expecting only one result from trying to write to LocationDirectory Attribute")
            asserts.assert_equal(result[0].Status, Status.UnsupportedWrite, "Expecting Status of UnsupportedWrite")

            if idx == 0:
                self.step(8)


if __name__ == "__main__":
    default_matter_test_main()
