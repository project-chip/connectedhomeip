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

"""Diagnostic sweep of in-range attribute writes.

TC-IDM-9.1 writes values just *outside* each spec constraint and requires the DUT to
answer CONSTRAINT_ERROR. This sweep is its inverse: it writes values sitting exactly
*on* each bound, values the spec permits, and reports the ones the DUT refuses.

It is a diagnostic, not a certification test. Acceptance of an in-range value is not
universally required: other spec rules can legitimately reject a value that sits on a
constraint bound (Valve DefaultOpenLevel has a min of 1, but with LevelStep 2 the
device must still reject 1), and roughly a quarter of the attribute constraints in the
data model XML are prose-only <desc/> forms this sweep cannot reason about. So a
rejection here is a lead to investigate by hand, not a verdict. The sweep passes
regardless of what it finds unless it is run with --bool-arg fail_on_rejected:true.

Every accepted write mutates the DUT, so each probe restores the attribute's prior
value immediately. Attributes whose value could not be restored are called out
separately in the report: the device is left dirty and should be factory reset before
running anything else against it.

Example:
    python3 src/python_testing/AttributeBoundarySweep.py --commissioning-method on-network \\
        --discriminator 1234 --passcode 20202021 --storage-path admin_storage.json
"""

import logging
from dataclasses import dataclass, field
from enum import Enum

from mobly import asserts
from support_modules.idm_support import (COMMAND_CONSTRAINT_DENIED_CLUSTERS, ConstraintProbe, IDMBaseTest, ProbeMode,
                                         WritableAttributeInfo)

import matter.clusters as Clusters
from matter.interaction_model import Status
from matter.testing.decorators import async_test_body
from matter.testing.global_attribute_ids import is_standard_cluster_id
from matter.testing.matter_testing import TestStep
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)

# A write the DUT accepts changes device state, so the sweep inherits the command
# fuzzing deny list (fabric and credential mutation, network credentials, commissioning
# windows, OTA flows, ACLs, group keys) and adds the clusters whose writable attributes
# reconfigure the transport the sweep itself depends on.
BOUNDARY_SWEEP_DENIED_CLUSTERS = COMMAND_CONSTRAINT_DENIED_CLUSTERS | frozenset({
    Clusters.IcdManagement.id,
    Clusters.TimeSynchronization.id,
})


class ProbeOutcome(Enum):
    """What the DUT did with a value written at one of its spec constraint bounds."""

    # Write succeeded and the attribute read back the written value.
    ACCEPTED = 'accepted'
    # CONSTRAINT_ERROR on a value the spec permits. This is what the sweep looks for.
    REJECTED = 'rejected'
    # Write succeeded but the attribute did not take the value (clamped or discarded).
    IGNORED = 'ignored'
    # Any other status. State-gated writes (INVALID_IN_STATE and friends) land here.
    OTHER_STATUS = 'other status'
    # The attribute already held the bound value, so the write would prove nothing.
    ALREADY_AT_BOUND = 'already at bound'
    # An exception was raised while probing, e.g. the value could not be encoded.
    ERROR = 'error'


@dataclass
class ProbeResult:
    """Outcome of writing one boundary value to one attribute on one endpoint."""
    endpoint_id: int
    cluster_name: str
    attribute_name: str
    probe: ConstraintProbe
    outcome: ProbeOutcome
    status_name: str = ''
    detail: str = ''
    # True when the probe changed the attribute and the original value could not be put
    # back; the DUT is left holding a value the sweep wrote.
    restore_failed: bool = False

    @property
    def path_str(self) -> str:
        return f'EP{self.endpoint_id} {self.cluster_name}.{self.attribute_name}'


@dataclass
class SweepTally:
    """Counts of each outcome plus the results worth printing individually."""
    counts: dict[ProbeOutcome, int] = field(default_factory=dict)
    notable: list[ProbeResult] = field(default_factory=list)
    dirty: list[ProbeResult] = field(default_factory=list)


