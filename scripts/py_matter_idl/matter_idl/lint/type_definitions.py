# Copyright (c) 2022-2023 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from abc import ABC, abstractmethod
from dataclasses import dataclass, field
from typing import List, MutableMapping, Optional, Union

from matter_idl.matter_idl_types import Idl, ParseMetaData


class MissingIdlError(Exception):
    def __init__(self):
        super().__init__("Missing IDL data")


@dataclass
class LocationInFile:
    file_name: str
    line: Optional[int]
    column: Optional[int]

    def __init__(self, file_name: str, meta: ParseMetaData):
        self.file_name = file_name
        self.line = meta.line
        self.column = meta.column


@dataclass
class LintError:
    """Represents a lint error, potentially at a specific location in a file"""

    message: str
    location: Optional[LocationInFile] = field(default=None)

    def __init__(self, text: str, location: Optional[LocationInFile] = None):
        self.message = text
        if location:
            self.message += " at %s:%d:%d" % (location.file_name,
                                              location.line, location.column)

    def __str__(self):
        return self.message


class LintRule(ABC):
    """Validates a linter rule on an idl"""

    def __init__(self, name):
        self.name = name

    @abstractmethod
    def LintIdl(self, idl: Idl) -> List[LintError]:
        """Runs the linter on the given IDL and returns back any errors it may find"""
        pass


@dataclass
class AttributeRequirement:
    """Contains information about a required attribute"""
    code: int  # required attributes are searched by codes
    name: str  # the name of this attribute. Expect it to be exposed properly

    # Optional filters to apply to specific locations
    filter_cluster: Optional[int] = field(default=None)


@dataclass
class ClusterAttributeDeny:
    cluster_id: Union[str, int]
    attribute_id: Union[str, int]


@dataclass
class ClusterRequirement:
    endpoint_id: int
    cluster_code: int
    cluster_name: str


class ErrorAccumulatingRule(LintRule):
    """Contains a lint error list and helps helpers to add to such a list of rules."""

    def __init__(self, name):
        super(ErrorAccumulatingRule, self).__init__(name)
        self._lint_errors = []
        self._idl = None

    def _AddLintError(self, text, location):
        self._lint_errors.append(
            LintError("%s: %s" % (self.name, text), location))

    def _ParseLocation(self, meta: Optional[ParseMetaData]) -> Optional[LocationInFile]:
        """Create a location in the current file that is being parsed. """
        if not meta or not self._idl or not self._idl.parse_file_name:
            return None
        return LocationInFile(self._idl.parse_file_name, meta)

    def LintIdl(self, idl: Idl) -> List[LintError]:
        self._idl = idl
        self._lint_errors = []
        self._LintImpl()
        return self._lint_errors

    @abstractmethod
    def _LintImpl(self):
        """Implements actual linting of the IDL.

        Uses the underlying _idl for validation.
        """
        pass


class ClusterValidationRule(ErrorAccumulatingRule):
    def __init__(self, name):
        super().__init__(name)
        self._mandatory_clusters: List[ClusterRequirement] = []
        self._rejected_clusters: List[ClusterRequirement] = []

    def __repr__(self):
        result = "ClusterValidationRule{\n"

        if self._mandatory_clusters:
            result += "  mandatory_clusters:\n"
            for cluster in self._mandatory_clusters:
                result += "    - %r\n" % cluster

        if self._rejected_clusters:
            result += "   rejected_clusters:\n"
            for cluster in self._rejected_clusters:
                result += "    - %r\n" % cluster

        result += "}"

        return result

    def RequireClusterInEndpoint(self, requirement: ClusterRequirement):
        self._mandatory_clusters.append(requirement)

    def RejectClusterInEndpoint(self, requirement: ClusterRequirement):
        self._rejected_clusters.append(requirement)

    def _ClusterCode(self, name: str, location: Optional[LocationInFile]):
        """Finds the server cluster definition with the given name.

        On error returns None and _lint_errors is updated internlly
        """
        if not self._idl:
            raise MissingIdlError()

        cluster_definition = [c for c in self._idl.clusters if c.name == name]
        if not cluster_definition:
            self._AddLintError(
                "Cluster definition for %s not found" % name, location)
            return None

        if len(cluster_definition) > 1:
            self._AddLintError(
                "Multiple cluster definitions found for %s" % name, location)
            return None

        return cluster_definition[0].code

    def _LintImpl(self):
        if not self._idl:
            raise MissingIdlError()

        for endpoint in self._idl.endpoints:
            cluster_codes = set()
            for cluster in endpoint.server_clusters:
                cluster_code = self._ClusterCode(
                    cluster.name, self._ParseLocation(cluster.parse_meta))
                if not cluster_code:
                    continue

                cluster_codes.add(cluster_code)

            for requirement in self._mandatory_clusters:
                if requirement.endpoint_id != endpoint.number:
                    continue

                if requirement.cluster_code not in cluster_codes:
                    self._AddLintError("Endpoint %d DOES NOT expose cluster %s (%d)" %
                                       (requirement.endpoint_id, requirement.cluster_name, requirement.cluster_code), location=None)

            for requirement in self._rejected_clusters:
                if requirement.endpoint_id != endpoint.number:
                    continue

                if requirement.cluster_code in cluster_codes:
                    self._AddLintError("Endpoint %d EXPOSES cluster %s (%d)" %
                                       (requirement.endpoint_id, requirement.cluster_name, requirement.cluster_code), location=None)


