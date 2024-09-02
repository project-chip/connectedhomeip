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

# This test requires a TH_SERVER_NO_UID application that returns UnsupportedAttribute
# when reading UniqueID from BasicInformation Cluster. Please specify the app
# location with --string-arg th_server_no_uid_app_path:<path_to_app>

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs: run1
# test-runner-run/run1/app: examples/fabric-admin/scripts/fabric-sync-app.py
# test-runner-run/run1/app-args: --app-admin=${FABRIC_ADMIN_APP} --app-bridge=${FABRIC_BRIDGE_APP} --stdin-pipe=dut-fsa-stdin --discriminator=1234
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --string-arg th_fsa_app_path:examples/fabric-admin/scripts/fabric-sync-app.py th_fsa_admin_path:${FABRIC_ADMIN_APP} th_fsa_bridge_path:${FABRIC_BRIDGE_APP} th_server_no_uid_app_path:${LIGHTING_APP_NO_UNIQUE_ID} dut_fsa_stdin_pipe:dut-fsa-stdin
# test-runner-run/run1/script-start-delay: 5
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

import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from chip.interaction_model import Status
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main, type_matches
from mobly import asserts


class Subprocess(threading.Thread):

    def __init__(self, args: list = [], stdout_cb=None, tag="", **kw):
        super().__init__(**kw)
        self.tag = f"[{tag}] " if tag else ""
        self.stdout_cb = stdout_cb
        self.args = args

    def forward_f(self, f_in, f_out):
        while True:
            line = f_in.readline()
            if not line:
                break
            f_out.write(f"{self.tag}{line}")
            f_out.flush()
            if self.stdout_cb is not None:
                self.stdout_cb(line)

    def run(self):
        logging.info("RUN: %s", " ".join(self.args))
        self.p = subprocess.Popen(self.args, errors="ignore", stdin=subprocess.PIPE,
                                  stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        # Forward stdout and stderr with a tag attached.
        t1 = threading.Thread(target=self.forward_f, args=[self.p.stdout, sys.stdout])
        t1.start()
        t2 = threading.Thread(target=self.forward_f, args=[self.p.stderr, sys.stderr])
        t2.start()
        # Wait for the process to finish.
        self.p.wait()
        t1.join()
        t2.join()

    def stop(self):
        self.p.terminate()
        self.join()


class FabricSyncApp:

    def _process_admin_output(self, line):
        if self.wait_for_text_text is not None and self.wait_for_text_text in line:
            self.wait_for_text_event.set()

    def wait_for_text(self, timeout=30):
        if not self.wait_for_text_event.wait(timeout=timeout):
            raise Exception(f"Timeout waiting for text: {self.wait_for_text_text}")
        self.wait_for_text_event.clear()
        self.wait_for_text_text = None

    def __init__(self, fabricSyncAppPath, fabricAdminAppPath, fabricBridgeAppPath,
                 storageDir, fabricName=None, nodeId=None, vendorId=None, paaTrustStorePath=None,
                 bridgePort=None, bridgeDiscriminator=None, bridgePasscode=None):

        self.wait_for_text_event = threading.Event()
        self.wait_for_text_text = None

        args = [fabricSyncAppPath]
        args.append(f"--app-admin={fabricAdminAppPath}")
        args.append(f"--app-bridge={fabricBridgeAppPath}")
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

        self.admin = Subprocess(args, stdout_cb=self._process_admin_output)
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
        self.app = Subprocess(args, tag="SERVER")
        self.app.start()

    def stop(self):
        self.app.stop()


class TC_MCORE_FS_1_4(MatterBaseTest):

    @property
    def default_timeout(self) -> int:
        # This test has some manual steps, so we need a longer timeout.
        return 200

    def setup_class(self):
        super().setup_class()

        # Get the path to the TH_FSA (fabric-admin and fabric-bridge) app from the user params.
        th_fsa_app_path = self.user_params.get("th_fsa_app_path")
        if not th_fsa_app_path:
            asserts.fail("This test requires a TH_FSA app. Specify app path with --string-arg th_fsa_app_path:<path_to_app>")
        if not os.path.exists(th_fsa_app_path):
            asserts.fail(f"The path {th_fsa_app_path} does not exist")
        th_fsa_admin_path = self.user_params.get("th_fsa_admin_path")
        if not th_fsa_admin_path:
            asserts.fail("This test requires a TH_FSA_ADMIN app. Specify app path with --string-arg th_fsa_admin_path:<path_to_app>")
        if not os.path.exists(th_fsa_admin_path):
            asserts.fail(f"The path {th_fsa_admin_path} does not exist")
        th_fsa_bridge_path = self.user_params.get("th_fsa_bridge_path")
        if not th_fsa_bridge_path:
            asserts.fail("This test requires a TH_FSA_BRIDGE app. Specify app path with --string-arg th_fsa_bridge_path:<path_to_app>")
        if not os.path.exists(th_fsa_bridge_path):
            asserts.fail(f"The path {th_fsa_bridge_path} does not exist")

        # Get the path to the TH_SERVER_NO_UID app from the user params.
        th_server_app = self.user_params.get("th_server_no_uid_app_path", None)
        if not th_server_app:
            asserts.fail("This test requires a TH_SERVER_NO_UID app. Specify app path with --string-arg th_server_no_uid_app_path:<path_to_app>")
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
            th_fsa_app_path,
            th_fsa_admin_path,
            th_fsa_bridge_path,
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

        self.th_server_port = 5544
        self.th_server_discriminator = random.randint(0, 4095)
        self.th_server_passcode = 20202021

        # Start the TH_SERVER_NO_UID app.
        self.th_server = AppServer(
            th_server_app,
            storageDir=self.storage.name,
            port=self.th_server_port,
            discriminator=self.th_server_discriminator,
            passcode=self.th_server_passcode)

    def teardown_class(self):
        self.th_fsa_controller.stop()
        self.th_server.stop()
        self.storage.cleanup()
        super().teardown_class()

    def steps_TC_MCORE_FS_1_4(self) -> list[TestStep]:
        return [
            TestStep(0, "Commission DUT if not done", is_commissioning=True),
            TestStep(1, "TH commissions TH_SERVER_NO_UID to TH's fabric.",
                     "TH verifies that the TH_SERVER_NO_UID does not provide a UniqueID."),
            TestStep(2, "TH instructs TH_FSA to commission TH_SERVER_NO_UID to TH_FSA's fabric."),
            TestStep(3, "TH instructs TH_FSA to open up commissioning window on it's aggregator."),
            TestStep(4, "Follow manufacturer provided instructions to have DUT_FSA commission TH_FSA's aggregator."),
            TestStep(5, "Follow manufacturer provided instructions to enable DUT_FSA to synchronize TH_SERVER_NO_UID"
                     " from TH_FSA onto DUT_FSA's fabric. TH to provide endpoint saved from step 2 in user prompt."),
            TestStep(6, "DUT_FSA synchronizes TH_SERVER_NO_UID onto DUT_FSA's fabric and copies the UniqueID presented"
                     " by TH_FSA's Bridged Device Basic Information Cluster."),
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

    @async_test_body
    async def test_TC_MCORE_FS_1_4(self):
        self.is_ci = self.check_pics('PICS_SDK_CI_ONLY')

        # Commissioning - done
        self.step(0)

        self.step(1)

        th_server_th_node_id = 1
        await self.default_controller.CommissionOnNetwork(
            nodeId=th_server_th_node_id,
            setupPinCode=self.th_server_passcode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=self.th_server_discriminator,
        )

        await self.read_single_attribute_expect_error(
            cluster=Clusters.BasicInformation,
            attribute=Clusters.BasicInformation.Attributes.UniqueID,
            node_id=th_server_th_node_id,
            error=Status.UnsupportedAttribute,
        )

        self.step(2)

        th_fsa_bridge_th_node_id = 2
        # Commissioning TH_FSA_BRIDGE to TH fabric.
        await self.default_controller.CommissionOnNetwork(
            nodeId=th_fsa_bridge_th_node_id,
            setupPinCode=self.th_fsa_bridge_passcode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=self.th_fsa_bridge_discriminator,
        )

        # Get the list of endpoints on the TH_FSA_BRIDGE before adding the TH_SERVER_NO_UID.
        th_fsa_bridge_endpoints = set(await self.read_single_attribute_check_success(
            cluster=Clusters.Descriptor,
            attribute=Clusters.Descriptor.Attributes.PartsList,
            node_id=th_fsa_bridge_th_node_id,
            endpoint=0,
        ))

        discriminator = random.randint(0, 4095)
        # Open commissioning window on TH_SERVER_NO_UID.
        params = await self.default_controller.OpenCommissioningWindow(
            nodeid=th_server_th_node_id,
            option=self.default_controller.CommissioningWindowPasscode.kTokenWithRandomPin,
            discriminator=discriminator,
            iteration=10000,
            timeout=600)

        th_server_th_fsa_node_id = 3
        # Commissioning TH_SERVER_NO_UID to TH_FSA.
        self.th_fsa_controller.CommissionOnNetwork(
            nodeId=th_server_th_fsa_node_id,
            setupPinCode=params.setupPinCode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=discriminator,
        )

        # Wait some time, so the dynamic endpoint will appear on the TH_FSA_BRIDGE.
        await asyncio.sleep(5)

        # Get the list of endpoints on the TH_FSA_BRIDGE after adding the TH_SERVER_NO_UID.
        th_fsa_bridge_endpoints_new = set(await self.read_single_attribute_check_success(
            cluster=Clusters.Descriptor,
            attribute=Clusters.Descriptor.Attributes.PartsList,
            node_id=th_fsa_bridge_th_node_id,
            endpoint=0,
        ))

        # Get the endpoint number for just added TH_SERVER_NO_UID.
        logging.info("Endpoints on TH_FSA_BRIDGE: old=%s, new=%s", th_fsa_bridge_endpoints, th_fsa_bridge_endpoints_new)
        asserts.assert_true(th_fsa_bridge_endpoints_new.issuperset(th_fsa_bridge_endpoints),
                            "Expected only new endpoints to be added")
        unique_endpoints_set = th_fsa_bridge_endpoints_new - th_fsa_bridge_endpoints
        asserts.assert_equal(len(unique_endpoints_set), 1, "Expected only one new endpoint")
        th_fsa_bridge_th_server_endpoint = list(unique_endpoints_set)[0]

        # Verify that TH_FSA created a UniqueID for TH_SERVER_NO_UID.
        th_fsa_bridge_th_server_unique_id = await self.read_single_attribute_check_success(
            cluster=Clusters.BridgedDeviceBasicInformation,
            attribute=Clusters.BridgedDeviceBasicInformation.Attributes.UniqueID,
            node_id=th_fsa_bridge_th_node_id,
            endpoint=th_fsa_bridge_th_server_endpoint)
        asserts.assert_true(type_matches(th_fsa_bridge_th_server_unique_id, str), "UniqueID should be a string")
        asserts.assert_true(th_fsa_bridge_th_server_unique_id, "UniqueID should not be an empty string")
        logging.info("UniqueID generated for TH_SERVER_NO_UID: %s", th_fsa_bridge_th_server_unique_id)

        self.step(3)

        discriminator = random.randint(0, 4095)
        # Open commissioning window on TH_FSA_BRIDGE.
        params = await self.default_controller.OpenCommissioningWindow(
            nodeid=th_fsa_bridge_th_node_id,
            option=self.default_controller.CommissioningWindowPasscode.kTokenWithRandomPin,
            discriminator=discriminator,
            iteration=10000,
            timeout=600)

        self.step(4)

        # Commissioning TH_FSA_BRIDGE to DUT_FSA fabric.
        if not self.is_ci:
            self.wait_for_user_input(
                f"Commission TH_FSA's aggregator on DUT using manufacturer specified mechanism.\n"
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

        self.step(5)

        # Get the list of endpoints on the DUT_FSA_BRIDGE before synchronization.
        dut_fsa_bridge_endpoints = set(await self.read_single_attribute_check_success(
            cluster=Clusters.Descriptor,
            attribute=Clusters.Descriptor.Attributes.PartsList,
            node_id=self.dut_node_id,
            endpoint=0,
        ))

        # Synchronize TH_SERVER_NO_UID from TH_FSA to DUT_FSA fabric.
        if not self.is_ci:
            self.wait_for_user_input(
                f"Synchronize endpoint from TH_FSA's aggregator to DUT using manufacturer specified mechanism.\n"
                f"Use the following parameters:\n"
                f"- endpointID: {th_fsa_bridge_th_server_endpoint}\n"
                f"If using FabricSync Admin, you may type:\n"
                f">>> fabricsync sync-device {th_fsa_bridge_th_server_endpoint}")
        else:
            self.dut_fsa_stdin.write(f"fabricsync sync-device {th_fsa_bridge_th_server_endpoint}\n")
            self.dut_fsa_stdin.flush()
            # Wait for the synchronization to complete.
            await asyncio.sleep(5)

        self.step(6)

        # Get the list of endpoints on the DUT_FSA_BRIDGE after synchronization
        dut_fsa_bridge_endpoints_new = set(await self.read_single_attribute_check_success(
            cluster=Clusters.Descriptor,
            attribute=Clusters.Descriptor.Attributes.PartsList,
            node_id=self.dut_node_id,
            endpoint=0,
        ))

        # Get the endpoint number for just synced TH_SERVER_NO_UID.
        logging.info("Endpoints on DUT_FSA_BRIDGE: old=%s, new=%s", dut_fsa_bridge_endpoints, dut_fsa_bridge_endpoints_new)
        asserts.assert_true(dut_fsa_bridge_endpoints_new.issuperset(dut_fsa_bridge_endpoints),
                            "Expected only new endpoints to be added")
        unique_endpoints_set = dut_fsa_bridge_endpoints_new - dut_fsa_bridge_endpoints
        asserts.assert_equal(len(unique_endpoints_set), 1, "Expected only one new endpoint on DUT_FSA")
        dut_fsa_bridge_th_server_endpoint = list(unique_endpoints_set)[0]

        # Verify that DUT_FSA copied the TH_SERVER_NO_UID UniqueID from TH_FSA.
        dut_fsa_bridge_th_server_unique_id = await self.read_single_attribute_check_success(
            cluster=Clusters.BridgedDeviceBasicInformation,
            attribute=Clusters.BridgedDeviceBasicInformation.Attributes.UniqueID,
            endpoint=dut_fsa_bridge_th_server_endpoint)
        asserts.assert_true(type_matches(dut_fsa_bridge_th_server_unique_id, str), "UniqueID should be a string")
        asserts.assert_true(dut_fsa_bridge_th_server_unique_id, "UniqueID should not be an empty string")
        logging.info("UniqueID for TH_SERVER_NO_UID on DUT_FSA: %s", th_fsa_bridge_th_server_unique_id)

        # Make sure that the UniqueID on the DUT_FSA_BRIDGE is the same as the one on the DUT_FSA_BRIDGE.
        asserts.assert_equal(dut_fsa_bridge_th_server_unique_id, th_fsa_bridge_th_server_unique_id,
                             "UniqueID on DUT_FSA and TH_FSA should be the same")


if __name__ == "__main__":
    default_matter_test_main()
