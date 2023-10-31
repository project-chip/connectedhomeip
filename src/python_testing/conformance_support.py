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

import xml.etree.ElementTree as ElementTree
from dataclasses import dataclass
from enum import Enum, auto
from typing import Callable

from chip.tlv import uint

OTHERWISE_CONFORM = 'otherwiseConform'
OPTIONAL_CONFORM = 'optionalConform'
PROVISIONAL_CONFORM = 'provisionalConform'
MANDATORY_CONFORM = 'mandatoryConform'
DEPRECATE_CONFORM = 'deprecateConform'
DISALLOW_CONFORM = 'disallowConform'
AND_TERM = 'andTerm'
OR_TERM = 'orTerm'
NOT_TERM = 'notTerm'
FEATURE_TAG = 'feature'
ATTRIBUTE_TAG = 'attribute'
COMMAND_TAG = 'command'


class ConformanceException(Exception):
    def __init__(self, msg):
        self.msg = msg

    def __str__(self):
        return f"ConformanceException({self.msg})"


class ConformanceDecision(Enum):
    MANDATORY = auto()
    OPTIONAL = auto()
    NOT_APPLICABLE = auto()
    DISALLOWED = auto()
    PROVISIONAL = auto()


@dataclass
class ConformanceParseParameters:
    feature_map: dict[str, uint]
    attribute_map: dict[str, uint]
    command_map: dict[str, uint]


def conformance_allowed(conformance_decision: ConformanceDecision, allow_provisional: bool):
    if conformance_decision == ConformanceDecision.NOT_APPLICABLE or conformance_decision == ConformanceDecision.DISALLOWED:
        return False
    if conformance_decision == ConformanceDecision.PROVISIONAL:
        return allow_provisional
    return True


