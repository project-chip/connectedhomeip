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

import logging
import re
from collections.abc import Mapping
from importlib.resources.abc import Traversable
from pathlib import Path
from typing import Any

import yaml

import matter.clusters as Clusters
from matter.testing.problem_notices import (AttributePathLocation, ClusterPathLocation, CommandPathLocation, ProblemNotice,
                                            ProblemSeverity, UnknownProblemLocation)
from matter.tlv import uint

LOGGER = logging.getLogger(__name__)


AccessControlEntryPrivilegeEnum = Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum

# Reserved key under a cluster that targets the cluster's own reference revision
# (the `revision` property on the <cluster> XML element, stored as XmlCluster.revision)
# rather than a named attribute or command. This is distinct from the device's runtime
# ClusterRevision global attribute (0xFFFD), which is the value the conformance test
# compares against this reference.
CLUSTER_REVISION_KEY = 'revision'


def parse_errata_access(val: str) -> int:
    """Parses a human-readable access string into an AccessControlEntryPrivilegeEnum value."""
    clean_val = val.strip().lower()
    if clean_val in ['rv', 'view', 'v']:
        return AccessControlEntryPrivilegeEnum.kView
    if clean_val in ['ro', 'operate', 'o']:
        return AccessControlEntryPrivilegeEnum.kOperate
    if clean_val in ['rm', 'manage', 'm']:
        return AccessControlEntryPrivilegeEnum.kManage
    if clean_val in ['ra', 'admin', 'administer', 'a']:
        return AccessControlEntryPrivilegeEnum.kAdminister
    if clean_val in ['none', 'unknown', '']:
        return AccessControlEntryPrivilegeEnum.kUnknownEnumValue
    raise ValueError(f"Unknown access privilege identifier in errata: '{val}'")


def _sanitize_name(name: str) -> str:
    """Removes spaces, slashes, and punctuation, converting to a clean lowercase identifier."""
    return re.sub(r'[\s/\-_]', '', name).lower()


def load_errata_file(errata_path: Path | Traversable) -> dict[str, Any]:
    """Loads and parses raw errata YAML content from a Path or Traversable resource."""
    try:
        if isinstance(errata_path, Path):
            with open(errata_path, encoding='utf-8') as f:
                return yaml.safe_load(f) or {}
        else:
            with errata_path.open('r', encoding='utf-8') as f:
                return yaml.safe_load(f) or {}
    except Exception as e:
        LOGGER.error("Failed to load or parse errata YAML file '%s': %s", errata_path, e)
        return {}


def load_authoritative_errata(errata_target: str | Path | Traversable) -> dict[str, Any]:
    """Resolves and loads an authoritative errata overlay file by name or path."""
    if not isinstance(errata_target, (str, Path)):
        LOGGER.info("Loading errata from Traversable resource %s", errata_target)
        return load_errata_file(errata_target)

    # Load from the local path
    local_path = Path(errata_target)
    if local_path.is_absolute() or local_path.exists():
        LOGGER.info("Loading local errata file from %s", local_path)
        return load_errata_file(local_path)

    LOGGER.error("Errata file path '%s' not found.", errata_target)
    return {}


def _has_no_separators(name: str) -> bool:
    """Returns True if the identifier contains no spaces, slashes, or underscores."""
    return not bool(re.search(r'[\s/\-_]', name))


def _validate_spec_revision(errata_data: dict[str, Any], active_spec_revision: str | None) -> list[str]:
    """Validates active specification revision against the errata compatibility list."""
    if 'compatible_specification_revisions' not in errata_data:
        return []

    compat_list = errata_data['compatible_specification_revisions']
    if not isinstance(compat_list, list):
        return []

    if active_spec_revision is None:
        return [
            "CRITICAL: Errata overlay mandates a compatible specification check, but active_spec_revision was not supplied."]

    str_compat = [str(r).strip() for r in compat_list]
    if not any(active_spec_revision == r or active_spec_revision.startswith(r) for r in str_compat):
        return [
            f"CRITICAL: Errata overlay is not compatible with active Specification revision '{active_spec_revision}'. Compatible list: {str_compat}"]

    return []


