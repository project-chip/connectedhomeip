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

# This test requires a TH_SERVER application that returns UnsupportedAttribute when reading UniqueID from BasicInformation Cluster. Please specify with --string-arg th_server_no_uid_app_path:<path_to_app>

import logging
import os
import random
import signal
import subprocess
import time
import uuid

import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from chip.interaction_model import Status
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main, type_matches
from mobly import asserts


class TC_MCORE_FS_1_3(MatterBaseTest):
    @async_test_body
    async def setup_class(self):
        super().setup_class()

        self.th_server_nodeid = 1111
        self.th_server_kvs = None
        self.th_server_port = 5543
        self.app_process_for_dut_eco = None

        # Create a second controller on a new fabric to communicate to the server
        new_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        new_fabric_admin = new_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=2)
        paa_path = str(self.matter_test_config.paa_trust_store_path)
        self.TH_server_controller = new_fabric_admin.NewController(nodeId=112233, paaTrustStorePath=paa_path)

    def teardown_class(self):
        if self.app_process_for_dut_eco is not None:
            logging.warning("Stopping app with SIGTERM")
            self.app_process_for_dut_eco.send_signal(signal.SIGTERM.value)
            self.app_process_for_dut_eco.wait()

        os.remove(self.th_server_kvs)
        super().teardown_class()

    async def create_device_and_commission_to_th_fabric(self, kvs, port, node_id_for_th, device_info):
        app = self.user_params.get("th_server_no_uid_app_path", None)
        if not app:
            asserts.fail('This test requires a TH_SERVER app. Specify app path with --string-arg th_server_no_uid_app_path:<path_to_app>')

        if not os.path.exists(app):
            asserts.fail(f'The path {app} does not exist')

        discriminator = random.randint(0, 4095)
        passcode = 20202021

        cmd = [app]
        cmd.extend(['--secured-device-port', str(port)])
        cmd.extend(['--discriminator', str(discriminator)])
        cmd.extend(['--passcode', str(passcode)])
        cmd.extend(['--KVS', kvs])

        # TODO: Determine if we want these logs cooked or pushed to somewhere else
        logging.info(f"Starting TH device for {device_info}")
        self.app_process_for_dut_eco = subprocess.Popen(cmd)
        logging.info(f"Started TH device for {device_info}")
        time.sleep(3)

        logging.info("Commissioning from separate fabric")
        await self.TH_server_controller.CommissionOnNetwork(nodeId=node_id_for_th, setupPinCode=passcode, filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=discriminator)
        logging.info("Commissioning device for DUT ecosystem onto TH for managing")

    def steps_TC_MCORE_FS_1_3(self) -> list[TestStep]:
        steps = [TestStep(1, "TH commissions TH_SERVER to TH’s fabric.", is_commissioning=True),
                 TestStep(2, "DUT_FSA commissions TH_SERVER to DUT_FSA’s fabric and generates a UniqueID.")]
        return steps

    @async_test_body
    async def test_TC_MCORE_FS_1_3(self):
        self.is_ci = self.check_pics('PICS_SDK_CI_ONLY')
        self.print_step(0, "Commissioning DUT to TH, already done")

        self.step(1)
        root_node_endpoint = 0
        root_part_list = await self.read_single_attribute_check_success(cluster=Clusters.Descriptor, attribute=Clusters.Descriptor.Attributes.PartsList, endpoint=root_node_endpoint)
        set_of_endpoints_before_adding_device = set(root_part_list)
        logging.info(f"Set of endpoints before adding the device: {set_of_endpoints_before_adding_device}")

        kvs = f'kvs_{str(uuid.uuid4())}'
        device_info = "for TH ecosystem"
        await self.create_device_and_commission_to_th_fabric(kvs, self.th_server_port, self.th_server_nodeid, device_info)

        self.th_server_kvs = kvs
        read_result = await self.TH_server_controller.ReadAttribute(self.th_server_nodeid, [(root_node_endpoint, Clusters.BasicInformation.Attributes.UniqueID)])
        result = read_result[root_node_endpoint][Clusters.BasicInformation][Clusters.BasicInformation.Attributes.UniqueID]
        asserts.assert_true(type_matches(result, Clusters.Attribute.ValueDecodeFailure), "We were expecting a value decode failure")
        asserts.assert_equal(result.Reason.status, Status.UnsupportedAttribute, "Incorrect error returned from reading UniqueID")

        self.step(2)
        params = await self.openCommissioningWindow(dev_ctrl=self.TH_server_controller, node_id=self.th_server_nodeid)

        self.wait_for_user_input(
            prompt_msg=f"Using the DUT vendor's provided interface, commission the device using the following parameters:\n"
            f"- discriminator: {params.randomDiscriminator}\n"
            f"- setupPinCode: {params.commissioningParameters.setupPinCode}\n"
            f"- setupQRCode: {params.commissioningParameters.setupQRCode}\n"
            f"- setupManualcode: {params.commissioningParameters.setupManualCode}\n"
            f"If using FabricSync Admin, you may type:\n"
            f">>> pairing onnetwork <desired_node_id> {params.commissioningParameters.setupPinCode}")

        root_part_list = await self.read_single_attribute_check_success(cluster=Clusters.Descriptor, attribute=Clusters.Descriptor.Attributes.PartsList, endpoint=root_node_endpoint)
        set_of_endpoints_after_adding_device = set(root_part_list)
        logging.info(f"Set of endpoints after adding the device: {set_of_endpoints_after_adding_device}")

        asserts.assert_true(set_of_endpoints_after_adding_device.issuperset(
            set_of_endpoints_before_adding_device), "Expected only new endpoints to be added")
        unique_endpoints_set = set_of_endpoints_after_adding_device - set_of_endpoints_before_adding_device
        asserts.assert_equal(len(unique_endpoints_set), 1, "Expected only one new endpoint")
        newly_added_endpoint = list(unique_endpoints_set)[0]

        th_sed_dut_unique_id = await self.read_single_attribute_check_success(cluster=Clusters.BridgedDeviceBasicInformation, attribute=Clusters.BridgedDeviceBasicInformation.Attributes.UniqueID, endpoint=newly_added_endpoint)
        asserts.assert_true(type_matches(th_sed_dut_unique_id, str), "UniqueID should be a string")
        asserts.assert_true(th_sed_dut_unique_id, "UniqueID should not be an empty string")


if __name__ == "__main__":
    default_matter_test_main()
