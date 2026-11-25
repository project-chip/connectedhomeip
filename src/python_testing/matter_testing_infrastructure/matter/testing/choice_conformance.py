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

from matter.testing.conformance import (Choice, Conformance, ConformanceAssessmentData, ConformanceDecision,
                                        ConformanceDecisionWithChoice, otherwise)
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


def _get_choice_decision(conformance: Conformance, conformance_assessment_data: ConformanceAssessmentData, allow_provisional: bool) -> tuple[ConformanceDecisionWithChoice, bool]:
    decision_with_choice = conformance(conformance_assessment_data)
    if decision_with_choice.decision == ConformanceDecision.PROVISIONAL:
        if allow_provisional and isinstance(conformance, otherwise):
            # Evaluate the non-provisional fallback branches so provisional items in a choice group
            # (e.g., "P, O.a" or "P, O.a-") still contribute to choice conformance when provisional items are allowed.
            for op in conformance.op_list:
                sub_decision = op(conformance_assessment_data)
                if sub_decision.decision in (ConformanceDecision.NOT_APPLICABLE, ConformanceDecision.PROVISIONAL):
                    continue
                return sub_decision, True
        return decision_with_choice, True
    return decision_with_choice, False


def _add_to_counts_if_required(conformance_decision_with_choice: ConformanceDecisionWithChoice, element_present: bool, is_provisional_choice: bool, counts: dict[Choice, int], non_provisional_choices: set[Choice]):
    choice = conformance_decision_with_choice.choice
    if not choice:
        return
    if not is_provisional_choice:
        non_provisional_choices.add(choice)
    counts[choice] = counts.get(choice, 0)
    if element_present:
        counts[choice] += 1


def _evaluate_choices(location: AttributePathLocation, counts: dict[Choice, int], non_provisional_choices: set[Choice], cluster_is_provisional: bool) -> list[ChoiceConformanceProblemNotice]:
    problems: list[ChoiceConformanceProblemNotice] = []
    for choice, count in counts.items():
        # On a non-provisional cluster, if every applicable member of a choice set is provisional and none are selected,
        # do not require implementing a provisional item just because provisional elements are allowed on the test run.
        if not cluster_is_provisional and choice not in non_provisional_choices and count == 0:
            continue
        if count == 0 or (not choice.more and count > 1):
            problems.append(ChoiceConformanceProblemNotice(location, choice, count))
    return problems


def evaluate_feature_choice_conformance(endpoint_id: int, cluster_id: int, xml_clusters: dict[int, XmlCluster], conformance_assessment_data: ConformanceAssessmentData, allow_provisional: bool = False) -> list[ChoiceConformanceProblemNotice]:
    all_features = [uint(1 << i) for i in range(32)]
    all_features = [f for f in all_features if f in xml_clusters[cluster_id].features]

    # Other pieces of the 10.2 test check for unknown features, so just remove them here to check choice conformance
    counts: dict[Choice, int] = {}
    non_provisional_choices: set[Choice] = set()
    for f in all_features:
        xml_feature = xml_clusters[cluster_id].features[f]
        conformance_decision_with_choice, is_provisional_choice = _get_choice_decision(
            xml_feature.conformance, conformance_assessment_data, allow_provisional)
        _add_to_counts_if_required(conformance_decision_with_choice, (conformance_assessment_data.feature_map & f) != 0,
                                   is_provisional_choice, counts, non_provisional_choices)

    location = AttributePathLocation(endpoint_id=endpoint_id, cluster_id=cluster_id,
                                     attribute_id=GlobalAttributeIds.FEATURE_MAP_ID)
    return _evaluate_choices(location, counts, non_provisional_choices, xml_clusters[cluster_id].is_provisional)


def evaluate_attribute_choice_conformance(endpoint_id: int, cluster_id: int, xml_clusters: dict[int, XmlCluster], conformance_assessment_data: ConformanceAssessmentData, allow_provisional: bool = False) -> list[ChoiceConformanceProblemNotice]:
    all_attributes = xml_clusters[cluster_id].attributes.keys()

    counts: dict[Choice, int] = {}
    non_provisional_choices: set[Choice] = set()
    for attribute_id in all_attributes:
        conformance_decision_with_choice, is_provisional_choice = _get_choice_decision(
            xml_clusters[cluster_id].attributes[attribute_id].conformance, conformance_assessment_data, allow_provisional)
        _add_to_counts_if_required(conformance_decision_with_choice,
                                   attribute_id in conformance_assessment_data.attribute_list,
                                   is_provisional_choice, counts, non_provisional_choices)

    location = AttributePathLocation(endpoint_id=endpoint_id, cluster_id=cluster_id,
                                     attribute_id=GlobalAttributeIds.ATTRIBUTE_LIST_ID)
    return _evaluate_choices(location, counts, non_provisional_choices, xml_clusters[cluster_id].is_provisional)


def evaluate_command_choice_conformance(endpoint_id: int, cluster_id: int, xml_clusters: dict[int, XmlCluster], conformance_assessment_data: ConformanceAssessmentData, allow_provisional: bool = False) -> list[ChoiceConformanceProblemNotice]:
    all_commands = xml_clusters[cluster_id].accepted_commands.keys()

    counts: dict[Choice, int] = {}
    non_provisional_choices: set[Choice] = set()
    for command_id in all_commands:
        conformance_decision_with_choice, is_provisional_choice = _get_choice_decision(
            xml_clusters[cluster_id].accepted_commands[command_id].conformance, conformance_assessment_data, allow_provisional)
        _add_to_counts_if_required(conformance_decision_with_choice,
                                   command_id in conformance_assessment_data.all_command_list,
                                   is_provisional_choice, counts, non_provisional_choices)

    location = AttributePathLocation(endpoint_id=endpoint_id, cluster_id=cluster_id,
                                     attribute_id=GlobalAttributeIds.ACCEPTED_COMMAND_LIST_ID)
    return _evaluate_choices(location, counts, non_provisional_choices, xml_clusters[cluster_id].is_provisional)
