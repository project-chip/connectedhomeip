from chip.tlv import uint
from conformance_support import Choice, ConformanceDecisionWithChoice
from global_attribute_ids import GlobalAttributeIds
from matter_testing_support import AttributePathLocation, ProblemNotice, ProblemSeverity
from spec_parsing_support import XmlCluster


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


def evaluate_feature_choice_conformance(endpoint_id: int, cluster_id: int, xml_clusters: dict[int, XmlCluster], feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> list[ChoiceConformanceProblemNotice]:
    all_features = [1 << i for i in range(32)]
    all_features = [f for f in all_features if f in xml_clusters[cluster_id].features.keys()]

    # Other pieces of the 10.2 test check for unknown features, so just remove them here to check choice conformance
    counts: dict[Choice, int] = {}
    for f in all_features:
        xml_feature = xml_clusters[cluster_id].features[f]
        conformance_decision_with_choice = xml_feature.conformance(feature_map, attribute_list, all_command_list)
        _add_to_counts_if_required(conformance_decision_with_choice, (feature_map & f), counts)

    location = AttributePathLocation(endpoint_id=endpoint_id, cluster_id=cluster_id,
                                     attribute_id=GlobalAttributeIds.FEATURE_MAP_ID)
    return _evaluate_choices(location, counts)


def evaluate_attribute_choice_conformance(endpoint_id: int, cluster_id: int, xml_clusters: dict[int, XmlCluster], feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> list[ChoiceConformanceProblemNotice]:
    all_attributes = xml_clusters[cluster_id].attributes.keys()

    counts: dict[Choice, int] = {}
    for attribute_id in all_attributes:
        conformance_decision_with_choice = xml_clusters[cluster_id].attributes[attribute_id].conformance(
            feature_map, attribute_list, all_command_list)
        _add_to_counts_if_required(conformance_decision_with_choice, attribute_id in attribute_list, counts)

    location = AttributePathLocation(endpoint_id=endpoint_id, cluster_id=cluster_id,
                                     attribute_id=GlobalAttributeIds.ATTRIBUTE_LIST_ID)
    return _evaluate_choices(location, counts)


def evaluate_command_choice_conformance(endpoint_id: int, cluster_id: int, xml_clusters: dict[int, XmlCluster], feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> list[ChoiceConformanceProblemNotice]:
    all_commands = xml_clusters[cluster_id].accepted_commands.keys()

    counts: dict[Choice, int] = {}
    for command_id in all_commands:
        conformance_decision_with_choice = xml_clusters[cluster_id].accepted_commands[command_id].conformance(
            feature_map, attribute_list, all_command_list)
        _add_to_counts_if_required(conformance_decision_with_choice, command_id in all_command_list, counts)

    location = AttributePathLocation(endpoint_id=endpoint_id, cluster_id=cluster_id,
                                     attribute_id=GlobalAttributeIds.ACCEPTED_COMMAND_LIST_ID)
    return _evaluate_choices(location, counts)
