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

"""
Matter Specification Conformance Engine

This module implements the conformance checking system for Matter clusters, attributes, 
commands, and device types. It provides:

- Conformance decision types (mandatory, optional, disallowed, etc.)
- Boolean operations on conformance (AND, OR, NOT)
- Feature, attribute, and command-based conformance evaluation
- Choice conformance handling for mutually exclusive options
- XML parsing for conformance specifications
- Support for complex conditional conformance expressions

The conformance system evaluates whether a particular element (attribute, command, feature)
should be present on a device based on the device's implemented features, attributes, and
commands.
"""

import xml.etree.ElementTree as ElementTree
from dataclasses import dataclass
from enum import Enum, auto
from typing import Callable, Optional

from matter.tlv import uint

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
GREATER_TERM = 'greaterTerm'
FEATURE_TAG = 'feature'
ATTRIBUTE_TAG = 'attribute'
COMMAND_TAG = 'command'
CONDITION_TAG = 'condition'
LITERAL_TAG = 'literal'
ZIGBEE_CONDITION = 'zigbee'


class ConformanceException(Exception):
    def __init__(self, msg):
        self.msg = msg

    def __str__(self):
        return f"ConformanceException({self.msg})"


class ChoiceConformanceException(ConformanceException):
    def __str__(self):
        return f'ChoiceExceptions({self.msg})'


class BasicConformanceException(ConformanceException):
    pass


@dataclass(frozen=True)
class Choice:
    marker: str
    more: bool

    def __str__(self):
        more_str = '+' if self.more else ''
        return '.' + self.marker + more_str


def parse_choice(element: ElementTree.Element) -> Optional[Choice]:
    choice = element.get('choice', '')
    if not choice:
        return None
    if element.tag != OPTIONAL_CONFORM:
        raise ChoiceConformanceException('Choice conformance on non-optional attribute')
    more = element.get('more', 'false') == 'true'
    return Choice(choice, more)


class ConformanceDecision(Enum):
    MANDATORY = auto()
    OPTIONAL = auto()
    NOT_APPLICABLE = auto()
    DISALLOWED = auto()
    PROVISIONAL = auto()


@dataclass
class ConformanceDecisionWithChoice:
    decision: ConformanceDecision
    choice: Optional[Choice] = None

    def is_mandatory(self) -> bool:
        return self.decision == ConformanceDecision.MANDATORY


@dataclass
class ConformanceParseParameters:
    feature_map: dict[str, uint]
    attribute_map: dict[str, uint]
    command_map: dict[str, uint]


def conformance_allowed(conformance_decision: ConformanceDecisionWithChoice, allow_provisional: bool):
    if conformance_decision.decision in [ConformanceDecision.NOT_APPLICABLE, ConformanceDecision.DISALLOWED]:
        return False
    if conformance_decision.decision == ConformanceDecision.PROVISIONAL:
        return allow_provisional
    return True


def is_disallowed(conformance: Callable):
    # Deprecated and disallowed conformances will come back as disallowed regardless of the implemented features / attributes / etc.
    return conformance(0, [], []).decision == ConformanceDecision.DISALLOWED


