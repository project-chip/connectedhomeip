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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --endpoint 0
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
#   run2:
#     app: ${LIT_ICD_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --endpoint 0
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
#   run3:
#     app: ${CAMERA_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --endpoint 0
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

'''
Purpose:
Validate the MatterBaseTest cleanup framework.

Each test leaves the DUT in a known dirty state. This class overrides teardown_test()
to run framework cleanup after every test, so the next test starts from a clean state.

This is intentional. These scenarios must be isolated from each other. In normal test
classes, framework cleanup runs once in teardown_class(), not between individual tests.

Scenarios covered:
  - test_acl_cleanup:                   adds an extra ACL entry
  - test_commissioning_window_cleanup:  leaves an open Enhanced Commissioning Window
  - test_extra_fabric_cleanup:          commissions DUT onto a second fabric via TH2
  - test_failsafe_cleanup:              leaves a failsafe armed
  - test_group_key_cleanup:             adds a group key set and group key mapping
  - test_group_membership_cleanup:      adds a group membership via the Groups cluster
  - test_scenes_cleanup:                adds a group membership and a scene
  - test_doorlock_cleanup:              adds a DoorLock PIN credential and user
                                        (skipped if DoorLock cluster absent)
  - test_icd_client_cleanup:            registers an ICD client
                                        (skipped if IcdManagement cluster absent)
  - test_tls_endpoints_cleanup:         provisions a TLS root certificate and endpoint
                                        (skipped if TlsClientManagement cluster absent)
'''

import datetime
import logging

from cryptography import x509
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.x509.oid import NameOID

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.clusters.Types import NullValue
from matter.testing.decorators import _has_attribute, _has_cluster, async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import default_matter_test_main

logger = logging.getLogger(__name__)


