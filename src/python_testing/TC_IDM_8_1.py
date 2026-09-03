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
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --PICS src/app/tests/suites/certification/ci-pics-values
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio
import logging
import random

from mobly import asserts
from support_modules.idm_support import FabricCheckOutcome, FabricSensitiveEventInfo, IDMBaseTest

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.testing.decorators import async_test_body
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler, EventSubscriptionHandler
from matter.testing.problem_notices import AttributePathLocation, EventPathLocation
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)

# Label written by step 3. The spec constrains fabric labels to 32 characters.
FABRIC_LABEL_TH1 = "TC-IDM-8.1 F1"

# Seconds allowed for a subscription report to arrive, and the settle time used before
# asserting that no report arrived. A report that has not arrived within the settle
# time is treated as absent.
REPORT_TIMEOUT_SEC = 10.0
NO_REPORT_SETTLE_SEC = 5.0


def fabric_label(fabrics: list, fabric_index: int, description: str) -> str:
    """Return the label of the given fabric, failing readably when it is absent."""
    entry = next((f for f in fabrics if f.fabricIndex == fabric_index), None)
    asserts.assert_is_not_none(entry, f"{description} contains no entry for fabric index {fabric_index}")
    return entry.label


class TC_IDM_8_1(IDMBaseTest):
    # This test writes the ACL and other fabric-scoped attributes and adds a second
    # fabric. The background wildcard subscription keeps its own ACL entry and compares
    # reads against an unfiltered cache, both of which this test deliberately disturbs.
    disable_wildcard_subscription = True

    # Steps 4 and 5 subscribe per attribute and per event on every endpoint carrying
    # fabric-scoped data, on top of commissioning a second fabric.
    @property
    def default_timeout(self) -> int:
        return 900

    def teardown_test(self):
        """Undo what the test left on either fabric.

        The framework's cleanup restores only Acl, and a failing step skips any
        in-test cleanup, so this runs here rather than at the end of the test body.
        Two kinds of leftovers: the entries step 4 created through a cluster's own
        command set, and the Extension entries the event triggers wrote.
        """
        try:
            self.event_loop.run_until_complete(self.run_fabric_entry_cleanups())
        except Exception as e:
            # Swallowed for the same reason as the Extension cleanup below: the
            # test's own failure, if any, stays the reported one.
            log.warning("Could not remove the entries created during step 4: %s", e)

        extension_info = getattr(self, 'extension_info', None)
        if extension_info is None or not extension_info.writable:
            return
        for dev_ctrl in (getattr(self, 'th1', None), getattr(self, 'th2', None)):
            if dev_ctrl is None:
                continue
            try:
                self.event_loop.run_until_complete(self.write_fabric_scoped_attribute(extension_info, dev_ctrl, []))
            except Exception as e:
                # Swallowed so the base teardown still runs and the test's own failure,
                # if any, stays the reported one.
                log.warning("Could not clear %s on node %d's fabric: %s", extension_info.path_str, dev_ctrl.nodeId, e)

    def desc_TC_IDM_8_1(self) -> str:
        return "[TC-IDM-8.1] Fabric scoped Test Cases. [{DUT_Server}]"

    def pics_TC_IDM_8_1(self) -> list[str]:
        return ['MCORE.IDM.S']

    @async_test_body
    async def test_TC_IDM_8_1(self):
        self.step(0, "TH1 commissions the DUT, TH1 opens a commissioning window and TH2 commissions the DUT onto a second fabric. TH1 and TH2 read CurrentFabricIndex, stored as F1 and F2.",
                  expectation="The DUT is a member of both fabrics and reports a distinct fabric index to each controller.",
                  is_commissioning=True)
        await self.setup_class_helper(allow_pase=False)
        self.build_spec_xmls()

        self.th1 = self.default_controller
        f1 = await self.read_current_fabric_index(self.th1)

        # TH2 is created on its own certificate authority so that it lands on a separate
        # fabric, then commissions the DUT through a window opened by TH1.
        discriminator = random.randint(0, 4095)
        params = await self.th1.OpenCommissioningWindow(
            nodeId=self.dut_node_id, timeout=900, iteration=10000, discriminator=discriminator, option=1)
        th2_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        th2_fabric_admin = th2_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=self.th1.fabricId + 1)
        self.th2 = th2_fabric_admin.NewController(nodeId=2, useTestCommissioner=True)
        await self.th2.CommissionOnNetwork(
            nodeId=self.dut_node_id, setupPinCode=params.setupPinCode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=discriminator)
        f2 = await self.read_current_fabric_index(self.th2)
        asserts.assert_not_equal(f1, f2, "TH1 and TH2 must be on different fabrics")
        log.info("TH1 is on fabric index %d, TH2 is on fabric index %d", f1, f2)

        attribute_infos = self.discover_fabric_scoped_attributes()
        event_infos = self.discover_fabric_sensitive_events()
        self.record_fabric_metadata_disagreements(attribute_infos, event_infos)
        log.info("Discovered %d fabric-scoped attribute(s) and %d fabric-sensitive event(s) on the DUT",
                 len(attribute_infos), len(event_infos))
        for info in attribute_infos:
            log.info("  attribute %s (writable=%s, fabric-sensitive fields=%s)",
                     info.path_str, info.writable, sorted(info.fabric_sensitive_labels) or "none")
        for event_info in event_infos:
            log.info("  event %s", event_info.path_str)

        # The ACL is the attribute the test plan names for steps 1 and 2 and is mandatory
        # on endpoint 0, so its absence is a DUT problem rather than a reason to skip.
        acl_info = next((i for i in attribute_infos
                         if i.cluster_id == Clusters.AccessControl.id
                         and i.attribute_id == Clusters.AccessControl.Attributes.Acl.attribute_id
                         and i.endpoint_id == 0), None)
        asserts.assert_is_not_none(acl_info, "The ACL attribute was not found on endpoint 0")

        # Kept for teardown_test: the event triggers in steps 5 to 7 write Extension
        # entries that nothing else restores. Absent when the DUT omits the attribute.
        self.extension_info = next((i for i in attribute_infos
                                    if i.cluster_id == Clusters.AccessControl.id
                                    and i.attribute_id == Clusters.AccessControl.Attributes.Extension.attribute_id
                                    and i.endpoint_id == 0), None)

        self.step(1, "TH1 and TH2 each send a Read Request Message to read the ACL attribute, which is a fabric scoped list, from the DUT. Fabric filtered is set to false.",
                  expectation="The number of entries returned to TH1 and TH2 is the same, and the fabric-sensitive fields of the entries belonging to the other fabric are null or contain default values.")
        acl_th1_unfiltered = await self.read_fabric_scoped_attribute(acl_info, self.th1, fabric_filtered=False)
        acl_th2_unfiltered = await self.read_fabric_scoped_attribute(acl_info, self.th2, fabric_filtered=False)
        # Deliberately scoped to the ACL rather than generalised over every
        # fabric-scoped attribute in step 4. The ACL entry struct names individual
        # fabric-sensitive fields, so a cross-fabric entry is still reported, with
        # those fields omitted. An attribute the spec marks fabric sensitive as a
        # whole is reported fabric filtered whatever the request asked for, so the
        # two controllers legitimately see different numbers of entries there.
        asserts.assert_equal(len(acl_th1_unfiltered), len(acl_th2_unfiltered),
                             "TH1 and TH2 must see the same number of ACL entries when not fabric filtered")

        masked_for_th1 = self.assert_other_fabric_entries_masked(acl_info, acl_th1_unfiltered, own_fabric_index=f1)
        masked_for_th2 = self.assert_other_fabric_entries_masked(acl_info, acl_th2_unfiltered, own_fabric_index=f2)
        asserts.assert_greater(masked_for_th1, 0, "TH1's unfiltered ACL read contained no entry from TH2's fabric")
        asserts.assert_greater(masked_for_th2, 0, "TH2's unfiltered ACL read contained no entry from TH1's fabric")

        self.step(2, "TH1 and TH2 each send a fabric-filtered Read Request Message to read the ACL attribute. TH1 then writes an entry in the fabric scoped list associated with its own fabric, and both controllers read the attribute again, fabric filtered.",
                  expectation="The data read by TH2 after the write is the same as the data it read before the write, and the data read by TH1 reflects its modification.")
        acl_th1_before = await self.read_fabric_scoped_attribute(acl_info, self.th1, fabric_filtered=True)
        acl_th2_before = await self.read_fabric_scoped_attribute(acl_info, self.th2, fabric_filtered=True)

        # The ACL is the one attribute this step is required to write, so a DUT that does
        # not offer write access to it fails the step rather than skipping it.
        asserts.assert_true(acl_info.writable, "The DUT does not grant write access to the ACL attribute")

        # Appending to TH1's own ACL rather than replacing it: a replacing write can
        # remove the harness's Administer privilege and lock the rest of the test out.
        added_ace = self.build_view_only_ace()
        await self.add_ace_to_dut_acl(ctrl=self.th1, ace=added_ace, dut_acl_original=acl_th1_before)

        acl_th1_after = await self.read_fabric_scoped_attribute(acl_info, self.th1, fabric_filtered=True)
        acl_th2_after = await self.read_fabric_scoped_attribute(acl_info, self.th2, fabric_filtered=True)
        asserts.assert_equal(acl_th2_after, acl_th2_before,
                             "TH2's fabric-filtered ACL must be unaffected by a write on TH1's fabric")
        asserts.assert_equal(len(acl_th1_after), len(acl_th1_before) + 1,
                             "TH1's fabric-filtered ACL must contain the entry it wrote")
        asserts.assert_equal(acl_th1_after[-1].subjects, added_ace.subjects,
                             "The entry TH1 wrote was not stored as written")
        await self.write_dut_acl(ctrl=self.th1, acl=acl_th1_before)

        self.step(3, "TH1 and TH2 each read the Fabrics attribute of the Operational Credentials cluster with fabric filtered set to false. TH1 invokes UpdateFabricLabel to update the label of its own fabric, and both controllers read Fabrics again.",
                  expectation="The label of the fabric TH1 is on is updated, and the label of the fabric TH2 is on is not modified.")
        fabrics_attribute = Clusters.OperationalCredentials.Attributes.Fabrics
        fabrics_th1_before = await self.read_single_attribute_check_success(
            dev_ctrl=self.th1, endpoint=0, cluster=Clusters.OperationalCredentials, attribute=fabrics_attribute,
            fabric_filtered=False)
        fabrics_th2_before = await self.read_single_attribute_check_success(
            dev_ctrl=self.th2, endpoint=0, cluster=Clusters.OperationalCredentials, attribute=fabrics_attribute,
            fabric_filtered=False)
        label_f2_before = fabric_label(fabrics_th2_before, f2, "TH2's Fabrics read before UpdateFabricLabel")

        await self.th1.SendCommand(nodeId=self.dut_node_id, endpoint=0,
                                   payload=Clusters.OperationalCredentials.Commands.UpdateFabricLabel(
                                       label=FABRIC_LABEL_TH1))

        fabrics_th1_after = await self.read_single_attribute_check_success(
            dev_ctrl=self.th1, endpoint=0, cluster=Clusters.OperationalCredentials, attribute=fabrics_attribute,
            fabric_filtered=False)
        fabrics_th2_after = await self.read_single_attribute_check_success(
            dev_ctrl=self.th2, endpoint=0, cluster=Clusters.OperationalCredentials, attribute=fabrics_attribute,
            fabric_filtered=False)
        asserts.assert_equal(len(fabrics_th1_after), len(fabrics_th1_before),
                             "UpdateFabricLabel must not change the number of fabrics")
        asserts.assert_equal(fabric_label(fabrics_th1_after, f1, "TH1's Fabrics read"), FABRIC_LABEL_TH1,
                             "TH1's fabric label was not updated")
        asserts.assert_equal(fabric_label(fabrics_th2_after, f2, "TH2's Fabrics read"), label_f2_before,
                             "TH2's fabric label must not be modified by TH1")
        # Label is not fabric sensitive, so both controllers observe the same update.
        asserts.assert_equal(fabric_label(fabrics_th2_after, f1, "TH2's Fabrics read"), FABRIC_LABEL_TH1,
                             "TH2 must observe the label TH1 set on its own fabric")

        self.step(4, "For every fabric-scoped attribute discovered on the DUT, TH1 and TH2 read it both fabric filtered and unfiltered, then activate a subscription to it and TH1 modifies it wherever it is writable.",
                  expectation="A fabric-filtered read returns only the reader's own entries, the DUT reports the modified value to both TH1 and TH2, and no response contains fabric-sensitive data belonging to the other fabric.")
        modified = 0
        for info in attribute_infos:
            location = AttributePathLocation(endpoint_id=info.endpoint_id, cluster_id=info.cluster_id,
                                             attribute_id=info.attribute_id)
            # An attribute owned by its cluster's command set has no entry on either
            # fabric until one is created, and every masking assertion below passes
            # trivially on an empty list. Where a safe command sequence is known,
            # give both fabrics an entry first so the checks have something to work
            # against; the entries are removed again in teardown.
            populator = self.fabric_entry_populator(info)
            entries_created = False
            populate_failure_reason = "no command sequence is known that creates an entry on a given fabric"
            if populator is not None:
                populated_th1 = await populator(info, self.th1)
                populated_th2 = await populator(info, self.th2)
                entries_created = populated_th1 and populated_th2
                if not entries_created:
                    populate_failure_reason = ("the DUT would not accept the commands that create an entry, so "
                                               "neither fabric held one")

            # Counted across the reads and the subscription reports alike: a write on
            # TH1's fabric gives TH2's report a cross-fabric entry that the reads, taken
            # before the write, may not have had.
            cross_fabric_entries = 0
            for dev_ctrl, own_index, reader_name in ((self.th1, f1, "TH1"), (self.th2, f2, "TH2")):
                filtered = await self.read_fabric_scoped_attribute(info, dev_ctrl, fabric_filtered=True)
                self.assert_filtered_read_is_own_fabric_only(info, filtered, own_index, reader_name)
                unfiltered = await self.read_fabric_scoped_attribute(info, dev_ctrl, fabric_filtered=False)
                cross_fabric_entries += self.assert_other_fabric_entries_masked(info, unfiltered,
                                                                                own_fabric_index=own_index)
                # An attribute the spec marks fabric sensitive is reported fabric
                # filtered whatever the request asked for, so the other fabric's
                # entries must be absent rather than present with fields omitted.
                # Only checked once both fabrics are known to hold an entry: on an
                # empty list the absence would prove nothing.
                if info.whole_entry_sensitive and entries_created:
                    self.assert_other_fabrics_withheld(info, unfiltered, own_index, reader_name)

            self.record_fabric_check(info.path_str, "fabric-filtered read", FabricCheckOutcome.VERIFIED, location)

            if info.whole_entry_sensitive:
                if entries_created:
                    self.record_fabric_check(info.path_str, "cross-fabric withholding",
                                             FabricCheckOutcome.VERIFIED, location)
                else:
                    self.record_fabric_check(info.path_str, "cross-fabric withholding",
                                             FabricCheckOutcome.NOT_EXERCISED, location, populate_failure_reason)

            skip_reason = self.fabric_write_skip_reason(info)
            if skip_reason is not None:
                payload = None
                self.record_fabric_check(info.path_str, "write and report", FabricCheckOutcome.NOT_APPLICABLE,
                                         location, skip_reason)
            else:
                payload = self.fabric_scoped_write_payload(info, self.th1)
                if payload is None:
                    self.record_fabric_check(info.path_str, "write and report", FabricCheckOutcome.NOT_EXERCISED,
                                             location,
                                             "no value is known that this cluster would accept without "
                                             "cluster-specific state setup")

            if payload is not None:
                original = await self.read_fabric_scoped_attribute(info, self.th1, fabric_filtered=True)
                # Both subscriptions are unfiltered so that each controller also receives a
                # report for the other fabric's change and its masking can be checked.
                handler_th1 = AttributeSubscriptionHandler(expected_cluster=info.cluster_class,
                                                           expected_attribute=info.attribute)
                handler_th2 = AttributeSubscriptionHandler(expected_cluster=info.cluster_class,
                                                           expected_attribute=info.attribute)
                await handler_th1.start(self.th1, self.dut_node_id, info.endpoint_id, fabric_filtered=False)
                await handler_th2.start(self.th2, self.dut_node_id, info.endpoint_id, fabric_filtered=False)
                try:
                    await self.write_fabric_scoped_attribute(info, self.th1, payload)

                    report_th1 = handler_th1.wait_for_attribute_report(timeout_sec=REPORT_TIMEOUT_SEC)
                    report_th2 = handler_th2.wait_for_attribute_report(timeout_sec=REPORT_TIMEOUT_SEC)
                    asserts.assert_true(any(getattr(e, 'fabricIndex', None) == f1 for e in report_th1.value),
                                        f"{info.path_str}: TH1's report contained no entry for its own fabric "
                                        f"after writing one")
                    # TH2 must see that TH1's fabric gained an entry, with its fabric-sensitive
                    # fields masked: the entry's presence is not itself fabric-sensitive data.
                    # Not required where the whole entry is sensitive, because the DUT may
                    # then legitimately omit it from TH2's report rather than mask its fields.
                    if not info.whole_entry_sensitive:
                        asserts.assert_true(any(getattr(e, 'fabricIndex', None) == f1 for e in report_th2.value),
                                            f"{info.path_str}: TH2's report contained no entry for fabric {f1} after "
                                            f"TH1 wrote one")
                    cross_fabric_entries += self.assert_other_fabric_entries_masked(info, report_th1.value,
                                                                                    own_fabric_index=f1)
                    cross_fabric_entries += self.assert_other_fabric_entries_masked(info, report_th2.value,
                                                                                    own_fabric_index=f2)
                    modified += 1
                    self.record_fabric_check(info.path_str, "write and report",
                                             FabricCheckOutcome.VERIFIED, location)
                finally:
                    handler_th1.cancel()
                    handler_th2.cancel()
                    await self.write_fabric_scoped_attribute(info, self.th1, original)

            # Keyed on the attribute's own quality rather than on whether the
            # withholding check ran: masking never applies to an attribute whose
            # entries must be withheld outright, so recording it as unexercised
            # would report the same gap twice under two names.
            if info.whole_entry_sensitive:
                self.record_fabric_check(info.path_str, "cross-fabric masking", FabricCheckOutcome.NOT_APPLICABLE,
                                         location,
                                         "the spec marks the whole attribute fabric sensitive, so its entries are "
                                         "withheld from the other fabric rather than masked")
            elif not info.masked_field_labels:
                self.record_fabric_check(info.path_str, "cross-fabric masking", FabricCheckOutcome.NOT_APPLICABLE,
                                         location,
                                         "the spec marks neither the attribute nor any field of its entry struct "
                                         "fabric sensitive, so no entry carries data to mask")
            elif cross_fabric_entries > 0:
                self.record_fabric_check(info.path_str, "cross-fabric masking", FabricCheckOutcome.VERIFIED, location)
            else:
                self.record_fabric_check(info.path_str, "cross-fabric masking", FabricCheckOutcome.NOT_EXERCISED,
                                         location,
                                         f"no entry belonging to the other fabric appeared in any read or report "
                                         f"and {populate_failure_reason}")

        log.info("Step 4: %d fabric-scoped attribute(s) discovered, %d written and verified",
                 len(attribute_infos), modified)
        self.report_fabric_coverage("Step 4")

        self.step(5, "For every fabric-sensitive event discovered on the DUT that can be triggered, TH1 and TH2 activate a subscription to the event, not fabric filtered, and the event is triggered on the fabric TH1 is on.",
                  expectation="The DUT reports the event to TH1 and does not report it to TH2.")
        # Subscriptions here and in steps 6 and 7 are deliberately not fabric filtered.
        # An event whose associated fabric does not match the accessing fabric is dropped
        # before the fabric filtering flag is consulted (IncludeEventInReport in
        # src/app/EventManagement.cpp), so a filtered request would let a DUT that only
        # suppresses cross-fabric events when asked to filter pass the step.
        triggered: list[FabricSensitiveEventInfo] = []
        for event_info in event_infos:
            event_location = EventPathLocation(endpoint_id=event_info.endpoint_id,
                                               cluster_id=event_info.cluster_id, event_id=event_info.event_id)
            cluster_class = Clusters.ClusterObjects.ALL_CLUSTERS[event_info.cluster_id]
            handler_th1 = EventSubscriptionHandler(expected_cluster=cluster_class)
            handler_th2 = EventSubscriptionHandler(expected_cluster=cluster_class)
            await handler_th1.start(self.th1, self.dut_node_id, event_info.endpoint_id, fabric_filtered=False)
            await handler_th2.start(self.th2, self.dut_node_id, event_info.endpoint_id, fabric_filtered=False)
            try:
                handler_th1.flush_events()
                handler_th2.flush_events()
                if not await self.trigger_fabric_sensitive_event(event_info, self.th1):
                    self.record_fabric_check(event_info.path_str, "event fabric filtering",
                                             FabricCheckOutcome.NOT_EXERCISED, event_location,
                                             "no generic trigger is known for this event, so it never occurred on "
                                             "either fabric")
                    continue
                handler_th1.wait_for_event_type_report(event_info.event, timeout_sec=REPORT_TIMEOUT_SEC)
                await asyncio.sleep(NO_REPORT_SETTLE_SEC)
                self.assert_no_events_for_fabric(handler_th2, f1,
                                                 f"{event_info.path_str}: TH2's subscription")
                triggered.append(event_info)
                self.record_fabric_check(event_info.path_str, "event fabric filtering",
                                         FabricCheckOutcome.VERIFIED, event_location)
            finally:
                handler_th1.cancel()
                handler_th2.cancel()

        log.info("Step 5: %d fabric-sensitive event(s) discovered, %d triggered and verified",
                 len(event_infos), len(triggered))
        self.report_fabric_coverage("Step 5")
        asserts.assert_greater(len(triggered), 0, "No fabric-sensitive event could be triggered on the DUT")

        self.step(6, "TH2 triggers a fabric-sensitive event on its own fabric. TH1 then sends a Subscribe Request Message with EventRequests set to that event path and fabric filtered set to false.",
                  expectation="The DUT sends a Report Data Message with no entry for the event associated with the fabric TH2 is on, and the report carries at least one event.")
        # Steps 6 and 7 need an event that exists on TH2's fabric, so they reuse the
        # events step 5 was able to trigger, this time triggered by TH2.
        for event_info in triggered:
            await self.trigger_fabric_sensitive_event(event_info, self.th2)
            # Trigger on TH1's fabric as well: the check below is only meaningful if the
            # priming report has something to carry, and an event log that has rolled
            # over since step 5 would otherwise leave it empty either way.
            await self.trigger_fabric_sensitive_event(event_info, self.th1)
            handler_th1 = EventSubscriptionHandler(
                expected_cluster=Clusters.ClusterObjects.ALL_CLUSTERS[event_info.cluster_id])
            subscription = await handler_th1.start(self.th1, self.dut_node_id, event_info.endpoint_id,
                                                   fabric_filtered=False)
            try:
                # start() returns once the subscription is established, so the priming
                # report is already in the subscription's event list.
                primed = self.assert_no_subscription_events_for_fabric(
                    subscription, f2, f"{event_info.path_str}: TH1's subscription")
                asserts.assert_greater(primed, 0,
                                       f"{event_info.path_str}: TH1's priming report carried no event at all, so "
                                       f"the absence of fabric {f2}'s event proves nothing")
            finally:
                handler_th1.cancel()

        self.step(7, "TH2 triggers the same fabric-sensitive event on its own fabric. TH1 then sends a Read Request Message with EventRequests set to that event path and fabric filtered set to false.",
                  expectation="The DUT sends a Report Data Message with no entry for the event associated with the fabric TH2 is on, and the response carries at least one event for TH1's own fabric.")
        for event_info in triggered:
            await self.trigger_fabric_sensitive_event(event_info, self.th2)
            # As in step 6, trigger on TH1's fabric as well so an event log that has
            # rolled over cannot make the assertion below pass on an empty response.
            await self.trigger_fabric_sensitive_event(event_info, self.th1)
            events = await self.th1.ReadEvent(
                self.dut_node_id,
                events=[(event_info.endpoint_id, event_info.event)],
                fabricFiltered=False)
            leaked = [e for e in events if getattr(e.Data, 'fabricIndex', None) == f2]
            asserts.assert_equal(leaked, [],
                                 f"{event_info.path_str}: TH1's read returned "
                                 f"{len(leaked)} event(s) for TH2's fabric")
            own = [e for e in events if getattr(e.Data, 'fabricIndex', None) == f1]
            asserts.assert_greater(len(own), 0,
                                   f"{event_info.path_str}: TH1's read returned no event for its own fabric, so "
                                   f"the absence of fabric {f2}'s event proves nothing")


if __name__ == "__main__":
    default_matter_test_main()