def _apply_attribute_errata(target_attribute: Any, overrides: dict[str, Any], cluster_id: int, attr_id: int, context: str, problems: list[ProblemNotice]) -> None:
    """Applies errata overrides to a single XmlAttribute AST object."""
    if 'read_access' in overrides:
        try:
            target_attribute.read_access = parse_errata_access(str(overrides['read_access']))
        except ValueError as e:
            problems.append(ProblemNotice(test_name='Data Model Errata', location=AttributePathLocation(endpoint_id=0, cluster_id=cluster_id, attribute_id=attr_id),
                                          severity=ProblemSeverity.ERROR, problem=f"Invalid read_access on '{context}': {e}"))

    if 'write_access' in overrides:
        try:
            target_attribute.write_access = parse_errata_access(str(overrides['write_access']))
        except ValueError as e:
            problems.append(ProblemNotice(test_name='Data Model Errata', location=AttributePathLocation(endpoint_id=0, cluster_id=cluster_id, attribute_id=attr_id),
                                          severity=ProblemSeverity.ERROR, problem=f"Invalid write_access on '{context}': {e}"))


def _apply_command_errata(target_cmd: Any, overrides: dict[str, Any], cluster_id: int, cmd_id: int, context: str, problems: list[ProblemNotice]) -> None:
    """Applies errata overrides to a single XmlCommand AST object."""
    if 'invoke_access' in overrides or 'privilege' in overrides:
        val = overrides.get('invoke_access') or overrides.get('privilege')
        try:
            target_cmd.privilege = parse_errata_access(str(val))
        except ValueError as e:
            problems.append(ProblemNotice(test_name='Data Model Errata', location=CommandPathLocation(endpoint_id=0, cluster_id=cluster_id, command_id=cmd_id),
                                          severity=ProblemSeverity.ERROR, problem=f"Invalid privilege/invoke_access on '{context}': {e}"))


def _apply_cluster_revision_errata(target_cluster: Any, cluster_id: int, value: Any, cluster_name: str,
                                   problems: list[ProblemNotice]) -> None:
    """Overrides the cluster's reference revision in the AST (XmlCluster.revision).

    The conformance test compares the device-reported ClusterRevision attribute (0xFFFD)
    against this reference value. Bumping it here lets a device report a
    revision newer than the checked-in baseline XML records (e.g. a 'next/in-progress' spec bump).
    """
    try:
        if isinstance(value, (bool, float)):
            raise ValueError
        target_cluster.revision = int(value)
    except (TypeError, ValueError):
        problems.append(ProblemNotice(test_name='Data Model Errata', location=ClusterPathLocation(endpoint_id=0, cluster_id=cluster_id),
                                      severity=ProblemSeverity.ERROR, problem=f"Invalid revision override on '{cluster_name}': {value!r} is not an integer."))