class TestCleanupFramework(MatterBaseTest):

    def desc_TestCleanupFramework(self) -> str:
        return "[TestCleanupFramework] Validates the MatterBaseTest cleanup framework"

    @async_test_body
    async def teardown_test(self):
        """Invoke framework cleanup after each test to isolate scenarios from one another.

        The default framework behavior runs cleanup once in teardown_class. This class
        overrides teardown_test to trigger cleanup per test instead, because each
        scenario leaves the DUT in a specific dirty state that would otherwise
        interfere with subsequent test methods.
        """
        await self._run_framework_cleanup()
        super().teardown_test()

    async def _find_endpoint_with_cluster(self, cluster) -> int | None:
        """Returns the first endpoint that has the given cluster, or None."""
        await self._populate_wildcard()
        for ep in self.stored_global_wildcard.attributes:
            if _has_cluster(self.stored_global_wildcard, ep, cluster):
                return ep
        return None

    @async_test_body
    async def test_acl_cleanup(self):
        logger.info("--- Scenario: ACL modification ---")
        current_acl = await self.read_single_attribute_check_success(
            cluster=Clusters.AccessControl,
            attribute=Clusters.AccessControl.Attributes.Acl,
            endpoint=0
        )
        extra_entry = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[0xFFFF_FFFD_0001_0001],
            targets=NullValue,
        )
        logger.info("Adding an extra ACL entry")
        await self.default_controller.WriteAttribute(
            self.dut_node_id,
            [(0, Clusters.AccessControl.Attributes.Acl(current_acl + [extra_entry]))]
        )
        logger.info("ACL entry added — leaving ACL modified")
        logger.info("Expected: cleanup framework restores original ACL via _reset_acls_to_default")

    @async_test_body
    async def test_commissioning_window_cleanup(self):
        logger.info("--- Scenario: open commissioning window ---")
        logger.info("Opening an Enhanced Commissioning Window and leaving it open")
        ecw = await self.open_commissioning_window(dev_ctrl=self.default_controller, node_id=self.dut_node_id)
        logger.info(f"Window open: discriminator={ecw.randomDiscriminator}, "
                    f"pin={ecw.commissioningParameters.setupPinCode}")
        logger.info("Expected: cleanup framework closes this window via _close_commissioning_windows")

    @async_test_body
    async def test_extra_fabric_cleanup(self):
        logger.info("--- Scenario: extra fabric ---")
        logger.info("Creating TH2 on a new fabric")
        th2_ca = self.certificate_authority_manager.NewCertificateAuthority()
        th2_fabric_admin = th2_ca.NewFabricAdmin(vendorId=0xFFF1, fabricId=self.default_controller.fabricId + 1)
        th2 = th2_fabric_admin.NewController(nodeId=2, useTestCommissioner=True)

        logger.info("TH1 opening Enhanced Commissioning Window for TH2")
        ecw = await self.open_commissioning_window(dev_ctrl=self.default_controller, node_id=self.dut_node_id)
        th2_node_id = self.dut_node_id + 1

        logger.info(f"TH2 commissioning DUT (th2_node_id={th2_node_id:#x})")
        await th2.CommissionOnNetwork(
            nodeId=th2_node_id,
            setupPinCode=ecw.commissioningParameters.setupPinCode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=ecw.randomDiscriminator
        )

        logger.info("TH2 commissioned successfully")
        logger.info("Expected: cleanup framework auto-discovers and removes TH2's fabric via "
                    "_remove_extra_fabrics, and shuts down TH2 and its CA via _shutdown_extra_controllers")

    @async_test_body
    async def test_failsafe_cleanup(self):
        logger.info("--- Scenario: armed failsafe ---")
        expiry = 30
        logger.info(f"Arming failsafe with expiryLengthSeconds={expiry}")
        resp = await self.send_single_cmd(
            cmd=Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=expiry),
            endpoint=0
        )
        logger.info(f"ArmFailSafe response: errorCode={resp.errorCode}")
        logger.info("Expected: cleanup framework disarms this failsafe via _disarm_failsafes")

    @async_test_body
    async def test_group_key_cleanup(self):
        logger.info("--- Scenario: group key set and mapping (GroupKeyManagement) ---")
        key_set_id = 1
        epoch_key = b'\x00' * 16

        logger.info(f"Writing group key set groupKeySetID={key_set_id}")
        await self.send_single_cmd(
            cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(
                groupKeySet=Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
                    groupKeySetID=key_set_id,
                    groupKeySecurityPolicy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
                    epochKey0=epoch_key,
                    epochStartTime0=1,
                    epochKey1=NullValue,
                    epochStartTime1=NullValue,
                    epochKey2=NullValue,
                    epochStartTime2=NullValue,
                )
            ),
            endpoint=0
        )

        logger.info("Writing group key mapping: group 1 -> key set 1")
        await self.default_controller.WriteAttribute(
            self.dut_node_id,
            [(0, Clusters.GroupKeyManagement.Attributes.GroupKeyMap([
                Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(groupId=1, groupKeySetID=key_set_id)
            ]))]
        )
        logger.info(f"Group key set {key_set_id} and mapping added")
        logger.info("Expected: cleanup framework removes key sets and mappings via _purge_groups")

    @async_test_body
    async def test_group_membership_cleanup(self):
        logger.info("--- Scenario: group membership (Groups cluster) ---")
        ep = await self._find_endpoint_with_cluster(Clusters.Groups)
        if ep is None:
            logger.info("Groups cluster not present on DUT — skipping")
            return

        group_id = 0x0001
        logger.info(f"Adding group {group_id:#x} on endpoint {ep}")
        await self.send_single_cmd(
            cmd=Clusters.Groups.Commands.AddGroup(groupID=group_id, groupName="cleanup-test"),
            endpoint=ep
        )
        logger.info(f"Group {group_id:#x} added on endpoint {ep}")
        logger.info("Expected: cleanup framework removes all group memberships via _purge_group_memberships")

    @async_test_body
    async def test_scenes_cleanup(self):
        logger.info("--- Scenario: scene entry (ScenesManagement cluster) ---")
        ep = await self._find_endpoint_with_cluster(Clusters.ScenesManagement)
        if ep is None:
            logger.info("ScenesManagement cluster not present on DUT — skipping")
            return
        if not _has_cluster(self.stored_global_wildcard, ep, Clusters.Groups):
            logger.info(f"Groups cluster not present on endpoint {ep} — skipping scenes scenario")
            return

        group_id = 0x0002
        scene_id = 0x01

        logger.info(f"Adding group {group_id:#x} on endpoint {ep}")
        await self.send_single_cmd(
            cmd=Clusters.Groups.Commands.AddGroup(groupID=group_id, groupName="scenes-test"),
            endpoint=ep
        )

        logger.info(f"Storing scene {scene_id:#x} for group {group_id:#x} on endpoint {ep}")
        await self.send_single_cmd(
            cmd=Clusters.ScenesManagement.Commands.StoreScene(groupID=group_id, sceneID=scene_id),
            endpoint=ep
        )
        logger.info(f"Scene {scene_id:#x} stored for group {group_id:#x}")
        logger.info("Expected: cleanup framework removes scenes via _purge_scenes, "
                    "then group membership via _purge_group_memberships")

    @async_test_body
    async def test_doorlock_cleanup(self):
        logger.info("--- Scenario: DoorLock credential and user ---")
        ep = await self._find_endpoint_with_cluster(Clusters.DoorLock)
        if ep is None:
            logger.info("DoorLock cluster not present on DUT — skipping")
            return

        logger.info(f"Setting PIN credential on DoorLock endpoint {ep}")
        await self.send_single_cmd(
            cmd=Clusters.DoorLock.Commands.SetCredential(
                operationType=Clusters.DoorLock.Enums.DataOperationTypeEnum.kAdd,
                credential=Clusters.DoorLock.Structs.CredentialStruct(
                    credentialType=Clusters.DoorLock.Enums.CredentialTypeEnum.kPin,
                    credentialIndex=1
                ),
                credentialData=b'1234',
                userIndex=NullValue,
                userStatus=NullValue,
                userType=NullValue,
            ),
            endpoint=ep,
            timedRequestTimeoutMs=1000
        )
        logger.info("PIN credential set (user auto-created by DUT)")
        logger.info("Expected: cleanup framework clears all credentials and users via _purge_doorlock")

    @async_test_body
    async def test_icd_client_cleanup(self):
        logger.info("--- Scenario: ICD client registration ---")
        await self._populate_wildcard()
        if not _has_attribute(wildcard=self.stored_global_wildcard, endpoint=0,
                              attribute=Clusters.IcdManagement.Attributes.RegisteredClients):
            logger.info("ICD Management cluster not present on DUT — skipping")
            return

        check_in_node_id = self.default_controller.nodeId
        logger.info(f"Registering ICD client checkInNodeID={check_in_node_id:#x}")
        await self.send_single_cmd(
            cmd=Clusters.IcdManagement.Commands.RegisterClient(
                checkInNodeID=check_in_node_id,
                monitoredSubject=check_in_node_id,
                key=b'\x01' * 16,
                clientType=Clusters.IcdManagement.Enums.ClientTypeEnum.kEphemeral,
            ),
            endpoint=0
        )
        logger.info(f"ICD client {check_in_node_id:#x} registered")
        logger.info("Expected: cleanup framework unregisters this client via _unregister_icd_clients")

    @async_test_body
    async def test_tls_endpoints_cleanup(self):
        logger.info("--- Scenario: TLS endpoint provisioning (TlsClientManagement) ---")
        ep = await self._find_endpoint_with_cluster(Clusters.TlsClientManagement)
        if ep is None:
            logger.info("TlsClientManagement cluster not present on DUT — skipping")
            return
        if not _has_cluster(self.stored_global_wildcard, ep, Clusters.TlsCertificateManagement):
            logger.info(f"TlsCertificateManagement cluster not present on endpoint {ep} — skipping")
            return

        # Generate a minimal self-signed root certificate for provisioning
        key = ec.generate_private_key(ec.SECP256R1())
        subject = x509.Name([x509.NameAttribute(NameOID.COMMON_NAME, "cleanup-test-root")])
        cert_bytes = (
            x509.CertificateBuilder()
            .subject_name(subject)
            .issuer_name(subject)
            .public_key(key.public_key())
            .serial_number(x509.random_serial_number())
            .not_valid_before(datetime.datetime.now(datetime.UTC))
            .not_valid_after(datetime.datetime.now(datetime.UTC) + datetime.timedelta(days=365))
            .add_extension(x509.BasicConstraints(ca=True, path_length=0), critical=True)
            .sign(key, hashes.SHA256())
            .public_bytes(serialization.Encoding.DER)
        )

        logger.info(f"Provisioning root certificate on endpoint {ep}")
        resp = await self.send_single_cmd(
            cmd=Clusters.TlsCertificateManagement.Commands.ProvisionRootCertificate(
                certificate=cert_bytes,
                caid=NullValue,
            ),
            endpoint=ep,
            payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD,
        )
        caid = resp.caid
        logger.info(f"Root certificate provisioned with CAID={caid:#x}")

        logger.info(f"Provisioning TLS endpoint with CAID={caid:#x} on endpoint {ep}")
        await self.send_single_cmd(
            cmd=Clusters.TlsClientManagement.Commands.ProvisionEndpoint(
                hostname=b"cleanup-test.example.com",
                port=443,
                caid=caid,
                ccdid=NullValue,
                endpointID=NullValue,
            ),
            endpoint=ep,
            payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD,
        )
        logger.info("TLS endpoint provisioned and left open")
        logger.info("Expected: cleanup framework removes it via _purge_tls_endpoints")


if __name__ == "__main__":
    default_matter_test_main()
