#
#    Copyright (c) 2023 Project CHIP Authors
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
# test-runner-runs: run1
# test-runner-run/run1/app: ${ALL_CLUSTERS_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import inspect
import logging
import random
from dataclasses import dataclass

import chip.clusters as Clusters
import chip.discovery as Discovery
from chip import ChipUtility
from chip.exceptions import ChipStackError
from chip.interaction_model import InteractionModelError, Status
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main, type_matches
from mobly import asserts


def get_all_cmds_for_cluster_id(cid: int) -> list[Clusters.ClusterObjects.ClusterCommand]:
    cluster = Clusters.ClusterObjects.ALL_CLUSTERS[cid]
    try:
        return inspect.getmembers(cluster.Commands, inspect.isclass)
    except AttributeError:
        return []


def client_cmd(cmd_class):
    # Inspect returns all the classes, not just the ones we want, so use a try
    # here incase we're inspecting a builtin class
    try:
        return cmd_class if cmd_class.is_client else None
    except AttributeError:
        return None

# one of the steps in this test requires sending a command that requires a timed interaction
# without first sending the TimedRequest action
# OpenCommissioningWindow requires a timed invoke and is mandatory on servers, BUT, it's marked
# that way in the base class. We need a new, fake class that doesn't have that set


@dataclass
class FakeRevokeCommissioning(Clusters.AdministratorCommissioning.Commands.RevokeCommissioning):
    @ChipUtility.classproperty
    def must_use_timed_invoke(cls) -> bool:
        return False


