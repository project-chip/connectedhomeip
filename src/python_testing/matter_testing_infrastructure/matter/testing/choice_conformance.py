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

from matter.testing.conformance import Choice, ConformanceAssessmentData, ConformanceDecisionWithChoice
from matter.testing.global_attribute_ids import GlobalAttributeIds
from matter.testing.problem_notices import AttributePathLocation, ProblemNotice, ProblemSeverity
from matter.testing.spec_parsing import XmlCluster
from matter.tlv import uint


class ChoiceConformanceProblemNotice(ProblemNotice):
    def __init__(self, location: AttributePathLocation, choice: Choice, count: int):
        problem = f'Problem with choice conformance {choice} - {count} selected'
        super().__init__(test_name='Choice conformance', location=location, severity=ProblemSeverity.ERROR, problem=problem, spec_location='')
        self.choice = choice
        self.count = count


def _add_to_counts_if_required(conformance_decision_with_choice: ConformanceDecisionWithChoice, element_present: bool, counts: dict[Choice, int]):
    choice = conformance_decision_with_choice.choice
    if not choice:
        return
    counts[choice] = counts.get(choice, 0)
    if element_present:
        counts[choice] += 1


def _evaluate_choices(location: AttributePathLocation, counts: dict[Choice, int]) -> list[ChoiceConformanceProblemNotice]:
    problems: list[ChoiceConformanceProblemNotice] = []
    for choice, count in counts.items():
        if count == 0 or (not choice.more and count > 1):
            problems.append(ChoiceConformanceProblemNotice(location, choice, count))
    return problems


def evaluate_feature_choice_conformance(endpoint_id: int, cluster_id: int, xml_clusters: dict[int, XmlCluster], info: ConformanceAssessmentData) -> list[ChoiceConformanceProblemNotice]:
    all_features = [uint(1 << i) for i in range(32)]
    all_features = [f for f in all_features if f in xml_clusters[cluster_id].features]

    # Other pieces of the 10.2 test check for unknown features, so just remove them here to check choice conformance
    counts: dict[Choice, int] = {}
    for f in all_features:
        xml_feature = xml_clusters[cluster_id].features[f]
        conformance_decision_with_choice = xml_feature.conformance(info)
        _add_to_counts_if_required(conformance_decision_with_choice, (info.feature_map & f) != 0, counts)

    location = AttributePathLocation(endpoint_id=endpoint_id, cluster_id=cluster_id,
                                     attribute_id=GlobalAttributeIds.FEATURE_MAP_ID)
    return _evaluate_choices(location, counts)


def evaluate_attribute_choice_conformance(endpoint_id: int, cluster_id: int, xml_clusters: dict[int, XmlCluster], info: ConformanceAssessmentData) -> list[ChoiceConformanceProblemNotice]:
    all_attributes = xml_clusters[cluster_id].attributes.keys()

    counts: dict[Choice, int] = {}
    for attribute_id in all_attributes:
        conformance_decision_with_choice = xml_clusters[cluster_id].attributes[attribute_id].conformance(info)
        _add_to_counts_if_required(conformance_decision_with_choice, attribute_id in info.attribute_list, counts)

    location = AttributePathLocation(endpoint_id=endpoint_id, cluster_id=cluster_id,
                                     attribute_id=GlobalAttributeIds.ATTRIBUTE_LIST_ID)
    return _evaluate_choices(location, counts)


def evaluate_command_choice_conformance(endpoint_id: int, cluster_id: int, xml_clusters: dict[int, XmlCluster], info: ConformanceAssessmentData) -> list[ChoiceConformanceProblemNotice]:
    all_commands = xml_clusters[cluster_id].accepted_commands.keys()

    counts: dict[Choice, int] = {}
    for command_id in all_commands:
        conformance_decision_with_choice = xml_clusters[cluster_id].accepted_commands[command_id].conformance(info)
        _add_to_counts_if_required(conformance_decision_with_choice, command_id in info.all_command_list, counts)

    location = AttributePathLocation(endpoint_id=endpoint_id, cluster_id=cluster_id,
                                     attribute_id=GlobalAttributeIds.ACCEPTED_COMMAND_LIST_ID)
    return _evaluate_choices(location, counts)
