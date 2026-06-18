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
#       --string-arg jfa_server_app:${JF_ADMIN_APP}
#       --string-arg jfc_server_app:${JF_CONTROL_APP}
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --PICS src/app/tests/suites/certification/ci-pics-values
#     factory-reset: true
# === END CI TEST ARGUMENTS ===

import base64
import logging
import os
import random
import tempfile
from configparser import ConfigParser

from mobly import asserts

import matter.clusters as Clusters
import matter.tlv
from matter import CertificateAuthority
from matter.storage import VolatileTemporaryPersistentStorage
from matter.testing.apps import AppServerSubprocess, JFAdministratorSubprocess, JFControllerSubprocess
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_JFADMIN_2_1(MatterBaseTest):
    @async_test_body
    async def setup_class(self):
        super().setup_class()

        self.fabric_a_ctrl = None
        self.storage_fabric_a = self.user_params.get("fabric_a_storage", None)
        self.fabric_b_ctrl = None
        self.storage_fabric_b = self.user_params.get("fabric_b_storage", None)
        self.fabric_a_server_app = None
        self.fabric_b_server_app = None
        self.fabric_a_admin = None
        self.fabric_b_admin = None
        self.dev_ctrl_eco_a = None
        self.dev_ctrl_eco_b = None
        self.joint_fabric_persistent_storage = None
        self.joint_fabric_ca_manager = None

        self.jfc_server_app = self.user_params.get("jfc_server_app", None)
        if not self.jfc_server_app:
            asserts.fail("This test requires a Joint Fabric Controller app. Specify app path with --string-arg jfc_server_app:<path_to_app>")
        if not os.path.exists(self.jfc_server_app):
            asserts.fail(f"The path {self.jfc_server_app} does not exist")

        self.jfa_server_app = self.user_params.get("jfa_server_app", None)
        if not self.jfa_server_app:
            asserts.fail("This test requires a Joint Fabrics Admin app. Specify app path with --string-arg jfa_server_app:<path_to_app>")
        if not os.path.exists(self.jfa_server_app):
            asserts.fail(f"The path {self.jfa_server_app} does not exist")

        self.th_server_app = self.user_params.get("th_server_app", None)
        if not self.th_server_app:
            asserts.fail("This test requires a TH Server app. Specify app path with --string-arg th_server_app:<path_to_app>")
        if not os.path.exists(self.th_server_app):
            asserts.fail(f"The path {self.th_server_app} does not exist")

        # Create a temporary storage directory for both ecosystems to keep KVS files if not already provided by user.
        if self.storage_fabric_a is None:
            self.storage_directory_ecosystem_a = tempfile.mkdtemp(prefix=self.__class__.__name__+"_A_")
            self.storage_fabric_a = self.storage_directory_ecosystem_a
            log.info("Temporary storage directory: %s", self.storage_fabric_a)
        if self.storage_fabric_b is None:
            self.storage_directory_ecosystem_b = tempfile.mkdtemp(prefix=self.__class__.__name__+"_B_")
            self.storage_fabric_b = self.storage_directory_ecosystem_b
            log.info("Temporary storage directory: %s", self.storage_fabric_b)

    def teardown_class(self):
        # Stop all Subprocesses that were started in this test case
        if self.fabric_a_admin is not None:
            self.fabric_a_admin.terminate()
        if self.fabric_b_admin is not None:
            self.fabric_b_admin.terminate()
        if self.fabric_a_ctrl is not None:
            self.fabric_a_ctrl.terminate()
        if self.fabric_b_ctrl is not None:
            self.fabric_b_ctrl.terminate()
        if self.fabric_a_server_app is not None:
            self.fabric_a_server_app.terminate()
        if self.fabric_b_server_app is not None:
            self.fabric_b_server_app.terminate()

        if self.dev_ctrl_eco_a is not None:
            self.dev_ctrl_eco_a.Shutdown()
            self.dev_ctrl_eco_a = None
        if self.dev_ctrl_eco_b is not None:
            self.dev_ctrl_eco_b.Shutdown()
            self.dev_ctrl_eco_b = None
        if self.joint_fabric_ca_manager is not None:
            self.joint_fabric_ca_manager.Shutdown()
            self.joint_fabric_ca_manager = None
        if self.joint_fabric_persistent_storage is not None:
            self.joint_fabric_persistent_storage.Shutdown()
            self.joint_fabric_persistent_storage = None

        super().teardown_class()

    def steps_TC_JFADMIN_2_1(self) -> list[TestStep]:
        return [
            TestStep("1", "DUT_AAF starts a commissioning process to commission TH_DEV1 on Fabric 1",
                     "TH_DEV1 is commissioned by DUT_AAF on Fabric 1"),
            TestStep("2", "TH_AJF2 starts a commissioning process to commission TH_DEV2 on Fabric 2",
                     "TH_DEV2 is commissioned by TH_AJF2 on Fabric 2"),
            TestStep("3", "Open a Commissioning Window on TH_AJF2 using OpenJointCommissioningWindow command",
                     "Verify TH_AJF2 opens its Commissioning window to allow another commissioning"),
            TestStep("4", "DUT_AAF starts a commissioning process to commission TH_AJF2 on Fabric 1 using Joint Commissioning Method",
                     "TH_AJF2 is successfuly commissioned into Fabric 1 by DUT_AAF using Joint Commissioning Method")
        ]

    def pics_TC_JFADMIN_2_1(self):
        return ['JFADMIN.S']

    @async_test_body
    async def test_TC_JFADMIN_2_1(self):
        self.dev_ctrl_eco_a = None
        self.dev_ctrl_eco_b = None
        self.joint_fabric_persistent_storage = None
        self.joint_fabric_ca_manager = None
        jfadmin_fabric_a_passcode = None
        dut_rpc_server_ip = None
        dut_rpc_server_port = None

        self.step("1")
        self.jfctrl_fabric_a_vid = random.randint(0x0001, 0xFFF0)

        # If test is executed in CI environment, start JFA app for Fabric A
        if self.is_pics_sdk_ci_only:
            dut_rpc_server_ip = "127.0.0.1"
            jfadmin_fabric_a_passcode = random.randint(110220011, 110220999)
            dut_rpc_server_port = str(self.get_random_port())
            # Start Fabric A JF-Administrator App (DUT)
            self.fabric_a_admin = JFAdministratorSubprocess(
                self.jfa_server_app,
                prefix="JFA-A",
                storage_dir=self.storage_fabric_a,
                port=self.get_random_port(),
                discriminator=random.randint(0, 4095),
                passcode=jfadmin_fabric_a_passcode,
                extra_args=["--capabilities", "0x04", "--rpc-server-port", dut_rpc_server_port, "--secured-commissioner-port", str(self.get_random_port())])
            self.fabric_a_admin.start(
                expected_output="Server initialization complete",
                timeout=20)
        else:
            # We have a DUT that is already running, connect to it via RPC
            dut_rpc_server_ip = self.user_params.get("dut_rpc_server_ip", None)
            if not dut_rpc_server_ip:
                asserts.fail("DUT RPC server IP must be specified via --string-arg dut_rpc_server_ip:<ip_address>")
            dut_rpc_server_port = self.user_params.get("dut_rpc_server_port", None)
            if not dut_rpc_server_port:
                asserts.fail("DUT RPC server PORT must be specified via --string-arg dut_rpc_server_port:<port>")
            jfadmin_fabric_a_passcode = self.matter_test_config.setup_passcodes[0]
            if not jfadmin_fabric_a_passcode:
                asserts.fail(
                    "JF-Administrator passcode and discriminator must be specified via --passcode:<passcode> --discriminator:<discriminator>")

        # Start Fabric A JF-Controller App
        self.fabric_a_ctrl = JFControllerSubprocess(
            self.jfc_server_app,
            prefix="JFC-A",
            rpc_server_port=dut_rpc_server_port,
            storage_dir=self.storage_fabric_a,
            vendor_id=self.jfctrl_fabric_a_vid,
            extra_args=["--rpc-server-ip", dut_rpc_server_ip])
        self.fabric_a_ctrl.start(
            expected_output="CHIP task running",
            timeout=20)

        # Commission JF-ADMIN app with JF-Controller on Fabric A
        self.fabric_a_ctrl.send(
            message=f"pairing onnetwork 1 {jfadmin_fabric_a_passcode} --anchor true",
            expected_output="[JF] Anchor Administrator (nodeId=1) commissioned with success",
            timeout=20)

        # Extract the Ecosystem A certificates and inject them in the storage that will be provided to a new Python Controller later
        jfcStorage = ConfigParser()
        jfcStorage.read(self.storage_fabric_a+'/chip_tool_config.alpha.ini')
        self.ecoACtrlStorage = {
            "sdk-config": {
                "ExampleOpCredsCAKey1": jfcStorage.get("Default", "ExampleOpCredsCAKey0"),
                "ExampleOpCredsICAKey1": jfcStorage.get("Default", "ExampleOpCredsICAKey0"),
                "ExampleCARootCert1": jfcStorage.get("Default", "ExampleCARootCert0"),
                "ExampleCAIntermediateCert1": jfcStorage.get("Default", "ExampleCAIntermediateCert0"),
            },
            "repl-config": {
                "caList": {
                    "1": [
                        {
                            "fabricId": 1,
                            "vendorId": self.jfctrl_fabric_a_vid
                        }
                    ]
                }
            }
        }
        # Extract CATs to be provided to the Python Controller later
        self.ecoACATs = base64.b64decode(jfcStorage.get("Default", "CommissionerCATs"))[::-1].hex().strip('0')

        self.thserver_fabric_a_passcode = random.randint(110220011, 110220999)
        self.fabric_a_server_app = AppServerSubprocess(
            self.th_server_app,
            storage_dir=self.storage_fabric_a,
            port=self.get_random_port(),
            discriminator=random.randint(0, 4095),
            passcode=self.thserver_fabric_a_passcode,
            extra_args=["--capabilities", "0x04"])
        self.fabric_a_server_app.start(
            expected_output="Server initialization complete",
            timeout=20)

        self.fabric_a_ctrl.send(
            message=f"pairing onnetwork 2 {self.thserver_fabric_a_passcode} --regular true",
            expected_output="[CTL] Commissioning complete for node ID 0x0000000000000002: success",
            timeout=20)

        # Creating a unified Controller Manager for both ecosystems
        self.joint_fabric_persistent_storage = VolatileTemporaryPersistentStorage(
            self.ecoACtrlStorage['repl-config'], self.ecoACtrlStorage['sdk-config'])
        self.joint_fabric_ca_manager = CertificateAuthority.CertificateAuthorityManager(
            chipStack=self.matter_stack._chip_stack,
            persistentStorage=self.joint_fabric_persistent_storage)
        self.joint_fabric_ca_manager.LoadAuthoritiesFromStorage()
        self.dev_ctrl_eco_a = self.joint_fabric_ca_manager.activeCaList[0].adminList[0].NewController(
            nodeId=101,
            paaTrustStorePath=str(self.matter_test_config.paa_trust_store_path),
            catTags=[int(self.ecoACATs, 16)])

        # Precondition 2 Validation
        response = await self.dev_ctrl_eco_a.ReadAttribute(
            nodeId=1, attributes=[(0, Clusters.OperationalCredentials.Attributes.NOCs)],
            returnClusterObject=True)
        # Search Administrator CAT (FFFF0001) and Anchor CAT (FFFD0001) in JF-Admin NOC on Ecoystem A
        noc_tlv_data = matter.tlv.TLVReader(response[0][Clusters.OperationalCredentials].NOCs[0].noc).get()
        _admin_cat_found = False
        _anchor_cat_found = False
        for _tag, _value in noc_tlv_data['Any'][6]:
            if _tag == 22 and _value == int(self.ecoACATs, 16):
                _admin_cat_found = True
            elif _tag == 22 and _value == int("FFFE0001", 16):
                _anchor_cat_found = True
            if _admin_cat_found and _anchor_cat_found:
                break
        asserts.assert_true(_admin_cat_found, "Administrator CAT not found in Admin App NOC on Ecosystem A")
        asserts.assert_true(_anchor_cat_found, "Anchor CAT not found in Admin App NOC on Ecosystem A")
        # Search jf-anchor-cat in Subject field of JF-Admin ICAC on Ecoystem A
        try:
            icac_tlv_data = matter.tlv.TLVReader(response[0][Clusters.OperationalCredentials].NOCs[0].icac).get()
            _found = False
            for _tag, _value in icac_tlv_data['Any'][6]:
                if _tag == 8 and _value == 'jf-anchor-icac':
                    _found = True
                    break
            asserts.assert_true(_found, "Anchor ICAC (jf-anchor-icac) not found in Admin App ICAC Subject field on Ecosystem A")
        except (TypeError, AttributeError) as e:
            asserts.fail(f"Failed to parse ICAC in precondition 2: {e}")

        response = await self.dev_ctrl_eco_a.ReadAttribute(
            nodeId=2, attributes=[(0, Clusters.OperationalCredentials.Attributes.Fabrics)],
            returnClusterObject=True)
        asserts.assert_equal(
            self.jfctrl_fabric_a_vid,
            response[0][Clusters.OperationalCredentials].fabrics[0].vendorID,
            "JF-Admin App on Ecosystem A doesn't have the correct VID")

        response = await self.dev_ctrl_eco_a.ReadAttribute(
            nodeId=2, attributes=[(0, Clusters.AccessControl.Attributes.Acl)],
            returnClusterObject=True)
        asserts.assert_equal(
            int('0xFFFFFFFD'+self.ecoACATs, 16),
            response[0][Clusters.AccessControl].acl[0].subjects[0],
            "EcoA Server App Subjects field has wrong value")

        self.step("2")
        self.jfadmin_fabric_b_passcode = random.randint(110220011, 110220999)
        self.jfctrl_fabric_b_vid = random.randint(0x0001, 0xFFF0)

        fabric_b_rpc_port = self.get_random_port()

        # Start Fabric B JF-Administrator App
        self.fabric_b_admin = JFAdministratorSubprocess(
            self.jfa_server_app,
            prefix="JFA-B",
            storage_dir=self.storage_fabric_b,
            port=self.get_random_port(),
            discriminator=random.randint(0, 4095),
            passcode=self.jfadmin_fabric_b_passcode,
            extra_args=["--capabilities", "0x04", "--rpc-server-port", str(fabric_b_rpc_port), "--secured-commissioner-port", str(self.get_random_port())])
        self.fabric_b_admin.start(
            expected_output="Server initialization complete",
            timeout=20)

        # Start Fabric B JF-Administrator App
        self.fabric_b_ctrl = JFControllerSubprocess(
            self.jfc_server_app,
            prefix="JFC-B",
            rpc_server_port=fabric_b_rpc_port,
            storage_dir=self.storage_fabric_b,
            vendor_id=self.jfctrl_fabric_b_vid)
        self.fabric_b_ctrl.start(
            expected_output="CHIP task running",
            timeout=20)

        # Commission JF-ADMIN app with JF-Controller on Fabric B
        self.fabric_b_ctrl.send(
            message=f"pairing onnetwork 11 {self.jfadmin_fabric_b_passcode} --anchor true --commissioner-name beta",
            expected_output="[JF] Anchor Administrator (nodeId=11) commissioned with success",
            timeout=20)

        # Extract the Ecosystem B certificates and inject them in the storage that will be provided to a new Python Controller later
        jfcStorage = ConfigParser()
        jfcStorage.read(self.storage_fabric_b+'/chip_tool_config.beta.ini')
        self.ecoBCtrlStorage = {
            "sdk-config": {
                "ExampleOpCredsCAKey2": jfcStorage.get("Default", "ExampleOpCredsCAKey0"),
                "ExampleOpCredsICAKey2": jfcStorage.get("Default", "ExampleOpCredsICAKey0"),
                "ExampleCARootCert2": jfcStorage.get("Default", "ExampleCARootCert0"),
                "ExampleCAIntermediateCert2": jfcStorage.get("Default", "ExampleCAIntermediateCert0"),
            },
            "repl-config": {
                "caList": {
                    "2": [
                        {
                            "fabricId": 2,
                            "vendorId": self.jfctrl_fabric_b_vid
                        }
                    ]
                }
            }
        }
        # Extract CATs to be provided to the Python Controller later
        self.ecoBCATs = base64.b64decode(jfcStorage.get("Default", "CommissionerCATs"))[::-1].hex().strip('0')

        self.thserver_fabric_b_passcode = random.randint(110220011, 110220999)
        self.fabric_b_server_app = AppServerSubprocess(
            self.th_server_app,
            storage_dir=self.storage_fabric_b,
            port=self.get_random_port(),
            discriminator=random.randint(0, 4095),
            passcode=self.thserver_fabric_b_passcode,
            extra_args=["--capabilities", "0x04"])
        self.fabric_b_server_app.start(
            expected_output="Server initialization complete",
            timeout=20)

        self.fabric_b_ctrl.send(
            message=f"pairing onnetwork 22 {self.thserver_fabric_b_passcode} --commissioner-name beta --regular true",
            expected_output="[CTL] Commissioning complete for node ID 0x0000000000000016: success",
            timeout=20)

        # Merge Ecosystem B config into the unified storage
        for k, v in self.ecoBCtrlStorage['sdk-config'].items():
            self.joint_fabric_persistent_storage.SetSdkKey(k, base64.b64decode(v))

        caList = self.joint_fabric_persistent_storage.GetKey('caList') or {}
        caList["2"] = self.ecoBCtrlStorage['repl-config']['caList']['2']
        self.joint_fabric_persistent_storage.SetKey('caList', caList)

        # Create and load CA Index 2 explicitly in the unified manager
        ca2 = self.joint_fabric_ca_manager.NewCertificateAuthority(caIndex=2)
        ca2.LoadFabricAdminsFromStorage()

        # Create Ecosystem B controller using ca2 directly
        self.dev_ctrl_eco_b = ca2.adminList[0].NewController(
            nodeId=201,
            paaTrustStorePath=str(self.matter_test_config.paa_trust_store_path),
            catTags=[int(self.ecoBCATs, 16)])

        response = await self.dev_ctrl_eco_b.ReadAttribute(
            nodeId=11, attributes=[(0, Clusters.OperationalCredentials.Attributes.NOCs)],
            returnClusterObject=True)
        # Search Administrator CAT (FFFF0001) and Anchor CAT (FFFD0001) in JF-Admin NOC on Ecoystem A
        noc_tlv_data = matter.tlv.TLVReader(response[0][Clusters.OperationalCredentials].NOCs[0].noc).get()
        _admin_cat_found = False
        _anchor_cat_found = False
        for _tag, _value in noc_tlv_data['Any'][6]:
            if _tag == 22 and _value == int(self.ecoBCATs, 16):
                _admin_cat_found = True
            elif _tag == 22 and _value == int("FFFE0001", 16):
                _anchor_cat_found = True
            if _admin_cat_found and _anchor_cat_found:
                break
        asserts.assert_true(_admin_cat_found, "Administrator CAT not found in Admin App NOC on Ecosystem A")
        asserts.assert_true(_anchor_cat_found, "Anchor CAT not found in Admin App NOC on Ecosystem A")
        # Search jf-anchor-cat in Subject field of JF-Admin ICAC on Ecoystem A
        try:
            icac_tlv_data = matter.tlv.TLVReader(response[0][Clusters.OperationalCredentials].NOCs[0].icac).get()
            _found = False
            for _tag, _value in icac_tlv_data['Any'][6]:
                if _tag == 8 and _value == 'jf-anchor-icac':
                    _found = True
                    break
            asserts.assert_true(_found, "Anchor ICAC (jf-anchor-icac) not found in Admin App ICAC Subject field on Ecosystem A")
        except (TypeError, AttributeError) as e:
            asserts.fail(f"Failed to parse ICAC in precondition 2 (Ecosystem B): {e}")

        response = await self.dev_ctrl_eco_b.ReadAttribute(
            nodeId=22, attributes=[(0, Clusters.OperationalCredentials.Attributes.Fabrics)],
            returnClusterObject=True)
        asserts.assert_equal(
            self.jfctrl_fabric_b_vid,
            response[0][Clusters.OperationalCredentials].fabrics[0].vendorID,
            "JF-Admin App on Ecosystem B doesn't have the correct VID")

        response = await self.dev_ctrl_eco_b.ReadAttribute(
            nodeId=22, attributes=[(0, Clusters.AccessControl.Attributes.Acl)],
            returnClusterObject=True)
        asserts.assert_equal(
            int('0xFFFFFFFD'+self.ecoBCATs, 16),
            response[0][Clusters.AccessControl].acl[0].subjects[0],
            "EcoB Server App Subjects field has wrong value")

        # Precondition 3 Validation
        response = await self.dev_ctrl_eco_a.ReadAttribute(
            nodeId=1, attributes=[(0, Clusters.OperationalCredentials.Attributes.Fabrics)],
            returnClusterObject=True)
        asserts.assert_equal(
            self.jfctrl_fabric_a_vid,
            response[0][Clusters.OperationalCredentials].fabrics[0].vendorID,
            "JF-Admin App on Ecosystem A doesn't have the correct VID")

        response = await self.dev_ctrl_eco_b.ReadAttribute(
            nodeId=11, attributes=[(0, Clusters.OperationalCredentials.Attributes.Fabrics)],
            returnClusterObject=True)
        asserts.assert_equal(
            self.jfctrl_fabric_b_vid,
            response[0][Clusters.OperationalCredentials].fabrics[0].vendorID,
            "JF-Admin App on Ecosystem B doesn't have the correct VID")

        self.step("3")
        discriminator = random.randint(0, 4095)
        try:
            response = await self.dev_ctrl_eco_b.OpenJointCommissioningWindow(
                nodeId=11,
                endpointId=1,
                timeout=400,
                iteration=random.randint(1000, 100000),
                discriminator=discriminator
            )
        except Exception as e:
            asserts.fail(f'Exception {e} occured during OJCW')

        self.step("4")
        self.fabric_a_ctrl.send(
            message=f"pairing onnetwork-long 15 {response.setupPinCode} {discriminator} --jcm true",
            expected_output="[CTL] Commissioning complete for node ID 0x000000000000000F: success",
            timeout=60)

        fabric_a_trusted_roots = await self.dev_ctrl_eco_a.ReadAttribute(
            nodeId=1, attributes=[(0, Clusters.OperationalCredentials.Attributes.TrustedRootCertificates)],
            returnClusterObject=True, fabricFiltered=False)

        fabric_b_nocs = await self.dev_ctrl_eco_b.ReadAttribute(
            nodeId=11, attributes=[(0, Clusters.OperationalCredentials.Attributes.NOCs)],
            returnClusterObject=True, fabricFiltered=False)

        fabric_a_root_subjects = []
        fabric_a_roots = fabric_a_trusted_roots[0][Clusters.OperationalCredentials].trustedRootCertificates
        for root_cert in fabric_a_roots:
            try:
                rcac_data = matter.tlv.TLVReader(root_cert).get()
                root_subject = dict(rcac_data.get('Any', [])).get(6)  # Tag 6 = Subject
                if root_subject is not None:
                    fabric_a_root_subjects.append(root_subject)
            except (TypeError, AttributeError):
                # Skip root certs that can't be parsed
                continue

        asserts.assert_true(
            len(fabric_a_root_subjects) > 0,
            "No valid RCAC Subject found in Ecosystem A TrustedRootCertificates")

        icac_issuers = []
        for noc in fabric_b_nocs[0][Clusters.OperationalCredentials].NOCs:
            if noc.icac is None:
                continue
            try:
                icac_data = matter.tlv.TLVReader(noc.icac).get()
                icac_issuer = dict(icac_data.get('Any', [])).get(3)  # Tag 3 = Issuer
                if icac_issuer is not None:
                    icac_issuers.append(icac_issuer)
            except (TypeError, AttributeError):
                # Skip NOCs with unparseable ICAC (e.g., Nullable wrapper types)
                continue

        asserts.assert_true(
            len(icac_issuers) > 0,
            "No valid ICAC Issuer found in Ecosystem B NOCs")

        log.info("Verify the chain of trust between Ecosystem A and Ecosystem B. Issuer of an ICAC for Ecosystem B should match a RCAC Subject from Ecosystem A")
        asserts.assert_true(
            any(issuer in fabric_a_root_subjects for issuer in icac_issuers),
            "No Ecosystem B ICAC Issuer matches any Ecosystem A RCAC Subject"
        )


if __name__ == "__main__":
    default_matter_test_main()
