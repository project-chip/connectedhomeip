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

# This test requires a TH_SERVER application that returns UnsupportedAttribute
# when reading UniqueID from BasicInformation Cluster. Please specify the app
# location with --string-arg th_server_app_path:<path_to_app>

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs: run1
# test-runner-run/run1/app: examples/fabric-admin/scripts/fabric-sync-app.py
# test-runner-run/run1/app-args: --stdin-pipe=dut-fsa/stdin --storage-dir=dut-fsa --discriminator=1234
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --string-arg th_server_app_path:${LIGHTING_APP_NO_UNIQUE_ID} dut_fsa_stdin_pipe:dut-fsa/stdin
# test-runner-run/run1/script-start-delay: 10
# test-runner-run/run1/quiet: false
# === END CI TEST ARGUMENTS ===

import asyncio
import logging
import os
import random
import subprocess
import sys
import tempfile
import threading
import uuid

import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from chip.interaction_model import Status
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main, type_matches
from mobly import asserts


class ThreadWithStop(threading.Thread):

    def __init__(self, args: list = [], stdout_cb=None, tag="", **kw):
        super().__init__(**kw)
        self.tag = f"[{tag}] " if tag else ""
        self.start_event = threading.Event()
        self.stop_event = threading.Event()
        self.stdout_cb = stdout_cb
        self.args = args

    def forward_stdout(self, f):
        while True:
            line = f.readline()
            if not line:
                break
            sys.stdout.write(f"{self.tag}{line}")
            if self.stdout_cb is not None:
                self.stdout_cb(line)

    def forward_stderr(self, f):
        while True:
            line = f.readline()
            if not line:
                break
            sys.stderr.write(f"{self.tag}{line}")

    def run(self):
        logging.info("RUN: %s", " ".join(self.args))
        self.p = subprocess.Popen(self.args, errors="ignore", stdin=subprocess.PIPE,
                                  stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        self.start_event.set()
        # Feed stdout and stderr to console and given callback.
        t1 = threading.Thread(target=self.forward_stdout, args=[self.p.stdout])
        t1.start()
        t2 = threading.Thread(target=self.forward_stderr, args=[self.p.stderr])
        t2.start()
        # Wait for the stop event.
        self.stop_event.wait()
        self.p.terminate()
        t1.join()
        t2.join()

    def start(self):
        super().start()
        self.start_event.wait()

    def stop(self):
        self.stop_event.set()
        self.join()


class FabricSyncApp:

    APP_PATH = "examples/fabric-admin/scripts/fabric-sync-app.py"

    def _process_admin_output(self, line):
        if self.wait_for_text_text is not None and self.wait_for_text_text in line:
            self.wait_for_text_event.set()

    def wait_for_text(self, timeout=30):
        if not self.wait_for_text_event.wait(timeout=timeout):
            raise Exception(f"Timeout waiting for text: {self.wait_for_text_text}")
        self.wait_for_text_event.clear()
        self.wait_for_text_text = None

    def __init__(self, storageDir, fabricName=None, nodeId=None, vendorId=None, paaTrustStorePath=None,
                 bridgePort=None, bridgeDiscriminator=None, bridgePasscode=None):

        self.wait_for_text_event = threading.Event()
        self.wait_for_text_text = None

        args = [self.APP_PATH]
        # Override default ports, so it will be possible to run
        # our TH_FSA alongside the DUT_FSA during CI testing.
        args.append("--app-admin-rpc-port=44000")
        args.append("--app-bridge-rpc-port=44001")
        # Keep the storage directory in a temporary location.
        args.append(f"--storage-dir={storageDir}")
        # FIXME: Passing custom PAA store breaks something
        # if paaTrustStorePath is not None:
        #     args.append(f"--paa-trust-store-path={paaTrustStorePath}")
        if fabricName is not None:
            args.append(f"--commissioner-name={fabricName}")
        if nodeId is not None:
            args.append(f"--commissioner-node-id={nodeId}")
        args.append(f"--commissioner-vendor-id={vendorId}")
        args.append(f"--secured-device-port={bridgePort}")
        args.append(f"--discriminator={bridgeDiscriminator}")
        args.append(f"--passcode={bridgePasscode}")

        self.admin = ThreadWithStop(args, self._process_admin_output)
        self.wait_for_text_text = "Connected to Fabric-Bridge"
        self.admin.start()

        # Wait for the bridge to connect to the admin.
        self.wait_for_text()

    def CommissionOnNetwork(self, nodeId, setupPinCode=None, filterType=None, filter=None):
        self.wait_for_text_text = f"Commissioning complete for node ID 0x{nodeId:016x}: success"
        # Send the commissioning command to the admin.
        self.admin.p.stdin.write(f"pairing onnetwork {nodeId} {setupPinCode}\n")
        self.admin.p.stdin.flush()
        # Wait for success message.
        self.wait_for_text()

    def stop(self):
        self.admin.stop()


class AppServer:

    def __init__(self, app, storageDir, port=None, discriminator=None, passcode=None):

        args = [app]
        args.extend(["--KVS", tempfile.mkstemp(dir=storageDir, prefix="kvs-app-")[1]])
        args.extend(['--secured-device-port', str(port)])
        args.extend(["--discriminator", str(discriminator)])
        args.extend(["--passcode", str(passcode)])
        self.app = ThreadWithStop(args, tag="APP")
        self.app.start()

    def stop(self):
        self.app.stop()


class TC_MCORE_FS_1_3(MatterBaseTest):

    def setup_class(self):
        super().setup_class()

        # Get the path to the TH_FSA (fabric-admin and fabric-bridge) app from
        # the user params or use the default path.
        FabricSyncApp.APP_PATH = self.user_params.get("th_fsa_app_path", FabricSyncApp.APP_PATH)
        if not os.path.exists(FabricSyncApp.APP_PATH):
            asserts.fail("This test requires a TH_FSA app. Specify app path with --string-arg th_fsa_app_path:<path_to_app>")

        # Get the path to the TH_SERVER app from the user params.
        th_server_app = self.user_params.get("th_server_app_path", None)
        if not th_server_app:
            asserts.fail("This test requires a TH_SERVER app. Specify app path with --string-arg th_server_app_path:<path_to_app>")
        if not os.path.exists(th_server_app):
            asserts.fail(f"The path {th_server_app} does not exist")

        # Create a temporary storage directory for keeping KVS files.
        self.storage = tempfile.TemporaryDirectory(prefix=self.__class__.__name__)
        logging.info("Temporary storage directory: %s", self.storage.name)

        self.th_fsa_bridge_address = "::1"
        self.th_fsa_bridge_port = 5543
        self.th_fsa_bridge_discriminator = random.randint(0, 4095)
        self.th_fsa_bridge_passcode = 20202021

        self.th_fsa_controller = FabricSyncApp(
            storageDir=self.storage.name,
            paaTrustStorePath=self.matter_test_config.paa_trust_store_path,
            bridgePort=self.th_fsa_bridge_port,
            bridgeDiscriminator=self.th_fsa_bridge_discriminator,
            bridgePasscode=self.th_fsa_bridge_passcode,
            vendorId=0xFFF1)

        # Get the named pipe path for the DUT_FSA app input from the user params.
        dut_fsa_stdin_pipe = self.user_params.get("dut_fsa_stdin_pipe", None)
        if dut_fsa_stdin_pipe is not None:
            self.dut_fsa_stdin = open(dut_fsa_stdin_pipe, "w")

        self.server_for_dut_port = 5544
        self.server_for_dut_discriminator = random.randint(0, 4095)
        self.server_for_dut_passcode = 20202021

        self.server_for_th_port = 5545
        self.server_for_th_discriminator = random.randint(0, 4095)
        self.server_for_th_passcode = 20202021

        # Start the TH_SERVER_FOR_TH_FSA app.
        self.server_for_th = AppServer(
            th_server_app,
            storageDir=self.storage.name,
            port=self.server_for_th_port,
            discriminator=self.server_for_th_discriminator,
            passcode=self.server_for_th_passcode)

    def teardown_class(self):
        self.th_fsa_controller.stop()
        self.server_for_th.stop()
        self.storage.cleanup()
        super().teardown_class()

    def steps_TC_MCORE_FS_1_3(self) -> list[TestStep]:
        return [
            TestStep(0, "Commission DUT if not done", is_commissioning=True),
            TestStep(1, "DUT_FSA commissions TH_SERVER_FOR_DUT_FSA to DUT_FSA's fabric and generates a UniqueID"),
            TestStep(2, "TH instructs TH_FSA to commission TH_SERVER_FOR_TH_FSA to TH_FSA's fabric"),
            TestStep(3, "TH instructs TH_FSA to open up commissioning window on it's aggregator"),
            TestStep(4, "Follow manufacturer provided instructions to have DUT_FSA commission TH_FSA's aggregator"),
            TestStep(5, "Follow manufacturer provided instructions to enable DUT_FSA to synchronize TH_SERVER_FOR_TH_FSA from TH_FSA onto DUT_FSA's fabric. TH to provide endpoint saved from step 2 in user prompt"),
            TestStep(6, "DUT_FSA synchronizes TH_SERVER_FOR_TH_FSA onto DUT_FSA's fabric and copies the UniqueID presented by TH_FSA's Bridged Device Basic Information Cluster"),
        ]

    async def commission_via_commissioner_control(self, controller_node_id: int, device_node_id: int):
        """Commission device_node_id to controller_node_id using CommissionerControl cluster."""

        request_id = random.randint(0, 0xFFFFFFFFFFFFFFFF)

        vendor_id = await self.read_single_attribute_check_success(
            node_id=device_node_id,
            cluster=Clusters.BasicInformation,
            attribute=Clusters.BasicInformation.Attributes.VendorID,
        )

        product_id = await self.read_single_attribute_check_success(
            node_id=device_node_id,
            cluster=Clusters.BasicInformation,
            attribute=Clusters.BasicInformation.Attributes.ProductID,
        )

        await self.send_single_cmd(
            node_id=controller_node_id,
            cmd=Clusters.CommissionerControl.Commands.RequestCommissioningApproval(
                requestId=request_id,
                vendorId=vendor_id,
                productId=product_id,
            ),
        )

        if not self.is_ci:
            self.wait_for_user_input("Approve Commissioning Approval Request on DUT using manufacturer specified mechanism")

        resp = await self.send_single_cmd(
            node_id=controller_node_id,
            cmd=Clusters.CommissionerControl.Commands.CommissionNode(
                requestId=request_id,
                responseTimeoutSeconds=30,
            ),
        )

        asserts.assert_equal(type(resp), Clusters.CommissionerControl.Commands.ReverseOpenCommissioningWindow,
                             "Incorrect response type")

        await self.send_single_cmd(
            node_id=device_node_id,
            cmd=Clusters.AdministratorCommissioning.Commands.OpenCommissioningWindow(
                commissioningTimeout=3*60,
                PAKEPasscodeVerifier=resp.PAKEPasscodeVerifier,
                discriminator=resp.discriminator,
                iterations=resp.iterations,
                salt=resp.salt,
            ),
            timedRequestTimeoutMs=5000,
        )

        if not self.is_ci:
            await asyncio.sleep(30)

    @async_test_body
    async def test_TC_MCORE_FS_1_3(self):
        self.is_ci = self.check_pics('PICS_SDK_CI_ONLY')
        self.is_ci = True

        # Commissioning - done
        self.step(0)

        th_server_for_th_fsa_th_node_id = 1
        self.print_step(1, "Commissioning TH_SERVER_FOR_TH_FSA to TH fabric")
        await self.default_controller.CommissionOnNetwork(
            nodeId=th_server_for_th_fsa_th_node_id,
            setupPinCode=self.server_for_th_passcode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=self.server_for_th_discriminator,
        )

        self.print_step(2, "Verify that TH_SERVER_FOR_TH_FSA does not have a UniqueID")
        # FIXME: Sometimes reading the UniqueID fails...
        await self.read_single_attribute_expect_error(
            cluster=Clusters.BasicInformation,
            attribute=Clusters.BasicInformation.Attributes.UniqueID,
            node_id=th_server_for_th_fsa_th_node_id,
            error=Status.UnsupportedAttribute,
        )

        # Get the list of endpoints on the DUT_FSA_BRIDGE before adding the TH_SERVER_FOR_TH_FSA.
        dut_fsa_bridge_endpoints = set(await self.read_single_attribute_check_success(
            cluster=Clusters.Descriptor,
            attribute=Clusters.Descriptor.Attributes.PartsList,
            endpoint=0,
        ))

        self.print_step(3, "Commissioning TH_SERVER_FOR_TH_FSA to DUT_FSA fabric")
        await self.commission_via_commissioner_control(
            controller_node_id=self.dut_node_id,
            device_node_id=th_server_for_th_fsa_th_node_id)

        # Wait for the device to appear on the DUT_FSA_BRIDGE.
        await asyncio.sleep(2)

        # Get the list of endpoints on the DUT_FSA_BRIDGE after adding the TH_SERVER_FOR_TH_FSA.
        dut_fsa_bridge_endpoints_new = set(await self.read_single_attribute_check_success(
            cluster=Clusters.Descriptor,
            attribute=Clusters.Descriptor.Attributes.PartsList,
            endpoint=0,
        ))

        # Get the endpoint number for just added TH_SERVER_FOR_TH_FSA.
        logging.info("Endpoints on DUT_FSA_BRIDGE: old=%s, new=%s", dut_fsa_bridge_endpoints, dut_fsa_bridge_endpoints_new)
        asserts.assert_true(dut_fsa_bridge_endpoints_new.issuperset(dut_fsa_bridge_endpoints),
                            "Expected only new endpoints to be added")
        unique_endpoints_set = dut_fsa_bridge_endpoints_new - dut_fsa_bridge_endpoints
        asserts.assert_equal(len(unique_endpoints_set), 1, "Expected only one new endpoint on DUT_FSA")
        dut_fsa_bridge_th_server_endpoint = list(unique_endpoints_set)[0]
        dut_fsa_bridge_endpoints = dut_fsa_bridge_endpoints_new

        self.print_step(4, "Verify that DUT_FSA created a UniqueID for TH_SERVER_FOR_TH_FSA")
        dut_fsa_bridge_th_server_unique_id = await self.read_single_attribute_check_success(
            cluster=Clusters.BridgedDeviceBasicInformation,
            attribute=Clusters.BridgedDeviceBasicInformation.Attributes.UniqueID,
            endpoint=dut_fsa_bridge_th_server_endpoint)
        asserts.assert_true(type_matches(dut_fsa_bridge_th_server_unique_id, str), "UniqueID should be a string")
        asserts.assert_true(dut_fsa_bridge_th_server_unique_id, "UniqueID should not be an empty string")
        logging.info("TH_SERVER_FOR_TH_FSA on TH_SERVER_BRIDGE UniqueID: %s", dut_fsa_bridge_th_server_unique_id)

        th_fsa_bridge_th_node_id = 2
        self.print_step(5, "Commissioning TH_FSA_BRIDGE to TH fabric")
        await self.default_controller.CommissionOnNetwork(
            nodeId=th_fsa_bridge_th_node_id,
            setupPinCode=self.th_fsa_bridge_passcode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=self.th_fsa_bridge_discriminator,
        )

        # Get the list of endpoints on the TH_FSA_BRIDGE before adding the TH_SERVER_FOR_TH_FSA.
        th_fsa_bridge_endpoints = set(await self.read_single_attribute_check_success(
            cluster=Clusters.Descriptor,
            attribute=Clusters.Descriptor.Attributes.PartsList,
            node_id=th_fsa_bridge_th_node_id,
            endpoint=0,
        ))

        discriminator = random.randint(0, 4095)
        self.print_step(6, "Open commissioning window on TH_SERVER_FOR_TH_FSA")
        params = await self.default_controller.OpenCommissioningWindow(
            nodeid=th_server_for_th_fsa_th_node_id,
            option=self.default_controller.CommissioningWindowPasscode.kTokenWithRandomPin,
            discriminator=discriminator,
            iteration=10000,
            timeout=600)

        # FIXME: Sometimes the commissioning does not work with the error:
        # > Failed to verify peer's MAC. This can happen when setup code is incorrect.
        # However, the setup code is correct... so we need to investigate why this is happening.
        # The sleep(2) seems to help, though.
        await asyncio.sleep(2)

        th_server_for_th_fsa_th_fsa_node_id = 3
        self.print_step(7, "Commissioning TH_SERVER_FOR_TH_FSA to TH_FSA")
        self.th_fsa_controller.CommissionOnNetwork(
            nodeId=th_server_for_th_fsa_th_fsa_node_id,
            setupPinCode=params.setupPinCode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=discriminator,
        )

        # Wait some time, so the dynamic endpoint will appear on the TH_FSA_BRIDGE.
        await asyncio.sleep(2)

        # Get the list of endpoints on the TH_FSA_BRIDGE after adding the TH_SERVER_FOR_TH_FSA.
        th_fsa_bridge_endpoints_new = set(await self.read_single_attribute_check_success(
            cluster=Clusters.Descriptor,
            attribute=Clusters.Descriptor.Attributes.PartsList,
            node_id=th_fsa_bridge_th_node_id,
            endpoint=0,
        ))

        # Get the endpoint number for just added TH_SERVER_FOR_TH_FSA.
        logging.info("Endpoints on TH_FSA_BRIDGE: old=%s, new=%s", th_fsa_bridge_endpoints, th_fsa_bridge_endpoints_new)
        asserts.assert_true(th_fsa_bridge_endpoints_new.issuperset(th_fsa_bridge_endpoints),
                            "Expected only new endpoints to be added")
        unique_endpoints_set = th_fsa_bridge_endpoints_new - th_fsa_bridge_endpoints
        asserts.assert_equal(len(unique_endpoints_set), 1, "Expected only one new endpoint")
        th_fsa_bridge_th_server_endpoint = list(unique_endpoints_set)[0]

        self.print_step(8, "Verify that TH_FSA created a UniqueID for TH_SERVER_FOR_TH_FSA")
        th_fsa_bridge_th_server_unique_id = await self.read_single_attribute_check_success(
            cluster=Clusters.BridgedDeviceBasicInformation,
            attribute=Clusters.BridgedDeviceBasicInformation.Attributes.UniqueID,
            node_id=th_fsa_bridge_th_node_id,
            endpoint=th_fsa_bridge_th_server_endpoint)
        asserts.assert_true(type_matches(th_fsa_bridge_th_server_unique_id, str), "UniqueID should be a string")
        asserts.assert_true(th_fsa_bridge_th_server_unique_id, "UniqueID should not be an empty string")
        logging.info("TH_SERVER_FOR_TH_FSA on TH_SERVER_BRIDGE UniqueID: %s", th_fsa_bridge_th_server_unique_id)

        # Make sure that the UniqueID on the TH_FSA_BRIDGE is different from the one on the DUT_FSA_BRIDGE.
        asserts.assert_not_equal(dut_fsa_bridge_th_server_unique_id, th_fsa_bridge_th_server_unique_id,
                                 "UniqueID on DUT_FSA_BRIDGE and TH_FSA_BRIDGE should be different")

        discriminator = random.randint(0, 4095)
        self.print_step(9, "Open commissioning window on TH_FSA_BRIDGE")
        params = await self.default_controller.OpenCommissioningWindow(
            nodeid=th_fsa_bridge_th_node_id,
            option=self.default_controller.CommissioningWindowPasscode.kTokenWithRandomPin,
            discriminator=discriminator,
            iteration=10000,
            timeout=600)

        self.print_step(9, "Commissioning TH_FSA_BRIDGE to DUT_FSA fabric")
        if not self.is_ci:
            self.wait_for_user_input(
                f"Commission TH Fabric-Sync Bridge on DUT using manufacturer specified mechanism.\n"
                f"Use the following parameters:\n"
                f"- discriminator: {discriminator}\n"
                f"- setupPinCode: {params.setupPinCode}\n"
                f"- setupQRCode: {params.setupQRCode}\n"
                f"- setupManualCode: {params.setupManualCode}\n"
                f"If using FabricSync Admin, you may type:\n"
                f">>> fabricsync add-bridge <desired_node_id> {params.setupPinCode} <th_host_ip> {self.th_fsa_bridge_port}")
        else:
            self.dut_fsa_stdin.write(
                f"fabricsync add-bridge 10 {params.setupPinCode} {self.th_fsa_bridge_address} {self.th_fsa_bridge_port}\n")
            self.dut_fsa_stdin.flush()
            # Wait for the commissioning to complete.
            await asyncio.sleep(5)

        await asyncio.sleep(10)

        return

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