class AttributeBoundarySweep(IDMBaseTest):
    @property
    def default_timeout(self) -> int:
        return 900

    def desc_AttributeBoundarySweep(self) -> str:
        return "[Diagnostic] Write values at spec constraint bounds and report the ones the DUT rejects"

    def steps_AttributeBoundarySweep(self) -> list[TestStep]:
        return [
            TestStep(0, "Commissioning, already done", is_commissioning=True),
            TestStep(1, "Walk every endpoint of the DUT and collect the writable attributes of every standard "
                        "server cluster that carries a machine-readable spec constraint, excluding clusters whose "
                        "writes could break the test session. Resolve constraint bounds that reference another "
                        "attribute by reading that attribute from the DUT.",
                     "A set of attributes and, for each, the values sitting exactly on its constraint bounds."),
            TestStep(2, "Write each boundary value to its attribute, read the attribute back to confirm the write "
                        "took effect, and restore the attribute's prior value.",
                     "Each write is recorded as accepted, rejected with CONSTRAINT_ERROR, silently ignored, or "
                        "answered with some other status."),
            TestStep(3, "Report the tally and list every attribute that rejected or ignored an in-range value, "
                        "along with any attribute whose original value could not be restored.",
                     "A report for manual triage. The sweep does not fail on rejections unless run with "
                        "--bool-arg fail_on_rejected:true."),
        ]

    @async_test_body
    async def test_AttributeBoundarySweep(self):
        self.step(0)
        await self.setup_class_helper(allow_pase=False)
        self.build_spec_xmls()

        self.step(1)
        attributes = self._collect_constrained_writable_attributes()
        log.info("Found %s writable attributes with constraints", len(attributes))

        self.step(2)
        results: list[ProbeResult] = []
        for attr_info in attributes:
            results.extend(await self._probe_attribute(attr_info))

        self.step(3)
        tally = self._tally(results)
        self._log_report(tally, len(attributes))

        asserts.assert_greater(len(results), 0,
                               "No boundary values could be generated for any attribute on this DUT; "
                               "the sweep exercised nothing")

        if self.user_params.get('fail_on_rejected', False):
            rejected = [r for r in tally.notable if r.outcome is ProbeOutcome.REJECTED]
            if rejected:
                asserts.fail("DUT rejected in-range values for: "
                             + ', '.join(f'{r.path_str} ({r.probe.description})' for r in rejected))

    def _collect_constrained_writable_attributes(self) -> list[WritableAttributeInfo]:
        """Collect the writable attributes carrying spec constraints, deny list applied."""
        attributes: list[WritableAttributeInfo] = []
        for endpoint_id, endpoint in self.endpoints_tlv.items():
            for cluster_id, device_cluster_data in endpoint.items():
                if not is_standard_cluster_id(cluster_id):
                    continue
                if cluster_id not in self.xml_clusters or cluster_id not in Clusters.ClusterObjects.ALL_ATTRIBUTES:
                    continue
                if cluster_id in BOUNDARY_SWEEP_DENIED_CLUSTERS:
                    log.info("Skipping cluster 0x%04X on EP%s: deny-listed for boundary writes", cluster_id, endpoint_id)
                    continue

                xml_cluster = self.xml_clusters[cluster_id]
                cluster_class = Clusters.ClusterObjects.ALL_CLUSTERS[cluster_id]

                for attribute_id in self.checkable_attributes(cluster_id, device_cluster_data, xml_cluster):
                    xml_attr = xml_cluster.attributes[attribute_id]
                    if xml_attr.write_access == Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kUnknownEnumValue:
                        continue
                    if not xml_attr.constraints or not xml_attr.constraints.has_constraints():
                        continue

                    attributes.append(WritableAttributeInfo(
                        endpoint_id=endpoint_id,
                        cluster_id=cluster_id,
                        cluster_name=xml_cluster.name,
                        attribute_id=attribute_id,
                        attribute_name=xml_attr.name,
                        attribute=Clusters.ClusterObjects.ALL_ATTRIBUTES[cluster_id][attribute_id],
                        cluster_class=cluster_class,
                        datatype=xml_attr.datatype,
                        constraints=xml_attr.constraints,
                    ))
        return attributes

    async def _probe_attribute(self, attr_info: WritableAttributeInfo) -> list[ProbeResult]:
        """Write every boundary value of one attribute, restoring state after each."""
        constraints = attr_info.constraints
        # Bounds expressed as a reference to another attribute are resolved against the
        # live DUT; an unresolvable reference leaves that bound unset and it is skipped.
        if constraints.min_value_ref:
            constraints.min_value = await self.resolve_dynamic_constraint(
                attr_info.cluster_class, attr_info.endpoint_id, constraints.min_value_ref)
        if constraints.max_value_ref:
            constraints.max_value = await self.resolve_dynamic_constraint(
                attr_info.cluster_class, attr_info.endpoint_id, constraints.max_value_ref)

        probes = self.generate_constraint_probes(attr_info, constraints, ProbeMode.AT_BOUND)
        results = []
        for probe in probes:
            try:
                probe_result = await self._write_and_restore(attr_info, probe)
                # Log every probe as it happens, not just the notable ones: a sweep is only
                # useful for triage if the accepted writes are visible alongside the rest.
                log.info("%-16s %s: %s", probe_result.outcome.value.upper(), probe_result.path_str,
                         probe_result.detail)
                results.append(probe_result)
            except Exception as e:
                log.warning("Error probing %s.%s with %s: %s", attr_info.cluster_name, attr_info.attribute_name,
                            probe.value, e)
                results.append(ProbeResult(
                    endpoint_id=attr_info.endpoint_id, cluster_name=attr_info.cluster_name,
                    attribute_name=attr_info.attribute_name, probe=probe,
                    outcome=ProbeOutcome.ERROR, detail=str(e)))
        return results

    async def _write_and_restore(self, attr_info: WritableAttributeInfo, probe: ConstraintProbe) -> ProbeResult:
        """Write one boundary value, classify the response, and put the old value back."""
        def result(outcome: ProbeOutcome, **kwargs) -> ProbeResult:
            return ProbeResult(endpoint_id=attr_info.endpoint_id, cluster_name=attr_info.cluster_name,
                               attribute_name=attr_info.attribute_name, probe=probe, outcome=outcome, **kwargs)

        original = await self.read_single_attribute_check_success(
            endpoint=attr_info.endpoint_id, cluster=attr_info.cluster_class, attribute=attr_info.attribute)

        if original == probe.value:
            return result(ProbeOutcome.ALREADY_AT_BOUND, detail=f'attribute already reads {original}')

        write_result = await self.default_controller.WriteAttribute(
            nodeId=self.dut_node_id, attributes=[(attr_info.endpoint_id, attr_info.attribute(probe.value))])
        status = write_result[0].Status

        if status != Status.Success:
            outcome = ProbeOutcome.REJECTED if status == Status.ConstraintError else ProbeOutcome.OTHER_STATUS
            # Status is an IntEnum and formats as a bare number, so log the name too.
            return result(outcome, status_name=getattr(status, 'name', str(status)),
                          detail=f'{probe.description}, value {probe.value!r}')

        stored = await self.read_single_attribute_check_success(
            endpoint=attr_info.endpoint_id, cluster=attr_info.cluster_class, attribute=attr_info.attribute)
        outcome = ProbeOutcome.ACCEPTED if stored == probe.value else ProbeOutcome.IGNORED
        detail = f'{probe.description}, value {probe.value!r}'
        if outcome is ProbeOutcome.IGNORED:
            detail += f', attribute reads {stored!r}'

        restore_failed = False
        if stored != original:
            restore_result = await self.default_controller.WriteAttribute(
                nodeId=self.dut_node_id, attributes=[(attr_info.endpoint_id, attr_info.attribute(original))])
            if restore_result[0].Status != Status.Success:
                restore_failed = True
                log.error("Could not restore %s.%s on EP%s to %r: %s", attr_info.cluster_name,
                          attr_info.attribute_name, attr_info.endpoint_id, original,
                          getattr(restore_result[0].Status, 'name', restore_result[0].Status))

        return result(outcome, status_name=Status.Success.name, detail=detail, restore_failed=restore_failed)

    @staticmethod
    def _tally(results: list[ProbeResult]) -> SweepTally:
        """Group results by outcome, separating the ones needed to look at."""
        tally = SweepTally()
        for outcome in ProbeOutcome:
            tally.counts[outcome] = 0
        for probe_result in results:
            tally.counts[probe_result.outcome] += 1
            if probe_result.outcome in (ProbeOutcome.REJECTED, ProbeOutcome.IGNORED, ProbeOutcome.OTHER_STATUS,
                                        ProbeOutcome.ERROR):
                tally.notable.append(probe_result)
            if probe_result.restore_failed:
                tally.dirty.append(probe_result)
        return tally

    @staticmethod
    def _log_report(tally: SweepTally, attribute_count: int) -> None:
        """Print the sweep report: tally, then every result worth attention."""
        total = sum(tally.counts.values())
        log.info("=== Boundary sweep: %s probes across %s attributes ===", total, attribute_count)
        for outcome, count in tally.counts.items():
            log.info("  %-18s %s", outcome.value, count)

        rejected = [r for r in tally.notable if r.outcome is ProbeOutcome.REJECTED]
        if rejected:
            log.info("--- In-range values the DUT rejected with CONSTRAINT_ERROR ---")
            for probe_result in rejected:
                log.info("  %s: %s", probe_result.path_str, probe_result.detail)

        others = [r for r in tally.notable if r.outcome is not ProbeOutcome.REJECTED]
        if others:
            log.info("--- Other results worth review ---")
            for probe_result in others:
                log.info("  %s [%s%s]: %s", probe_result.path_str, probe_result.outcome.value,
                         f' {probe_result.status_name}' if probe_result.status_name else '', probe_result.detail)

        if tally.dirty:
            log.error("--- DUT left dirty: original value could not be restored ---")
            for probe_result in tally.dirty:
                log.error("  %s now holds %r; factory reset before the next run", probe_result.path_str,
                          probe_result.probe.value)


if __name__ == "__main__":
    default_matter_test_main()