def mandatory(feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> ConformanceDecision:
    return ConformanceDecision.MANDATORY


def optional(feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> ConformanceDecision:
    return ConformanceDecision.OPTIONAL


def deprecated(feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> ConformanceDecision:
    return ConformanceDecision.DISALLOWED


def disallowed(feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> ConformanceDecision:
    return ConformanceDecision.DISALLOWED


def provisional(feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> ConformanceDecision:
    return ConformanceDecision.PROVISIONAL


def feature(requiredFeature: uint) -> Callable:
    def feature_inner(feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> ConformanceDecision:
        if requiredFeature & feature_map != 0:
            return ConformanceDecision.MANDATORY
        return ConformanceDecision.NOT_APPLICABLE
    return feature_inner


def attribute(requiredAttribute: uint) -> Callable:
    def attribute_inner(feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> ConformanceDecision:
        if requiredAttribute in attribute_list:
            return ConformanceDecision.MANDATORY
        return ConformanceDecision.NOT_APPLICABLE
    return attribute_inner


def command(requiredCommand: uint) -> Callable:
    def command_inner(feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> ConformanceDecision:
        if requiredCommand in all_command_list:
            return ConformanceDecision.MANDATORY
        return ConformanceDecision.NOT_APPLICABLE
    return command_inner


def optional_wrapper(op: Callable) -> Callable:
    def optional_wrapper_inner(feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> ConformanceDecision:
        decision = op(feature_map, attribute_list, all_command_list)
        if decision == ConformanceDecision.MANDATORY or decision == ConformanceDecision.OPTIONAL:
            return ConformanceDecision.OPTIONAL
        elif decision == ConformanceDecision.NOT_APPLICABLE:
            return ConformanceDecision.NOT_APPLICABLE
        else:
            raise ConformanceException(f'Optional wrapping invalid op {decision}')
    return optional_wrapper_inner


def mandatory_wrapper(op: Callable) -> Callable:
    def mandatory_wrapper_inner(feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> ConformanceDecision:
        return op(feature_map, attribute_list, all_command_list)
    return mandatory_wrapper_inner


def not_operation(op: Callable):
    def not_operation_inner(feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> ConformanceDecision:
        # not operations can't be used with anything that returns DISALLOWED
        # not operations also can't be used with things that are optional
        # ie, ![AB] doesn't make sense, nor does !O
        decision = op(feature_map, attribute_list, all_command_list)
        if decision == ConformanceDecision.OPTIONAL or decision == ConformanceDecision.DISALLOWED or decision == ConformanceDecision.PROVISIONAL:
            raise ConformanceException('NOT operation on optional or disallowed item')
        elif decision == ConformanceDecision.NOT_APPLICABLE:
            return ConformanceDecision.MANDATORY
        elif decision == ConformanceDecision.MANDATORY:
            return ConformanceDecision.NOT_APPLICABLE
        else:
            raise ConformanceException('NOT called on item with non-conformance value')
    return not_operation_inner


def and_operation(op_list: list[Callable]) -> Callable:
    def and_operation_inner(feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> ConformanceDecision:
        for op in op_list:
            decision = op(feature_map, attribute_list, all_command_list)
            # and operations can't happen on optional or disallowed
            if decision == ConformanceDecision.OPTIONAL or decision == ConformanceDecision.DISALLOWED or decision == ConformanceDecision.PROVISIONAL:
                raise ConformanceException('AND operation on optional or disallowed item')
            elif decision == ConformanceDecision.NOT_APPLICABLE:
                return ConformanceDecision.NOT_APPLICABLE
            elif decision == ConformanceDecision.MANDATORY:
                continue
            else:
                raise ConformanceException('Oplist item returned non-conformance value')
        return ConformanceDecision.MANDATORY
    return and_operation_inner


def or_operation(op_list: list[Callable]) -> Callable:
    def or_operation_inner(feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> ConformanceDecision:
        for op in op_list:
            decision = op(feature_map, attribute_list, all_command_list)
            if decision == ConformanceDecision.DISALLOWED or decision == ConformanceDecision.PROVISIONAL:
                raise ConformanceException('OR operation on optional or disallowed item')
            elif decision == ConformanceDecision.NOT_APPLICABLE:
                continue
            elif decision == ConformanceDecision.MANDATORY:
                return ConformanceDecision.MANDATORY
            elif decision == ConformanceDecision.OPTIONAL:
                return ConformanceDecision.OPTIONAL
            else:
                raise ConformanceException('Oplist item returned non-conformance value')
        return ConformanceDecision.NOT_APPLICABLE
    return or_operation_inner

# TODO: add xor operation once it's required
# TODO: how would equal and unequal operations work here?


def otherwise(op_list: list[Callable]) -> Callable:
    def otherwise_inner(feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> ConformanceDecision:
        # Otherwise operations apply from left to right. If any of them
        # has a definite decision (optional, mandatory or disallowed), that is the one that applies
        # Provisional items are meant to be marked as the first item in the list
        # Deprecated items are either on their own, or follow an O as O,D.
        # For O,D, optional applies (leftmost), but we should consider some way to warn here as well,
        # possibly in another function
        for op in op_list:
            decision = op(feature_map, attribute_list, all_command_list)
            if decision == ConformanceDecision.NOT_APPLICABLE:
                continue
            return decision
        return ConformanceDecision.NOT_APPLICABLE
    return otherwise_inner


def parse_callable_from_xml(element: ElementTree.Element, params: ConformanceParseParameters) -> Callable:
    if len(list(element)) == 0:
        # no subchildren here, so this can only be mandatory, optional, provisional, deprecated, disallowed, feature or attribute
        if element.tag == MANDATORY_CONFORM:
            return mandatory
        elif element.tag == OPTIONAL_CONFORM:
            return optional
        elif element.tag == PROVISIONAL_CONFORM:
            return provisional
        elif element.tag == DEPRECATE_CONFORM:
            return deprecated
        elif element.tag == DISALLOW_CONFORM:
            return disallowed
        elif element.tag == FEATURE_TAG:
            try:
                return feature(params.feature_map[element.get('name')])
            except KeyError:
                raise ConformanceException(f'Conformance specifies feature not in feature table: {element.get("name")}')
        elif element.tag == ATTRIBUTE_TAG:
            # Some command conformance tags are marked as attribute, so if this key isn't in attribute, try command
            name = element.get('name')
            if name in params.attribute_map:
                return attribute(params.attribute_map[name])
            elif name in params.command_map:
                return command(params.command_map[name])
            else:
                raise ConformanceException(f'Conformance specifies attribute or command not in table: {name}')
        elif element.tag == COMMAND_TAG:
            return command(params.command_map[element.get('name')])
        else:
            raise ConformanceException(
                f'Unexpected xml conformance element with no children {str(element.tag)} {str(element.attrib)}')

    # First build the list, then create the callable for this element
    ops = []
    for sub in element:
        ops.append(parse_callable_from_xml(sub, params))

    # optional can be a wrapper as well as a standalone
    # This can be any of the boolean operations, optional or otherwise
    if element.tag == OPTIONAL_CONFORM:
        if len(ops) > 1:
            raise ConformanceException(f'OPTIONAL term found with more than one subelement {list(element)}')
        return optional_wrapper(ops[0])
    elif element.tag == MANDATORY_CONFORM:
        if len(ops) > 1:
            raise ConformanceException(f'MANDATORY term found with more than one subelement {list(element)}')
        return mandatory_wrapper(ops[0])
    elif element.tag == AND_TERM:
        return and_operation(ops)
    elif element.tag == OR_TERM:
        return or_operation(ops)
    elif element.tag == NOT_TERM:
        if len(ops) > 1:
            raise ConformanceException(f'NOT term found with more than one subelement {list(element)}')
        return not_operation(ops[0])
    elif element.tag == OTHERWISE_CONFORM:
        return otherwise(ops)
    else:
        raise ConformanceException(f'Unexpected conformance tag with children {element}')
