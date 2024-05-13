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
TOP_LEVEL_CONFORMANCE_TAGS = {OTHERWISE_CONFORM, OPTIONAL_CONFORM,
                              PROVISIONAL_CONFORM, MANDATORY_CONFORM, DEPRECATE_CONFORM, DISALLOW_CONFORM}
AND_TERM = 'andTerm'
OR_TERM = 'orTerm'
NOT_TERM = 'notTerm'
FEATURE_TAG = 'feature'
ATTRIBUTE_TAG = 'attribute'
COMMAND_TAG = 'command'
CONDITION_TAG = 'condition'


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


def is_disallowed(conformance: Callable):
    # Deprecated and disallowed conformances will come back as disallowed regardless of the implemented features / attributes / etc.
    return conformance(0, [], []) == ConformanceDecision.DISALLOWED


class zigbee:
    def __call__(self, feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> ConformanceDecision:
        return ConformanceDecision.NOT_APPLICABLE

    def __str__(self):
        return "Zigbee"


class mandatory:
    def __call__(self, feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> ConformanceDecision:
        return ConformanceDecision.MANDATORY

    def __str__(self):
        return 'M'


class optional:
    def __call__(self, feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> ConformanceDecision:
        return ConformanceDecision.OPTIONAL

    def __str__(self):
        return 'O'


class deprecated:
    def __call__(self, feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> ConformanceDecision:
        return ConformanceDecision.DISALLOWED

    def __str__(self):
        return 'D'


class disallowed:
    def __call__(self, feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> ConformanceDecision:
        return ConformanceDecision.DISALLOWED

    def __str__(self):
        return 'X'


class provisional:
    def __call__(self, feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> ConformanceDecision:
        return ConformanceDecision.PROVISIONAL

    def __str__(self):
        return 'P'


class feature:
    def __init__(self, requiredFeature: uint, code: str):
        self.requiredFeature = requiredFeature
        self.code = code

    def __call__(self, feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> ConformanceDecision:
        if self.requiredFeature & feature_map != 0:
            return ConformanceDecision.MANDATORY
        return ConformanceDecision.NOT_APPLICABLE

    def __str__(self):
        return f'{self.code}'


class attribute:
    def __init__(self, requiredAttribute: uint, name: str):
        self.requiredAttribute = requiredAttribute
        self.name = name

    def __call__(self, feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> ConformanceDecision:
        if self.requiredAttribute in attribute_list:
            return ConformanceDecision.MANDATORY
        return ConformanceDecision.NOT_APPLICABLE

    def __str__(self):
        return f'{self.name}'


class command:
    def __init__(self, requiredCommand: uint, name: str):
        self.requiredCommand = requiredCommand
        self.name = name

    def __call__(self, feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> ConformanceDecision:
        if self.requiredCommand in all_command_list:
            return ConformanceDecision.MANDATORY
        return ConformanceDecision.NOT_APPLICABLE

    def __str__(self):
        return f'{self.name}'


def strip_outer_parentheses(inner: str) -> str:
    if inner[0] == '(' and inner[-1] == ')':
        return inner[1:-1]
    return inner


class optional_wrapper:
    def __init__(self, op: Callable):
        self.op = op

    def __call__(self, feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> ConformanceDecision:
        decision = self.op(feature_map, attribute_list, all_command_list)
        if decision == ConformanceDecision.MANDATORY or decision == ConformanceDecision.OPTIONAL:
            return ConformanceDecision.OPTIONAL
        elif decision == ConformanceDecision.NOT_APPLICABLE:
            return ConformanceDecision.NOT_APPLICABLE
        else:
            raise ConformanceException(f'Optional wrapping invalid op {decision}')

    def __str__(self):
        return f'[{strip_outer_parentheses(str(self.op))}]'


class mandatory_wrapper:
    def __init__(self, op: Callable):
        self.op = op

    def __call__(self, feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> ConformanceDecision:
        return self.op(feature_map, attribute_list, all_command_list)

    def __str__(self):
        return strip_outer_parentheses(str(self.op))


class not_operation:
    def __init__(self, op: Callable):
        self.op = op

    def __call__(self, feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> ConformanceDecision:
        # not operations can't be used with anything that returns DISALLOWED
        # not operations also can't be used with things that are optional
        # ie, ![AB] doesn't make sense, nor does !O
        decision = self.op(feature_map, attribute_list, all_command_list)
        if decision == ConformanceDecision.OPTIONAL or decision == ConformanceDecision.DISALLOWED or decision == ConformanceDecision.PROVISIONAL:
            raise ConformanceException('NOT operation on optional or disallowed item')
        elif decision == ConformanceDecision.NOT_APPLICABLE:
            return ConformanceDecision.MANDATORY
        elif decision == ConformanceDecision.MANDATORY:
            return ConformanceDecision.NOT_APPLICABLE
        else:
            raise ConformanceException('NOT called on item with non-conformance value')

    def __str__(self):
        return f'!{str(self.op)}'


class and_operation:
    def __init__(self, op_list: list[Callable]):
        self.op_list = op_list

    def __call__(self, feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> ConformanceDecision:
        for op in self.op_list:
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

    def __str__(self):
        op_strs = [str(op) for op in self.op_list]
        return f'({" & ".join(op_strs)})'


class or_operation:
    def __init__(self, op_list: list[Callable]):
        self.op_list = op_list

    def __call__(self, feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> ConformanceDecision:
        for op in self.op_list:
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

    def __str__(self):
        op_strs = [str(op) for op in self.op_list]
        return f'({" | ".join(op_strs)})'

# TODO: add xor operation once it's required
# TODO: how would equal and unequal operations work here?


class otherwise:
    def __init__(self, op_list: list[Callable]):
        self.op_list = op_list

    def __call__(self, feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> ConformanceDecision:
        # Otherwise operations apply from left to right. If any of them
        # has a definite decision (optional, mandatory or disallowed), that is the one that applies
        # Provisional items are meant to be marked as the first item in the list
        # Deprecated items are either on their own, or follow an O as O,D.
        # For O,D, optional applies (leftmost), but we should consider some way to warn here as well,
        # possibly in another function
        for op in self.op_list:
            decision = op(feature_map, attribute_list, all_command_list)
            if decision == ConformanceDecision.NOT_APPLICABLE:
                continue
            return decision
        return ConformanceDecision.NOT_APPLICABLE

    def __str__(self):
        op_strs = [strip_outer_parentheses(str(op)) for op in self.op_list]
        return ', '.join(op_strs)


def parse_callable_from_xml(element: ElementTree.Element, params: ConformanceParseParameters) -> Callable:
    if len(list(element)) == 0:
        # no subchildren here, so this can only be mandatory, optional, provisional, deprecated, disallowed, feature or attribute
        if element.tag == MANDATORY_CONFORM:
            return mandatory()
        elif element.tag == OPTIONAL_CONFORM:
            return optional()
        elif element.tag == PROVISIONAL_CONFORM:
            return provisional()
        elif element.tag == DEPRECATE_CONFORM:
            return deprecated()
        elif element.tag == DISALLOW_CONFORM:
            return disallowed()
        elif element.tag == FEATURE_TAG:
            try:
                return feature(params.feature_map[element.get('name')], element.get('name'))
            except KeyError:
                raise ConformanceException(f'Conformance specifies feature not in feature table: {element.get("name")}')
        elif element.tag == ATTRIBUTE_TAG:
            # Some command conformance tags are marked as attribute, so if this key isn't in attribute, try command
            name = element.get('name')
            if name in params.attribute_map:
                return attribute(params.attribute_map[name], name)
            elif name in params.command_map:
                return command(params.command_map[name], name)
            else:
                raise ConformanceException(f'Conformance specifies attribute or command not in table: {name}')
        elif element.tag == COMMAND_TAG:
            return command(params.command_map[element.get('name')], element.get('name'))
        elif element.tag == CONDITION_TAG and element.get('name').lower() == 'zigbee':
            return zigbee()
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
