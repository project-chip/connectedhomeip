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

# This test requires a TH_SERVER application. Please specify with --string-arg th_server_app_path:<path_to_app>

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: examples/fabric-admin/scripts/fabric-sync-app.py
#     app-args: --app-admin=${FABRIC_ADMIN_APP} --app-bridge=${FABRIC_BRIDGE_APP} --discriminator=1234
#     app-ready-pattern: "Successfully opened pairing window on the device"
#     app-stdin-pipe: dut-fsa-stdin
#     script-args: >
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --string-arg th_server_app_path:${ALL_CLUSTERS_APP}
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging
import os
import random
import tempfile
import time

import chip.clusters as Clusters
from chip.testing.apps import AppServerSubprocess, JFControllerSubprocess
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_JCM_1_1(MatterBaseTest):

    @async_test_body
    async def setup_class(self):
        super().setup_class()

        self.fabric_a_ctrl = None
        self.storage_fabric_a = None
        self.fabric_b_ctrl = None
        self.storage_fabric_b = None

        jfc_server_app = self.user_params.get("jfc_server_app", None)
        if not jfc_server_app:
            asserts.fail("This test requires a Joint Fabric Controller app. Specify app path with --string-arg jfc_server_app:<path_to_app>")
        if not os.path.exists(jfc_server_app):
            asserts.fail(f"The path {jfc_server_app} does not exist")

        jfa_server_app = self.user_params.get("jfa_server_app", None)
        if not jfa_server_app:
            asserts.fail("This test requires a Joint Fabrics Admin app. Specify app path with --string-arg jfa_server_app:<path_to_app>")
        if not os.path.exists(jfa_server_app):
            asserts.fail(f"The path {jfa_server_app} does not exist")

        self.th_server_app = self.user_params.get("th_server_app", None)
        if not self.th_server_app:
            asserts.fail("This test requires a TH Server app. Specify app path with --string-arg th_server_app:<path_to_app>")
        if not os.path.exists(self.th_server_app):
            asserts.fail(f"The path {self.th_server_app} does not exist")

        # Create a temporary storage directory for keeping KVS files.
        self.storage_fabric_a = tempfile.TemporaryDirectory(prefix=self.__class__.__name__+"_A_")
        logging.info("Temporary storage directory: %s", self.storage_fabric_a.name)
        self.storage_fabric_b = tempfile.TemporaryDirectory(prefix=self.__class__.__name__+"_B_")
        logging.info("Temporary storage directory: %s", self.storage_fabric_b.name)

        # Start Fabric A JF-Administrator
        self.fabric_a_admin = AppServerSubprocess(
            jfa_server_app,
            storage_dir=self.storage_fabric_a.name,
            port=5533,
            discriminator=random.randint(0, 4095),
            passcode=110220033,
            extra_args=["--capabilities", "0x04", "--rpc-server-port", "33033"])
        self.fabric_a_admin.start(
            expected_output="Server initialization complete",
            timeout=10)

        # Start Controller for Fabric A
        self.fabric_a_ctrl = JFControllerSubprocess(
            jfc_server_app,
            rpc_server_port=33033,
            storage_dir=self.storage_fabric_a.name,
            vendor_id=0xFFF1)
        self.fabric_a_ctrl.start(
            expected_output="CHIP task running",
            timeout=10)

        # Commission JF-ADMIN A
        self.fabric_a_ctrl.send(
            message="pairing onnetwork 1 110220033 --anchor true",
            expected_output="[JF] Anchor Administrator commissioned with sucess")

        # Start Fabric B JF-Administrator
        self.fabric_b_admin = AppServerSubprocess(
            jfa_server_app,
            storage_dir=self.storage_fabric_b.name,
            port=5555,
            discriminator=random.randint(0, 4095),
            passcode=110220055,
            extra_args=["--capabilities", "0x04", "--rpc-server-port", "33055"])
        self.fabric_b_admin.start(
            expected_output="Server initialization complete",
            timeout=10)

        # Start Controller for Fabric B
        self.fabric_b_ctrl = JFControllerSubprocess(
            jfc_server_app,
            rpc_server_port=33055,
            storage_dir=self.storage_fabric_b.name,
            vendor_id=0xFFF2)
        self.fabric_b_ctrl.start(
            expected_output="CHIP task running",
            timeout=10)

        # Commission JF-ADMIN B
        self.fabric_b_ctrl.send(
            message="pairing onnetwork 11 110220055 --anchor true",
            expected_output="[JF] Anchor Administrator commissioned with sucess")

    def teardown_class(self):
        if self.fabric_a_admin is not None:
            self.fabric_a_admin.terminate()
        if self.fabric_b_admin is not None:
            self.fabric_b_admin.terminate()
        if self.fabric_a_ctrl is not None:
            self.fabric_a_ctrl.terminate()
        if self.fabric_b_ctrl is not None:
            self.fabric_b_ctrl.terminate()
        if self.storage_fabric_a is not None:
            self.storage_fabric_a.cleanup()
        if self.storage_fabric_b is not None:
            self.storage_fabric_b.cleanup()

        super().teardown_class()

    def steps_TC_JCM_1_1(self) -> list[TestStep]:
        return [
            TestStep("1", "Commission DUT if not already done", is_commissioning=True),
            TestStep("2", "Read NOCs of both JF-Admin apps from Ecosystem A and Ecosystem B",
                     " Verify that NOC contains Anchor CAT and Administrator CAT"),
            TestStep("3", "Commission a server app in Ecosystem A and read Fabric Vendor ID", ""),
            TestStep("4", "Commission a server app in Ecosystem B and read Fabric Vendor ID", "")
        ]

    @async_test_body
    async def test_TC_JCM_1_1(self):

        self.step("1")

        self.fabric_a_ctrl.send(
            message="operationalcredentials read fabrics 1 0",
            expected_output="VendorID: 65521",
            timeout=10)
        self.fabric_b_ctrl.send(
            message="operationalcredentials read fabrics 11 0",
            expected_output="VendorID: 65522",
            timeout=10)

        self.step("2")
        self.fabric_a_ctrl.send(
            message="operationalcredentials read nocs 1 0",
            expected_output="NOCs: 1 entries",
            timeout=10)
        self.fabric_b_ctrl.send(
            message="operationalcredentials read nocs 11 0",
            expected_output="NOCs: 1 entries",
            timeout=10)

        self.step("3")
        fabric_a_server_app = AppServerSubprocess(
            self.th_server_app,
            storage_dir=self.storage_fabric_a.name,
            port=5534,
            discriminator=random.randint(0, 4095),
            passcode=110220044,
            extra_args=["--capabilities", "0x04"])
        fabric_a_server_app.start(
            expected_output="Server initialization complete",
            timeout=10)

        self.fabric_a_ctrl.send(
            message="pairing onnetwork 2 110220044",
            expected_output="[CTL] Commissioning complete for node ID 0x0000000000000002: success",
            timeout=10
        )

        self.fabric_a_ctrl.send(
            message="operationalcredentials read fabrics 2 0",
            expected_output="VendorID: 65521",
            timeout=10
        )

        self.fabric_a_ctrl.send(
            message="accesscontrol read acl 2 0",
            expected_output="[1]: 18446744065119551489",
            timeout=10
        )

        self.step("4")
        fabric_b_server_app = AppServerSubprocess(
            self.th_server_app,
            storage_dir=self.storage_fabric_b.name,
            port=5556,
            discriminator=random.randint(0, 4095),
            passcode=110220066,
            extra_args=["--capabilities", "0x04"])
        fabric_b_server_app.start(
            expected_output="Server initialization complete",
            timeout=10)

        self.fabric_b_ctrl.send(
            message="pairing onnetwork 22 110220066",
            expected_output="[CTL] Commissioning complete for node ID 0x0000000000000016: success",
            timeout=10
        )

        self.fabric_b_ctrl.send(
            message="operationalcredentials read fabrics 22 0",
            expected_output="VendorID: 65522",
            timeout=10
        )

        self.fabric_b_ctrl.send(
            message="accesscontrol read acl 22 0",
            expected_output="[1]: 18446744065119551489",
            timeout=10
        )

        self.teardown_test()


if __name__ == "__main__":
    default_matter_test_main()