class RequiredAttributesRule(ErrorAccumulatingRule):
    def __init__(self, name):
        super().__init__(name)
        self._mandatory_attributes: List[AttributeRequirement] = []
        self._deny_attributes: List[ClusterAttributeDeny] = []

    def __repr__(self):
        result = "RequiredAttributesRule{\n"

        if self._mandatory_attributes:
            result += "  mandatory_attributes:\n"
            for attr in self._mandatory_attributes:
                result += "    - %r\n" % attr

        if self._deny_attributes:
            result += "  deny_attributes:\n"
            for attr in self._deny_attributes:
                result += "    - %r\n" % attr

        result += "}"
        return result

    def RequireAttribute(self, attr: AttributeRequirement):
        """Mark an attribute required"""
        self._mandatory_attributes.append(attr)

    def Deny(self, what: ClusterAttributeDeny):
        """Mark a cluster (or cluster/attribute) as denied"""
        self._deny_attributes.append(what)

    def _ServerClusterDefinition(self, name: str, location: Optional[LocationInFile]):
        """Finds the server cluster definition with the given name.

        On error returns None and _lint_errors is updated internlly
        """
        if not self._idl:
            raise MissingIdlError()

        cluster_definition = [c for c in self._idl.clusters if c.name == name]
        if not cluster_definition:
            self._AddLintError(
                "Cluster definition for %s not found" % name, location)
            return None

        if len(cluster_definition) > 1:
            self._AddLintError(
                "Multiple cluster definitions found for %s" % name, location)
            return None

        return cluster_definition[0]

    def _LintImpl(self):
        if not self._idl:
            raise MissingIdlError()

        for endpoint in self._idl.endpoints:

            cluster_codes = set()

            for cluster in endpoint.server_clusters:
                cluster_definition = self._ServerClusterDefinition(
                    cluster.name, self._ParseLocation(cluster.parse_meta))
                if not cluster_definition:
                    continue

                cluster_codes.add(cluster_definition.code)

                # Cluster contains enabled attributes by name
                # cluster_definition contains the definition of the attributes themseves
                #
                # Join the two to receive attribute codes
                name_to_code_map = {}
                for attr in cluster_definition.attributes:
                    name_to_code_map[attr.definition.name] = attr.definition.code

                attribute_codes = set()
                # For all the instantiated attributes, figure out their code
                for attr in cluster.attributes:
                    if attr.name not in name_to_code_map:
                        self._AddLintError("Could not find attribute defintion (no code) for %s:%s" %
                                           (cluster.name, attr.name), self._ParseLocation(cluster.parse_meta))
                        continue

                    attribute_codes.add(name_to_code_map[attr.name])

                # Linting required attributes
                for check in self._mandatory_attributes:
                    if check.filter_cluster is not None and check.filter_cluster != cluster_definition.code:
                        continue

                    if check.code not in attribute_codes:
                        self._AddLintError("EP%d:%s does not expose %s(%d) attribute" %
                                           (endpoint.number, cluster.name,
                                            check.name, check.code),
                                           self._ParseLocation(cluster.parse_meta))

                # Lint rejected attributes
                for check in self._deny_attributes:
                    if check.cluster_id not in [cluster_definition.name, cluster_definition.code]:
                        continue  # different cluster

                    if check.attribute_id is None:
                        self._AddLintError(
                            f"EP{endpoint.number}: cluster {cluster_definition.name}({cluster_definition.code}) is DENIED!",
                            self._ParseLocation(cluster.parse_meta))
                        continue

                    # figure out every attribute that may be denied
                    # We already know every attribute name and have codes
                    for attr in cluster.attributes:
                        if check.attribute_id in [attr.name, name_to_code_map[attr.name]]:
                            cluster_str = f"{cluster_definition.name}({cluster_definition.code})"
                            attribute_str = f"{attr.name}({name_to_code_map[attr.name]})"
                            self._AddLintError(
                                f"EP{endpoint.number}: attribute {cluster_str}::{attribute_str} is DENIED!",
                                self._ParseLocation(cluster.parse_meta))


@dataclass
class ClusterCommandRequirement:
    cluster_code: int
    command_code: int
    command_name: str


class RequiredCommandsRule(ErrorAccumulatingRule):
    def __init__(self, name):
        super(RequiredCommandsRule, self).__init__(name)

        # Maps cluster id to mandatory cluster requirement
        self._mandatory_commands: MutableMapping[int,
                                                 List[ClusterCommandRequirement]] = {}

    def __repr__(self):
        result = "RequiredCommandsRule{\n"

        if self._mandatory_commands:
            result += "  mandatory_commands:\n"
            for key, value in self._mandatory_commands.items():
                result += "    - cluster %d:\n" % key
                for requirement in value:
                    result += "        - %r\n" % requirement

        result += "}"
        return result

    def RequireCommand(self, cmd: ClusterCommandRequirement):
        """Mark a command required"""

        if cmd.cluster_code in self._mandatory_commands:
            self._mandatory_commands[cmd.cluster_code].append(cmd)
        else:
            self._mandatory_commands[cmd.cluster_code] = [cmd]

    def _LintImpl(self):
        if not self._idl:
            raise MissingIdlError()

        for cluster in self._idl.clusters:
            if cluster.code not in self._mandatory_commands:
                continue  # no known mandatory commands

            defined_commands = set([c.code for c in cluster.commands])

            for requirement in self._mandatory_commands[cluster.code]:
                if requirement.command_code in defined_commands:
                    continue  # command exists

                self._AddLintError(
                    "Cluster %s does not define mandatory command %s(%d)" % (
                        cluster.name, requirement.command_name, requirement.command_code),
                    self._ParseLocation(cluster.parse_meta)
                )