class TC_IDM_1_2(MatterBaseTest):

    @async_test_body
    async def test_TC_IDM_1_2(self):
        self.print_step(0, "Commissioning - already done")
        wildcard_descriptor = await self.default_controller.ReadAttribute(self.dut_node_id, [(Clusters.Descriptor)])
        endpoints = list(wildcard_descriptor.keys())
        endpoints.sort()

        self.print_step(1, "Send Invoke to unsupported endpoint")
        # First non-existent endpoint is where the index and and endpoint number don't match
        non_existent_endpoint = next(i for i, e in enumerate(endpoints + [None]) if i != e)
        # General Commissioning cluster should be supported on all DUTs, so it will recognize this cluster and
        # command, but it is sent on an unsupported endpoint
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=0, breadcrumb=1)
        try:
            await self.default_controller.SendCommand(nodeid=self.dut_node_id, endpoint=non_existent_endpoint, payload=cmd)
            asserts.fail("Unexpected success return from sending command to unsupported endpoint")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.UnsupportedEndpoint, "Unexpected error returned from unsupported endpoint")

        self.print_step(2, "Send Invoke to unsupported cluster")
        all_cluster_ids = list(Clusters.ClusterObjects.ALL_CLUSTERS.keys())
        unsupported_clusters: dict[int, list[int]] = {}
        supported_clusters: dict[int, list[int]] = {}
        for i in endpoints:
            dut_ep_cluster_ids = wildcard_descriptor[i][Clusters.Descriptor][Clusters.Descriptor.Attributes.ServerList]
            unsupported_clusters[i] = list(set(all_cluster_ids) - set(dut_ep_cluster_ids))
            # We only want to consider the set of standard clusters as "supported clusters", so use the intersection
            supported_clusters[i] = set(dut_ep_cluster_ids).intersection(set(all_cluster_ids))

        # This is really unlikely to happen on any real product, so we're going to assert here if we can't find anything
        # since it's likely a test error
        asserts.assert_true(any(unsupported_clusters[i] for i in endpoints),
                            "Unable to find any unsupported clusters on any endpoint")
        asserts.assert_true(any(supported_clusters[i] for i in endpoints), "Unable to find supported clusters on any endpoint")

        sent = False
        for i in endpoints:
            if sent:
                break
            for cid in unsupported_clusters[i]:
                cluster = Clusters.ClusterObjects.ALL_CLUSTERS[cid]
                members = get_all_cmds_for_cluster_id(cid)
                if not members:
                    continue
                # just use the first command with default values
                name, cmd = members[0]
                logging.info(f'Sending {name} command to unsupported cluster {cluster} on endpoint {i}')
                try:
                    await self.default_controller.SendCommand(nodeid=self.dut_node_id, endpoint=i, payload=cmd())
                    asserts.fail("Unexpected success return from sending command to unsupported cluster")
                except InteractionModelError as e:
                    asserts.assert_equal(e.status, Status.UnsupportedCluster, "Unexpected error returned from unsupported cluster")
                sent = True
                break
        asserts.assert_true(sent, "Unable to find unsupported cluster with commands on any supported endpoint")

        self.print_step(3, "Send Invoke for unsupported command")
        # First read all the supported commands by wildcard reading the AcceptedCommands attribute from all clusters
        # We can't wildcard across clusters even if the attribute is the same, so we're going to go 1 by 1.
        # Just go endpoint by endpoint so we can early exit (each supports different clusters)
        # TODO: add option to make this a beefier test that does all the commands?
        sent = False
        for i in endpoints:
            if sent:
                break
            for cid in supported_clusters[i]:
                cluster = Clusters.ClusterObjects.ALL_CLUSTERS[cid]
                logging.info(f'Checking cluster {cluster} ({cid}) on ep {i} for supported commands')
                members = get_all_cmds_for_cluster_id(cid)
                if not members:
                    continue

                dut_supported_ids = await self.read_single_attribute_check_success(cluster=cluster, endpoint=i, attribute=cluster.Attributes.AcceptedCommandList)
                all_supported_cmds = list(filter(None, [client_cmd(x[1]) for x in members]))
                all_supported_ids = [x.command_id for x in all_supported_cmds]
                unsupported_commands = list(set(all_supported_ids) - set(dut_supported_ids))
                if not unsupported_commands:
                    continue

                # Let's just use the first unsupported command
                id = unsupported_commands[0]
                cmd = next(filter(lambda x: x.command_id == id, all_supported_cmds))
                try:
                    ret = await self.default_controller.SendCommand(nodeid=self.dut_node_id, endpoint=i, payload=cmd())
                    asserts.fail(f'Unexpected success sending unsupported cmd {cmd} to {cluster} cluster on ep {i}')
                except InteractionModelError as e:
                    asserts.assert_equal(e.status, Status.UnsupportedCommand, "Unexpected error returned from unsupported command")
                sent = True
                break

        # It might actually be the case that all the supported clusters support all the commands. In that case, let's just put a warning.
        # We could, in theory, send a command with a fully out of bounds command ID, but that's not supported by the controller
        if not sent:
            logging.warning("Unable to find a supported cluster with unsupported commands on any endpoint - SKIPPING")

        self.print_step(4, "Setup TH to have no privileges for a cluster, send Invoke")
        # Setup the ACL
        acl_only = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[self.matter_test_config.controller_node_id],
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(endpoint=0, cluster=Clusters.AccessControl.id)])
        result = await self.default_controller.WriteAttribute(self.dut_node_id, [(0, Clusters.AccessControl.Attributes.Acl([acl_only]))])
        asserts.assert_equal(result[0].Status, Status.Success, "ACL write failed")

        # For the unsupported access test, let's use a cluster that's known to be there and supports commands - general commissioning on EP0
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=0, breadcrumb=1)
        try:
            await self.default_controller.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=cmd)
            asserts.fail("Unexpected success return when sending a command with no privileges")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.UnsupportedAccess, "Unexpected error returned")

        full_access = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[self.matter_test_config.controller_node_id],
            targets=[])
        result = await self.default_controller.WriteAttribute(self.dut_node_id, [(0, Clusters.AccessControl.Attributes.Acl([full_access]))])
        asserts.assert_equal(result[0].Status, Status.Success, "ACL write failed")

        self.print_step(5, "setup TH with no accessing fabric and invoke command")
        # The only way to have no accessing fabric is to have a PASE session and no added NOC
        # KeySetRead - fabric scoped command, should not be accessible over PASE
        # To get a PASE session, we need an open commissioning window
        discriminator = random.randint(0, 4095)

        params = await self.default_controller.OpenCommissioningWindow(
            nodeid=self.dut_node_id, timeout=600, iteration=10000, discriminator=discriminator, option=1)

        # TH2 = new controller that's not connected over CASE
        new_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        new_fabric_admin = new_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=self.matter_test_config.fabric_id + 1)
        TH2 = new_fabric_admin.NewController(nodeId=112233)

        devices = await TH2.DiscoverCommissionableNodes(
            filterType=Discovery.FilterType.LONG_DISCRIMINATOR, filter=discriminator, stopOnFirst=False)
        # For some reason, the devices returned here aren't filtered, so filter ourselves
        device = next(filter(lambda d: d.commissioningMode == 2 and d.longDiscriminator == discriminator, devices))
        for a in device.addresses:
            try:
                await TH2.EstablishPASESessionIP(ipaddr=a, setupPinCode=params.setupPinCode,
                                                 nodeid=self.dut_node_id+1, port=device.port)
                break
            except ChipStackError:
                continue

        try:
            await TH2.GetConnectedDevice(nodeid=self.dut_node_id+1, allowPASE=True, timeoutMs=1000)
        except TimeoutError:
            asserts.fail("Unable to establish a PASE session to the device")

        try:
            # Any group ID is fine since we'll fail before this
            await TH2.SendCommand(nodeid=self.dut_node_id + 1, endpoint=0, payload=Clusters.GroupKeyManagement.Commands.KeySetRead(groupKeySetID=0x0001))
            asserts.fail("Incorrectly received a success response from a fabric-scoped command")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.UnsupportedAccess, "Incorrect error from fabric-sensitive read over PASE")

        # Cleanup - RevokeCommissioning so we can use ArmFailSafe etc. again.
        await self.default_controller.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=Clusters.AdministratorCommissioning.Commands.RevokeCommissioning(), timedRequestTimeoutMs=6000)

        self.print_step(6, "Send invoke request with requires a data response")
        # ArmFailSafe sends a data response
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=900, breadcrumb=1)
        ret = await self.default_controller.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=cmd)
        asserts.assert_true(type_matches(ret, Clusters.GeneralCommissioning.Commands.ArmFailSafeResponse),
                            "Unexpected response type from ArmFailSafe")

        self.print_step(7, "Send a command with suppress Response")
        # NOTE: This is out of scope currently due to https://github.com/project-chip/connectedhomeip/issues/8043
        # We perform this step, but the DUT will likely incorrectly send a response
        # Sending this command at least ensures the DUT doesn't crash with this flag set, even if the behvaior is not correct

        # Lucky candidate ArmFailSafe is at it again - command side effect is to set breadcrumb attribute
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=900, breadcrumb=2)
        try:
            await self.default_controller.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=cmd, suppressResponse=True)
            # TODO: Once the above issue is resolved, this needs a check to ensure that (always) no response was received.
        except ChipStackError:
            logging.info("DUT correctly supressed the response")

        # Verify that the command had the correct side effect even if a response was sent
        breadcrumb = await self.read_single_attribute_check_success(
            cluster=Clusters.GeneralCommissioning, attribute=Clusters.GeneralCommissioning.Attributes.Breadcrumb, endpoint=0)
        asserts.assert_equal(breadcrumb, 2, "Breadcrumb was not correctly set on ArmFailSafe with response suppressed")

        # Cleanup - Unset the failsafe
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=0, breadcrumb=0)
        await self.default_controller.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=cmd)

        self.print_step(8, "Send Invoke with timedRequest marked, but no timed request sent")
        # We can do this with any command, but to be thorough, test first with a command that does not
        # require a timed interaction (ArmFailSafe) and then one that does (RevokeCommissioning)
        try:
            await self.default_controller.TestOnlySendCommandTimedRequestFlagWithNoTimedInvoke(nodeid=self.dut_node_id, endpoint=0, payload=cmd)
            asserts.fail("Unexpected success response from sending an Invoke with TimedRequest flag and no timed interaction")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.TimedRequestMismatch,
                                 "Unexpected error response from Invoke with TimedRequest flag and no TimedInvoke")

        # Try with RevokeCommissioning
        # First open a commissioning window for us to revoke, so we know this command is able to succeed absent this error
        _ = await self.default_controller.OpenCommissioningWindow(
            nodeid=self.dut_node_id, timeout=600, iteration=10000, discriminator=discriminator, option=1)
        cmd = FakeRevokeCommissioning()
        try:
            await self.default_controller.TestOnlySendCommandTimedRequestFlagWithNoTimedInvoke(nodeid=self.dut_node_id, endpoint=0, payload=cmd)
            asserts.fail("Unexpected success response from sending an Invoke with TimedRequest flag and no timed interaction")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.TimedRequestMismatch,
                                 "Unexpected error response from Invoke with TimedRequest flag and no TimedInvoke")

        self.print_step(9, "Send invoke for a command that requires timedRequest, but doesn't use one")
        # RevokeCommissioning requires a timed interaction. This is enforced in the python layer because
        # the generated class indicates that a timed interaction is required. The fake class overrides this.
        try:
            await self.default_controller.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=cmd)
            asserts.fail("Incorrectly received a success response for a command that required TimedInvoke action")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.NeedsTimedInteraction)

        # Cleanup - actually revoke commissioning to close the open window
        await self.default_controller.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=Clusters.AdministratorCommissioning.Commands.RevokeCommissioning(), timedRequestTimeoutMs=6000)


if __name__ == "__main__":
    default_matter_test_main()
