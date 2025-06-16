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
#     script-args: >
#       --string-arg th_server_app:${ALL_CLUSTERS_APP}
#       --string-arg jfa_server_app:${ALL_CLUSTERS_APP}
#       --string-arg jfc_server_app:${ALL_CLUSTERS_APP}
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

import builtins

import chip.clusters as Clusters
from chip.testing.apps import AppServerSubprocess, JFControllerSubprocess
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main, MatterStackState, MatterTestConfig, parse_matter_test_args
from mobly import asserts
from chip import ChipStack, CertificateAuthority
from chip.storage import PersistentStorage


class TC_JCM_1_1(MatterBaseTest):

    @async_test_body
    async def setup_class(self):
        super().setup_class()

        self.fabric_a_ctrl = None
        self.storage_fabric_a = self.user_params.get("fabric_a_storage", None)
        self.fabric_b_ctrl = None
        self.storage_fabric_b = self.user_params.get("fabric_b_storage", None)

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
        if self.storage_fabric_a is None:
            self.storage_fabric_a = tempfile.TemporaryDirectory(prefix=self.__class__.__name__+"_A_")
            logging.info("Temporary storage directory: %s", self.storage_fabric_a.name)
        if self.storage_fabric_b is None:
            self.storage_fabric_b = tempfile.TemporaryDirectory(prefix=self.__class__.__name__+"_B_")
            logging.info("Temporary storage directory: %s", self.storage_fabric_b.name)

        self.jfadmin_fabric_a_passcode = random.randint(110220011, 110220999)
        self.jfctrl_fabric_a_vid = 0xFFF1

        # Start Fabric A JF-Administrator
        self.fabric_a_admin = AppServerSubprocess(
            jfa_server_app,
            storage_dir=self.storage_fabric_a,
            # storage_dir="/tmp/TC_JCM_1_1_A_e9j542kk/",
            port=random.randint(5001, 5999),
            discriminator=random.randint(0, 4095),
            passcode=self.jfadmin_fabric_a_passcode,
            extra_args=["--capabilities", "0x04", "--rpc-server-port", "33033"])
        self.fabric_a_admin.start(
            expected_output="Server initialization complete",
            timeout=10)

        # Start Controller for Fabric A
        self.fabric_a_ctrl = JFControllerSubprocess(
            jfc_server_app,
            rpc_server_port=33033,
            # storage_dir="/tmp/TC_JCM_1_1_A_e9j542kk/",
            storage_dir=self.storage_fabric_a,
            vendor_id=self.jfctrl_fabric_a_vid)
        self.fabric_a_ctrl.start(
            expected_output="CHIP task running",
            timeout=10)

        # Commission JF-ADMIN A
        self.fabric_a_ctrl.send(
            message=f"pairing onnetwork 1 {self.jfadmin_fabric_a_passcode} --anchor true",
            expected_output="[JF] Anchor Administrator commissioned with sucess")

        self.jfadmin_fabric_b_passcode = random.randint(110220011, 110220999)
        self.jfctrl_fabric_b_vid = 0xFFF2

        # Start Fabric B JF-Administrator
        self.fabric_b_admin = AppServerSubprocess(
            jfa_server_app,
            storage_dir=self.storage_fabric_b,
            # storage_dir="/tmp/TC_JCM_1_1_B_a5uxeocg/",
            port=random.randint(5001, 5999),
            discriminator=random.randint(0, 4095),
            passcode=self.jfadmin_fabric_b_passcode,
            extra_args=["--capabilities", "0x04", "--rpc-server-port", "33055"])
        self.fabric_b_admin.start(
            expected_output="Server initialization complete",
            timeout=10)

        # # Start Controller for Fabric B
        self.fabric_b_ctrl = JFControllerSubprocess(
            jfc_server_app,
            rpc_server_port=33055,
            storage_dir=self.storage_fabric_b,
            # storage_dir="/tmp/TC_JCM_1_1_B_a5uxeocg/",
            vendor_id=self.jfctrl_fabric_b_vid)
        self.fabric_b_ctrl.start(
            expected_output="CHIP task running",
            timeout=10)

        # # Commission JF-ADMIN B
        self.fabric_b_ctrl.send(
            message=f"pairing onnetwork 11 {self.jfadmin_fabric_b_passcode} --anchor true",
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
            TestStep("1", "Verify VID on the JFAdmin apps"),
            TestStep("2", "Verify NOCs of both JF-Admin apps from Ecosystem A and Ecosystem B"),
            TestStep("3", "Commission a server app in Ecosystem A and read Fabric Vendor ID"),
            TestStep("4", "Commission a server app in Ecosystem B and read Fabric Vendor ID")
        ]

    @async_test_body
    async def test_TC_JCM_1_1(self):

        self.step("1")
        # TODO: Create admin_storage.json from INI file of each controller
        _fabric_a_persistent_storage = PersistentStorage(path=self.storage_fabric_a+"admin_storage.json")
        _certAuthorityManagerA = CertificateAuthority.CertificateAuthorityManager(
            chipStack=self.matter_stack._chip_stack,
            persistentStorage=_fabric_a_persistent_storage)
        _certAuthorityManagerA.LoadAuthoritiesFromStorage()
        devCtrlA = _certAuthorityManagerA.activeCaList[0].adminList[0].NewController(
            nodeId=101,
            paaTrustStorePath=str(self.matter_test_config.paa_trust_store_path),
            # TODO: Extract AdminCATs from INI file
            catTags=[0xFFFF_0001]
        )

        _fabric_b_persistent_storage = PersistentStorage(path=self.storage_fabric_b+"admin_storage.json")
        _certAuthorityManagerB = CertificateAuthority.CertificateAuthorityManager(
            chipStack=self.matter_stack._chip_stack,
            persistentStorage=_fabric_b_persistent_storage)
        _certAuthorityManagerB.LoadAuthoritiesFromStorage()
        devCtrlB = _certAuthorityManagerB.activeCaList[0].adminList[0].NewController(
            nodeId=201,
            paaTrustStorePath=str(self.matter_test_config.paa_trust_store_path),
            catTags=[0xFFFF_0001]
        )

        # TODO: Parse response NOC and ICAC to extract certificates
        responseA = await devCtrlA.ReadAttribute(
            nodeid=1, attributes=[(0, Clusters.OperationalCredentials.Attributes.NOCs)],
            returnClusterObject=True
        )

        responseB = await devCtrlB.ReadAttribute(
            nodeid=11, attributes=[(0, Clusters.OperationalCredentials.Attributes.NOCs)],
            returnClusterObject=True
        )

        self.fabric_a_ctrl.send(
            message="operationalcredentials read fabrics 1 0",
            expected_output=f"VendorID: {int(self.jfctrl_fabric_a_vid)}",
            timeout=10)
        self.fabric_b_ctrl.send(
            message="operationalcredentials read fabrics 11 0",
            expected_output=f"VendorID: {int(self.jfctrl_fabric_b_vid)}",
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
        self.thserver_fabric_a_passcode = random.randint(110220011, 110220999)
        fabric_a_server_app = AppServerSubprocess(
            self.th_server_app,
            storage_dir=self.storage_fabric_a.name,
            port=random.randint(5001, 5999),
            discriminator=random.randint(0, 4095),
            passcode=self.thserver_fabric_a_passcode,
            extra_args=["--capabilities", "0x04"])
        fabric_a_server_app.start(
            expected_output="Server initialization complete",
            timeout=10)

        self.fabric_a_ctrl.send(
            message=f"pairing onnetwork 2 {self.thserver_fabric_a_passcode}",
            expected_output="[CTL] Commissioning complete for node ID 0x0000000000000002: success",
            timeout=10
        )

        self.fabric_a_ctrl.send(
            message="operationalcredentials read fabrics 2 0",
            expected_output=f"VendorID: {int(self.jfctrl_fabric_a_vid)}",
            timeout=10
        )

        self.fabric_a_ctrl.send(
            message="accesscontrol read acl 2 0",
            expected_output="[1]: 18446744065119551489",
            timeout=10
        )

        self.step("4")
        self.thserver_fabric_b_passcode = random.randint(110220011, 110220999)
        fabric_b_server_app = AppServerSubprocess(
            self.th_server_app,
            storage_dir=self.storage_fabric_b.name,
            port=random.randint(5001, 5999),
            discriminator=random.randint(0, 4095),
            passcode=self.thserver_fabric_b_passcode,
            extra_args=["--capabilities", "0x04"])
        fabric_b_server_app.start(
            expected_output="Server initialization complete",
            timeout=10)

        self.fabric_b_ctrl.send(
            message=f"pairing onnetwork 22 {self.thserver_fabric_b_passcode}",
            expected_output="[CTL] Commissioning complete for node ID 0x0000000000000016: success",
            timeout=10
        )

        self.fabric_b_ctrl.send(
            message="operationalcredentials read fabrics 22 0",
            expected_output=f"VendorID: {int(self.jfctrl_fabric_b_vid)}",
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