def _apply_element_errata(target_cluster: Any, cluster_id: int, element_name: str, overrides: Any, cluster_name: str,
                          sanitized_attr_map: Mapping[str, int], sanitized_cmd_map: Mapping[str, int],
                          problems: list[ProblemNotice]) -> None:
    """Resolves and applies errata overrides for a single cluster element."""
    if not isinstance(overrides, dict):
        problems.append(ProblemNotice(test_name='Data Model Errata', location=ClusterPathLocation(endpoint_id=0, cluster_id=cluster_id),
                                      severity=ProblemSeverity.ERROR, problem=f"Errata overrides for '{cluster_name}::{element_name}' must be a dictionary."))
        return

    if not _has_no_separators(element_name):
        problems.append(ProblemNotice(test_name='Data Model Errata', location=ClusterPathLocation(endpoint_id=0, cluster_id=cluster_id),
                                      severity=ProblemSeverity.ERROR, problem=f"CRITICAL: Element name '{element_name}' in '{cluster_name}' violates Matter SDK PascalCase conventions. Please use clean sanitized names."))
        return

    san_elem = element_name.lower()
    context = f"{cluster_name}::{element_name}"
    attr_id = sanitized_attr_map.get(san_elem)

    if attr_id is not None:
        target_attribute = target_cluster.attributes.get(attr_id)
        if not target_attribute:
            problems.append(ProblemNotice(test_name='Data Model Errata', location=ClusterPathLocation(endpoint_id=0, cluster_id=cluster_id),
                                          severity=ProblemSeverity.ERROR, problem=f"Attribute ID {attr_id} for '{context}' missing in AST."))
            return
        _apply_attribute_errata(target_attribute, overrides, cluster_id, attr_id, context, problems)
        return

    cmd_id = sanitized_cmd_map.get(san_elem)
    if cmd_id is not None:
        target_cmd = target_cluster.accepted_commands.get(cmd_id) or target_cluster.generated_commands.get(cmd_id)
        if not target_cmd:
            problems.append(ProblemNotice(test_name='Data Model Errata', location=ClusterPathLocation(endpoint_id=0, cluster_id=cluster_id),
                                          severity=ProblemSeverity.ERROR, problem=f"Command ID {cmd_id} for '{context}' missing in accepted/generated AST lists."))
            return
        _apply_command_errata(target_cmd, overrides, cluster_id, cmd_id, context, problems)
        return

    problems.append(ProblemNotice(test_name='Data Model Errata', location=ClusterPathLocation(endpoint_id=0, cluster_id=cluster_id),
                                  severity=ProblemSeverity.ERROR, problem=f"Errata referenced unknown Element '{element_name}' in Cluster '{cluster_name}'."))


def apply_errata(clusters: Mapping[uint, Any], errata_data: dict[str, Any],
                 active_spec_revision: str | None = None) -> list[ProblemNotice]:
    """Applies a dictionary of errata override rules directly to the built in-memory AST.

    Returns a list of ProblemNotice warning/error records for any orphaned, unrecognized, or incompatible rules.
    """
    problems: list[ProblemNotice] = []
    rev_problems = _validate_spec_revision(errata_data, active_spec_revision)
    for rp in rev_problems:
        problems.append(ProblemNotice(test_name='Data Model Errata', location=UnknownProblemLocation(),
                                      severity=ProblemSeverity.ERROR, problem=rp))
    if problems:
        return problems

    for cluster_name, elements in errata_data.items():
        if cluster_name == 'compatible_specification_revisions':
            continue

        if not _has_no_separators(cluster_name):
            problems.append(ProblemNotice(test_name='Data Model Errata', location=UnknownProblemLocation(),
                                          severity=ProblemSeverity.ERROR, problem=f"CRITICAL: Cluster name '{cluster_name}' in errata violates Matter SDK PascalCase conventions. Please use clean sanitized names (e.g. 'OnOff', 'AmbientContextSensing')."))
            continue

        target_cluster_id = next((k for k, c in clusters.items() if _sanitize_name(c.name) == cluster_name.lower()), None)
        if target_cluster_id is None:
            problems.append(ProblemNotice(test_name='Data Model Errata', location=UnknownProblemLocation(),
                                          severity=ProblemSeverity.ERROR, problem=f"Errata referenced unknown Cluster name: '{cluster_name}'"))
            continue

        target_cluster = clusters[target_cluster_id]
        if not isinstance(elements, dict):
            problems.append(ProblemNotice(test_name='Data Model Errata', location=ClusterPathLocation(endpoint_id=0, cluster_id=target_cluster_id),
                                          severity=ProblemSeverity.ERROR, problem=f"Errata definitions for cluster '{cluster_name}' must be a dictionary."))
            continue

        sanitized_attr_map = {_sanitize_name(k): v for k, v in target_cluster.attribute_map.items()}
        sanitized_cmd_map = {_sanitize_name(k): v for k, v in target_cluster.command_map.items()}

        for element_name, overrides in elements.items():
            if element_name.lower() == CLUSTER_REVISION_KEY:
                _apply_cluster_revision_errata(target_cluster, target_cluster_id, overrides, cluster_name, problems)
                continue
            _apply_element_errata(target_cluster, target_cluster_id, element_name, overrides, cluster_name,
                                  sanitized_attr_map, sanitized_cmd_map, problems)

    return problems
