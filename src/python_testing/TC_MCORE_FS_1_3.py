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

# This test requires a TH_SERVER application that returns UnsupportedAttribute when reading UniqueID from BasicInformation Cluster. Please specify with --string-arg th_server_app_path:<path_to_app>

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
        self.device_for_th_eco_nodeid = 1111
        self.device_for_th_eco_kvs = None
        self.device_for_th_eco_port = 5543
        self.app_process_for_th_eco = None

        self.device_for_dut_eco_nodeid = 1112
        self.device_for_dut_eco_kvs = None
        self.device_for_dut_eco_port = 5544
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
        if self.app_process_for_th_eco is not None:
            logging.warning("Stopping app with SIGTERM")
            self.app_process_for_th_eco.send_signal(signal.SIGTERM.value)
            self.app_process_for_th_eco.wait()

        os.remove(self.device_for_dut_eco_kvs)
        if self.device_for_th_eco_kvs is not None:
            os.remove(self.device_for_th_eco_kvs)
        super().teardown_class()

    async def create_device_and_commission_to_th_fabric(self, kvs, port, node_id_for_th, device_info):
        # TODO: confirm whether we can open processes like this on the TH
        app = self.user_params.get("th_server_app_path", None)
        if not app:
            asserts.fail('This test requires a TH_SERVER app. Specify app path with --string-arg th_server_app_path:<path_to_app>')

        discriminator = random.randint(0, 4095)
        passcode = 20202021
        app_args = f'--secured-device-port {port} --discriminator {discriminator} --passcode {passcode} --KVS {kvs}'
        cmd = f'{app} {app_args}'
        # TODO: Determine if we want these logs cooked or pushed to somewhere else
        logging.info(f"Starting TH device for {device_info}")
        self.app_process_for_dut_eco = subprocess.Popen(cmd, bufsize=0, shell=True)
        logging.info(f"Started TH device for {device_info}")
        time.sleep(3)

        logging.info("Commissioning from separate fabric")
        await self.TH_server_controller.CommissionOnNetwork(nodeId=node_id_for_th, setupPinCode=passcode, filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=discriminator)
        logging.info("Commissioning device for DUT ecosystem onto TH for managing")

    async def create_and_commission_device_for_th_ecosystem(self):
        # TODO: confirm whether we can open processes like this on the TH
        app = self.user_params.get("th_server_app_path", None)

        self.device_for_th_eco_kvs = f'kvs_{str(uuid.uuid4())}'
        discriminator = random.randint(0, 4095)
        passcode = 20202021
        app_args = f'--secured-device-port {self.device_for_th_eco_port} --discriminator {discriminator} --passcode {passcode} --KVS {self.device_for_th_eco_kvs}'
        cmd = f'{app} {app_args}'
        # TODO: Determine if we want these logs cooked or pushed to somewhere else
        logging.info("Starting TH device for TH ecosystem")
        self.app_process_for_th_eco = subprocess.Popen(cmd, bufsize=0, shell=True)
        logging.info("Started TH device for TH ecosystem")
        time.sleep(3)

        logging.info("Commissioning from separate fabric")
        self.server_nodeid = 1112
        await self.TH_server_controller.CommissionOnNetwork(nodeId=self.server_nodeid, setupPinCode=passcode, filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=discriminator)
        logging.info("Commissioning TH device for TH ecosystem")

    def steps_TC_MCORE_FS_1_3(self) -> list[TestStep]:
        steps = [TestStep(1, "DUT_FSA commissions TH_SED_DUT to DUT_FSAs fabric and generates a UniqueID", is_commissioning=True),
                 TestStep(2, "TH_FSA commissions TH_SED_TH onto TH_FSAs fabric and generates a UniqueID."),
                 TestStep(3, "Follow manufacturer provided instructions to enable DUT_FSA to synchronize TH_SED_TH onto DUT_FSAs fabric."),
                 TestStep(4, "DUT_FSA synchronizes TH_SED_TH onto DUT_FSAs fabric and copies the UniqueID presented by TH_FSAs Bridged Device Basic Information Cluster.")]
        return steps

    @async_test_body
    async def test_TC_MCORE_FS_1_3(self):
        self.is_ci = self.check_pics('PICS_SDK_CI_ONLY')
        self.print_step(0, "Commissioning DUT to TH, already done")
        self.step(1)
        # These steps are not explicitly in step 1, but they help identify the dynamically added endpoint in step 1.
        root_node_endpoint = 0
        root_part_list = await self.read_single_attribute_check_success(cluster=Clusters.Descriptor, attribute=Clusters.Descriptor.Attributes.PartsList, endpoint=root_node_endpoint)
        set_of_endpoints_before_adding_device = set(root_part_list)

        kvs = f'kvs_{str(uuid.uuid4())}'
        device_info = "for DUT ecosystem"
        await self.create_device_and_commission_to_th_fabric(kvs, self.device_for_dut_eco_port, self.device_for_dut_eco_nodeid, device_info)
        self.device_for_dut_eco_kvs = kvs
        read_result = await self.TH_server_controller.ReadAttribute(self.device_for_dut_eco_nodeid, [(root_node_endpoint, Clusters.BasicInformation.Attributes.UniqueID)])
        result = read_result[root_node_endpoint][Clusters.BasicInformation][Clusters.BasicInformation.Attributes.UniqueID]
        asserts.assert_true(type_matches(result, Clusters.Attribute.ValueDecodeFailure), "We were expecting a value decode failure")
        asserts.assert_equal(result.Reason.status, Status.UnsupportedAttribute, "Incorrect error returned from reading UniqueID")

        params = await self.openCommissioningWindow(dev_ctrl=self.TH_server_controller, node_id=self.device_for_dut_eco_nodeid)

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

        asserts.assert_true(set_of_endpoints_after_adding_device.issuperset(
            set_of_endpoints_before_adding_device), "Expected only new endpoints to be added")
        unique_endpoints_set = set_of_endpoints_after_adding_device - set_of_endpoints_before_adding_device
        asserts.assert_equal(len(unique_endpoints_set), 1, "Expected only one new endpoint")
        newly_added_endpoint = list(unique_endpoints_set)[0]

        th_sed_dut_unique_id = await self.read_single_attribute_check_success(cluster=Clusters.BridgedDeviceBasicInformation, attribute=Clusters.BridgedDeviceBasicInformation.Attributes.UniqueID, endpoint=newly_added_endpoint)
        asserts.assert_true(type_matches(th_sed_dut_unique_id, str), "UniqueID should be a string")
        asserts.assert_true(th_sed_dut_unique_id, "UniqueID should not be an empty string")

        self.step(2)
        kvs = f'kvs_{str(uuid.uuid4())}'
        device_info = "for TH_FSA ecosystem"
        await self.create_device_and_commission_to_th_fabric(kvs, self.device_for_th_eco_port, self.device_for_th_eco_nodeid, device_info)
        self.device_for_th_eco_kvs = kvs
        # TODO(https://github.com/CHIP-Specifications/chip-test-plans/issues/4375) During setup we need to create the TH_FSA device
        # where we would commission device created in create_device_and_commission_to_th_fabric to be commissioned into TH_FSA.

        # TODO(https://github.com/CHIP-Specifications/chip-test-plans/issues/4375) Because we cannot create a TH_FSA and there is
        # no way to mock it the following 2 test steps are skipped for now.
        self.skip_step(3)
        self.skip_step(4)


if __name__ == "__main__":
    default_matter_test_main()