@dataclass
class Conformance:
    def __call__(self, feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> ConformanceDecisionWithChoice:
        ''' Evaluates the conformance of a specific cluster or device type element.

            feature_map: The feature_map for the given cluster for which this conformance applies. Used to evaluate feature conformances
            attribute_list: The attribute list for the given cluster for which this conformance applied. Used to evaluate attribute conformances
            all_command_list: combined list of accepted and generated command IDs for the cluster. Used to evaluate command conformances

            Returns: ConformanceDevisionWithChoice
            Raises: ConformanceException if the conformance is invalid
        '''
        raise ConformanceException('Base conformance called')
    choice: Optional[Choice] = None


class zigbee(Conformance):
    def __call__(self, feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> ConformanceDecisionWithChoice:
        return ConformanceDecisionWithChoice(ConformanceDecision.NOT_APPLICABLE)

    def __str__(self):
        return "Zigbee"


class mandatory(Conformance):
    def __call__(self, feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> ConformanceDecisionWithChoice:
        return ConformanceDecisionWithChoice(ConformanceDecision.MANDATORY)

    def __str__(self):
        return 'M'


class optional(Conformance):
    def __init__(self, choice: Optional[Choice] = None):
        self.choice = choice

    def __call__(self, feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> ConformanceDecisionWithChoice:
        return ConformanceDecisionWithChoice(ConformanceDecision.OPTIONAL, self.choice)

    def __str__(self):
        return 'O' + (str(self.choice) if self.choice else '')


class deprecated(Conformance):
    def __call__(self, feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> ConformanceDecisionWithChoice:
        return ConformanceDecisionWithChoice(ConformanceDecision.DISALLOWED)

    def __str__(self):
        return 'D'


class disallowed(Conformance):
    def __call__(self, feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> ConformanceDecisionWithChoice:
        return ConformanceDecisionWithChoice(ConformanceDecision.DISALLOWED)

    def __str__(self):
        return 'X'


class provisional(Conformance):
    def __call__(self, feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> ConformanceDecisionWithChoice:
        return ConformanceDecisionWithChoice(ConformanceDecision.PROVISIONAL)

    def __str__(self):
        return 'P'


class literal(Conformance):
    def __init__(self, value: str):
        self.value = int(value)

    def __call__(self):
        # This should never be called
        raise ConformanceException('Literal conformance function should not be called - this is simply a value holder')

    def __str__(self):
        return str(self.value)


# Conformance options that apply regardless of the element set of the cluster or device
BASIC_CONFORMANCE: dict[str, Conformance] = {
    MANDATORY_CONFORM: mandatory(),
    OPTIONAL_CONFORM: optional(),
    PROVISIONAL_CONFORM: provisional(),
    DEPRECATE_CONFORM: deprecated(),
    DISALLOW_CONFORM: disallowed()
}


class feature(Conformance):
    def __init__(self, requiredFeature: uint, code: str):
        self.requiredFeature = requiredFeature
        self.code = code

    def __call__(self, feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> ConformanceDecisionWithChoice:
        if self.requiredFeature & feature_map != 0:
            return ConformanceDecisionWithChoice(ConformanceDecision.MANDATORY)
        return ConformanceDecisionWithChoice(ConformanceDecision.NOT_APPLICABLE)

    def __str__(self):
        return self.code


class device_feature(Conformance):
    ''' This is different than element feature because device types use "features" that aren't reported anywhere'''

    def __init__(self, feature: str):
        self.feature = feature

    def __call__(self, feature_map: uint = uint(0), attribute_list: list[uint] = [], all_command_list: list[uint] = []) -> ConformanceDecisionWithChoice:
        if (self.feature.lower() == "matter"):
            return ConformanceDecisionWithChoice(ConformanceDecision.MANDATORY)
        elif (self.feature.lower() == 'zigbee'):
            return ConformanceDecisionWithChoice(ConformanceDecision.DISALLOWED)
        else:
            return ConformanceDecisionWithChoice(ConformanceDecision.OPTIONAL)

    def __str__(self):
        return self.feature


class attribute(Conformance):
    def __init__(self, requiredAttribute: uint, name: str):
        self.requiredAttribute = requiredAttribute
        self.name = name

    def __call__(self, feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> ConformanceDecisionWithChoice:
        if self.requiredAttribute in attribute_list:
            return ConformanceDecisionWithChoice(ConformanceDecision.MANDATORY)
        return ConformanceDecisionWithChoice(ConformanceDecision.NOT_APPLICABLE)

    def __str__(self):
        return self.name


class command(Conformance):
    def __init__(self, requiredCommand: uint, name: str):
        self.requiredCommand = requiredCommand
        self.name = name

    def __call__(self, feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> ConformanceDecisionWithChoice:
        if self.requiredCommand in all_command_list:
            return ConformanceDecisionWithChoice(ConformanceDecision.MANDATORY)
        return ConformanceDecisionWithChoice(ConformanceDecision.NOT_APPLICABLE)

    def __str__(self):
        return self.name


def strip_outer_parentheses(inner: str) -> str:
    if inner[0] == '(' and inner[-1] == ')':
        return inner[1:-1]
    return inner


class optional_wrapper(Conformance):
    def __init__(self, op: Conformance, choice: Optional[Choice] = None):
        self.op = op
        self.choice = choice

    def __call__(self, feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> ConformanceDecisionWithChoice:
        decision_with_choice = self.op(feature_map, attribute_list, all_command_list)

        if decision_with_choice.decision in [ConformanceDecision.MANDATORY, ConformanceDecision.OPTIONAL]:
            return ConformanceDecisionWithChoice(ConformanceDecision.OPTIONAL, self.choice)
        elif decision_with_choice.decision == ConformanceDecision.NOT_APPLICABLE:
            return decision_with_choice
        else:
            raise ConformanceException(f'Optional wrapping invalid op {decision_with_choice}')

    def __str__(self):
        return f'[{strip_outer_parentheses(str(self.op))}]' + (str(self.choice) if self.choice else '')


class mandatory_wrapper(Conformance):
    def __init__(self, op: Conformance):
        self.op = op

    def __call__(self, feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> ConformanceDecisionWithChoice:
        return self.op(feature_map, attribute_list, all_command_list)

    def __str__(self):
        return strip_outer_parentheses(str(self.op))


class not_operation(Conformance):
    def __init__(self, op: Conformance):
        if op.choice:
            raise ChoiceConformanceException('NOT operation called on choice conformance')
        self.op = op

    def __call__(self, feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> ConformanceDecisionWithChoice:
        # not operations can't be used with anything that returns DISALLOWED
        # not operations also can't be used with things that are optional
        # ie, ![AB] doesn't make sense, nor does !O
        decision_with_choice = self.op(feature_map, attribute_list, all_command_list)
        if decision_with_choice.decision in [ConformanceDecision.DISALLOWED, ConformanceDecision.PROVISIONAL]:
            raise ConformanceException('NOT operation on optional or disallowed item')
        # Features in device types degrade to optional so a not operation here is still optional because we don't have any way to verify the features since they're not exposed anywhere
        elif decision_with_choice.decision == ConformanceDecision.OPTIONAL:
            return decision_with_choice
        elif decision_with_choice.decision == ConformanceDecision.NOT_APPLICABLE:
            return ConformanceDecisionWithChoice(ConformanceDecision.MANDATORY)
        elif decision_with_choice.decision == ConformanceDecision.MANDATORY:
            return ConformanceDecisionWithChoice(ConformanceDecision.NOT_APPLICABLE)
        else:
            raise ConformanceException('NOT called on item with non-conformance value')

    def __str__(self):
        return f'!{str(self.op)}'


class and_operation(Conformance):
    def __init__(self, op_list: list[Conformance]):
        for op in op_list:
            if op.choice:
                raise ChoiceConformanceException('AND operation with internal choice conformance')
        self.op_list = op_list

    def __call__(self, feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> ConformanceDecisionWithChoice:
        for op in self.op_list:
            decision_with_choice = op(feature_map, attribute_list, all_command_list)
            # and operations can't happen on optional or disallowed
            if decision_with_choice.decision == ConformanceDecision.OPTIONAL and all([type(op) == device_feature for op in self.op_list]):
                return decision_with_choice
            elif decision_with_choice.decision in [ConformanceDecision.OPTIONAL, ConformanceDecision.DISALLOWED, ConformanceDecision.PROVISIONAL]:
                raise ConformanceException('AND operation on optional or disallowed item')
            elif decision_with_choice.decision == ConformanceDecision.NOT_APPLICABLE:
                return decision_with_choice
            elif decision_with_choice.decision == ConformanceDecision.MANDATORY:
                continue
            else:
                raise ConformanceException('Oplist item returned non-conformance value')
        return ConformanceDecisionWithChoice(ConformanceDecision.MANDATORY)

    def __str__(self):
        op_strs = [str(op) for op in self.op_list]
        return f'({" & ".join(op_strs)})'


class or_operation(Conformance):
    def __init__(self, op_list: list[Conformance]):
        for op in op_list:
            if op.choice:
                raise ChoiceConformanceException('OR operation with internal choice conformance')
        self.op_list = op_list

    def __call__(self, feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> ConformanceDecisionWithChoice:
        for op in self.op_list:
            decision_with_choice = op(feature_map, attribute_list, all_command_list)
            if decision_with_choice.decision in [ConformanceDecision.DISALLOWED, ConformanceDecision.PROVISIONAL]:
                raise ConformanceException('OR operation on optional or disallowed item')
            elif decision_with_choice.decision == ConformanceDecision.NOT_APPLICABLE:
                continue
            elif decision_with_choice.decision in [ConformanceDecision.MANDATORY, ConformanceDecision.OPTIONAL]:
                return decision_with_choice
            else:
                raise ConformanceException('Oplist item returned non-conformance value')
        return ConformanceDecisionWithChoice(ConformanceDecision.NOT_APPLICABLE)

    def __str__(self):
        op_strs = [str(op) for op in self.op_list]
        return f'({" | ".join(op_strs)})'


class greater_operation(Conformance):
    def _type_ok(self, op: Conformance):
        return type(op) == attribute or type(op) == literal

    def __init__(self, op1: Conformance, op2: Conformance):
        if not self._type_ok(op1) or not self._type_ok(op2):
            raise ConformanceException('Arithmetic operations can only have attribute or literal value children')
        self.op1 = op1
        self.op2 = op2

    def __call__(self, feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> ConformanceDecisionWithChoice:
        # For now, this is fully optional, need to implement this properly later, but it requires access to the actual attribute values
        # We need to reach into the attribute, but can't use it directly because the attribute callable is an EXISTENCE check and
        # the arithmetic functions require a value.
        return ConformanceDecisionWithChoice(ConformanceDecision.OPTIONAL)

    def __str__(self):
        return f'{str(self.op1)} > {str(self.op2)}'


class otherwise(Conformance):
    def __init__(self, op_list: list[Conformance]):
        self.op_list = op_list

    def __call__(self, feature_map: uint, attribute_list: list[uint], all_command_list: list[uint]) -> ConformanceDecisionWithChoice:
        # Otherwise operations apply from left to right. If any of them
        # has a definite decision (optional, mandatory or disallowed), that is the one that applies
        # Provisional items are meant to be marked as the first item in the list
        # Deprecated items are either on their own, or follow an O as O,D.
        # For O,D, optional applies (leftmost), but we should consider some way to warn here as well,
        # possibly in another function
        for op in self.op_list:
            decision_with_choice = op(feature_map, attribute_list, all_command_list)
            if decision_with_choice.decision == ConformanceDecision.NOT_APPLICABLE:
                continue
            return decision_with_choice
        return ConformanceDecisionWithChoice(ConformanceDecision.NOT_APPLICABLE)

    def __str__(self):
        op_strs = [strip_outer_parentheses(str(op)) for op in self.op_list]
        return ', '.join(op_strs)


def parse_basic_callable_from_xml(element: ElementTree.Element) -> Conformance:
    """
    Parse simple, leaf-node conformance elements from XML.

    Basic conformance elements are XML elements without children that represent
    simple conformance decisions such as:
    - mandatoryConform (M)
    - optionalConform (O) 
    - disallowConform (X)
    - deprecateConform (D)
    - provisionalConform (P)
    - zigbee conditions
    - literal values

    This is in contrast to complex conformance expressions that involve
    boolean operations (AND, OR, NOT) or wrapper operations.

    Args:
        element: XML element representing a basic conformance rule

    Returns:
        Callable conformance object for the basic conformance type

    Raises:
        BasicConformanceException: If element has children or is not a recognized basic type
        ConformanceException: If a basic element is malformed (e.g. missing required attributes)
    """
    if list(element):
        raise BasicConformanceException("parse_basic_callable_from_xml called for XML element with children")
    # This will throw a key error if this is not a basic element key.
    try:
        choice = parse_choice(element)
        if choice and element.tag == OPTIONAL_CONFORM:
            return optional(choice)
        return BASIC_CONFORMANCE[element.tag]
    except KeyError:
        condition_name = element.get('name')
        if element.tag == CONDITION_TAG and condition_name and condition_name.lower() == ZIGBEE_CONDITION:
            return zigbee()
        elif element.tag == LITERAL_TAG:
            literal_value = element.get('value')
            if literal_value is None:
                raise ConformanceException(
                    f"Literal tag missing 'value' attribute: {ElementTree.tostring(element, encoding='unicode').strip()}")
            return literal(literal_value)
        else:
            raise BasicConformanceException(
                f'parse_basic_callable_from_xml called for unknown element {str(element.tag)} {str(element.attrib)}')


def parse_wrapper_callable_from_xml(element: ElementTree.Element, ops: list[Conformance]) -> Conformance:
    """
    Parse complex conformance expressions that wrap or operate on other conformance elements.

    Wrapper conformance elements are XML elements with children that represent
    composite conformance operations such as:
    - Boolean operations: AND, OR, NOT
    - Wrapper operations: optional[...], mandatory[...]
    - Control flow: otherwise (comma-separated alternatives)
    - Comparison operations: greater than

    These contrast with basic conformance elements which are simple leaf nodes.
    Wrapper conformances combine multiple sub-conformances using logical operations
    to create complex conditional requirements.

    Args:
        element: XML element representing a wrapper conformance operation
        ops: List of parsed child conformance callables to be combined

    Returns:
        Callable conformance object that wraps/operates on the child conformances

    Raises:
        ConformanceException: If element tag is unrecognized or has wrong number of children
    """
    # optional can be a wrapper as well as a standalone
    # This can be any of the boolean operations, optional or otherwise
    choice = parse_choice(element)
    if element.tag == OPTIONAL_CONFORM:
        if len(ops) > 1:
            raise ConformanceException(f'OPTIONAL term found with more than one subelement {list(element)}')
        return optional_wrapper(ops[0], choice)
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
    elif element.tag == GREATER_TERM:
        if len(ops) != 2:
            raise ConformanceException(f'Greater than term found with more than two subelements {list(element)}')
        return greater_operation(ops[0], ops[1])
    else:
        raise ConformanceException(f'Unexpected conformance tag with children {element}')


def parse_device_type_callable_from_xml(element: ElementTree.Element) -> Conformance:
    ''' Only allows basic, or wrappers over things that degrade to basic.'''
    if not list(element):
        try:
            return parse_basic_callable_from_xml(element)
        # For device types ONLY, there are conformances called "attributes" that are essentially just placeholders for conditions in the device library.
        # For example, temperature controlled cabinet has conditions called "heating" and "cooling". The cluster conditions are dependent on them, but they're not
        # actually exposed anywhere ON the device other than through the presence of the cluster. So for now, treat any attribute conditions that are cluster conditions
        # as just optional, because it's optional to implement any device type feature.
        # Device types also have some marked as "condition" that are similarly optional
        except BasicConformanceException:
            if element.tag == ATTRIBUTE_TAG or element.tag == CONDITION_TAG or element.tag == FEATURE_TAG:
                return device_feature(element.attrib['name'])
            raise

    ops = [parse_device_type_callable_from_xml(sub) for sub in element]
    return parse_wrapper_callable_from_xml(element, ops)


def parse_callable_from_xml(element: ElementTree.Element, params: ConformanceParseParameters) -> Conformance:
    if not list(element):
        try:
            return parse_basic_callable_from_xml(element)
        except BasicConformanceException:
            # If we get an exception here, it wasn't a basic type, so move on and check if its
            # something else.
            pass
        if element.tag == FEATURE_TAG:
            feature_name = element.get('name')
            if feature_name is None:
                raise ConformanceException(
                    f"Feature tag missing 'name' attribute for element: {ElementTree.tostring(element, encoding='unicode').strip()}")
            try:
                return feature(params.feature_map[feature_name], feature_name)
            except KeyError:
                raise ConformanceException(f'Conformance specifies feature "{feature_name}" not in feature table.')
        elif element.tag == ATTRIBUTE_TAG:
            # Some command conformance tags are marked as attribute, so if this key isn't in attribute, try command
            name = element.get('name')
            if name is None:
                raise ConformanceException(
                    f"Attribute tag missing 'name' attribute for element: {ElementTree.tostring(element, encoding='unicode').strip()}")

            if name in params.attribute_map:
                return attribute(params.attribute_map[name], name)
            elif name in params.command_map:
                return command(params.command_map[name], name)
            else:
                raise ConformanceException(f'Conformance specifies attribute or command "{name}" not in table.')
        elif element.tag == COMMAND_TAG:
            command_name = element.get('name')
            if command_name is None:
                raise ConformanceException(
                    f"Command tag missing 'name' attribute for element: {ElementTree.tostring(element, encoding='unicode').strip()}")
            try:
                return command(params.command_map[command_name], command_name)
            except KeyError:
                raise ConformanceException(f'Conformance specifies command "{command_name}" not in command table.')
            return command(params.command_map[element.get('name')], element.get('name'))
        elif element.tag == CONDITION_TAG:
            return device_feature(element.attrib['name'])
        else:
            raise ConformanceException(
                f'Unexpected xml conformance element with no children {str(element.tag)} {str(element.attrib)}')

    # First build the list, then create the callable for this element
    ops = [parse_callable_from_xml(sub, params) for sub in element]
    return parse_wrapper_callable_from_xml(element, ops)
