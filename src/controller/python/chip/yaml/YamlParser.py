#
#    Copyright (c) 2022 Project CHIP Authors
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

from dataclasses import field
import typing
from chip import ChipDeviceCtrl
from chip.tlv import float32
import yaml
import stringcase
import chip.clusters as Clusters
import chip.interaction_model
import asyncio as asyncio
import logging
import math
import chip.yaml.YamlUtils as YamlUtils


logger = logging.getLogger('YamlParser')


class ParsingError(Exception):
    def __init__(self, message):
        super().__init__(message)


class SingleCommandInterface:
    '''Interface for a single yaml command that is to be executed.'''

    def __init__(self):
        pass

    def run_command(self, dev_ctrl: ChipDeviceCtrl, endpoint: int, node_id: int):
        pass


class ClusterCommand(SingleCommandInterface):
    '''Single cluster command to be executed including validation of response.'''

    def __init__(self, label, request_type_name, cluster):
        self.label: str = label
        self.request_type_name: str = request_type_name
        self.cluster: str = cluster
        self.request_data: dict = field(default_factory=dict)
        self.request_object: None = None
        self.expected_response_data: dict = field(default_factory=dict)
        self.expected_response_object: None = None
        self.expected_raw_response: dict = field(default_factory=dict)
        super().__init__()

    def run_command(self, dev_ctrl: ChipDeviceCtrl, endpoint: int, node_id: int):
        try:
            resp = asyncio.run(dev_ctrl.SendCommand(node_id, endpoint, self.request_object))
        except chip.interaction_model.InteractionModelError:
            if (self.expected_raw_response is not None and
                    self.expected_raw_response.get('error')):
                logger.debug('Got error response, but was expected')
            else:
                raise

        if (self.expected_response_object is not None):
            if (self.expected_response_object != resp):
                logger.error(f'Expected response {self.expected_response_object} didnt match '
                             f'actual object {resp}')


class ReadAttributeCommand(SingleCommandInterface):
    '''Single read attribute command to be executed including validation.'''

    def __init__(self, label, request_type_name, cluster):
        self.label: str = label
        self.request_type_name: str = request_type_name
        self.cluster: str = cluster
        self.request_object: None = None
        self.expected_response_data: dict = field(default_factory=dict)
        self.expected_response_object: None = None
        self.expected_raw_response: dict = field(default_factory=dict)
        self.possibly_unsupported: bool = False
        super().__init__()

    def run_command(self, dev_ctrl: ChipDeviceCtrl, endpoint: int, node_id: int):
        try:
            resp = asyncio.run(dev_ctrl.ReadAttribute(node_id, [(self.request_object)]))
        except chip.interaction_model.InteractionModelError:
            if (self.expected_raw_response is not None and
                    self.expected_raw_response.get('error')):
                logger.debug('Got error, but was expected')
            else:
                raise

        if self.possibly_unsupported and not resp:
            # We have found an unsupported attribute and test case did specify
            # that it might be unsupports, so nothing left to validate.
            return

        # TODO: There is likely an issue here with Optional fields since None
        if (self.expected_response_object is not None):
            cluster_object_key = eval(f'Clusters.Objects.{self.cluster}')
            cluster_value_key = eval(
                f'Clusters.Objects.{self.cluster}.Attributes.{self.request_type_name}')
            parsed_resp = resp[endpoint][cluster_object_key][cluster_value_key]

            if (self.expected_response_data != parsed_resp):
                # TODO: It is debatable if this is the right thing to be doing here. This might
                # need a follow up cleanup.
                if (self.expected_response_object != float32 or
                        not math.isclose(self.expected_response_data, parsed_resp, rel_tol=1e-6)):
                    logger.error(f'Expected response {self.expected_response_data} didnt match '
                                 f'actual object {parsed_resp}')


class WriteAttributeCommand(SingleCommandInterface):
    '''Single write attribute command to be executed including validation.'''

    def __init__(self, label, request_type_name, cluster):
        self.label: str = label
        self.request_type_name: str = request_type_name
        self.cluster: str = cluster
        self.request_data: dict = field(default_factory=dict)
        self.request_object: None = None
        super().__init__()

    def run_command(self, dev_ctrl: ChipDeviceCtrl, endpoint: int, node_id: int):
        try:
            resp = asyncio.run(dev_ctrl.WriteAttribute(node_id, [(endpoint, self.request_object)]))
        except chip.interaction_model.InteractionModelError:
            if (self.expected_raw_response is not None and
                    self.expected_raw_response.get('error')):
                logger.debug('Got error, but was expected')
            else:
                raise

        # TODO: confirm resp give a Success value, although not all write commands are expected
        # to succeed, hence why this is a todo and not simply just done. Below is example of
        # what success check might look like.
        # asserts.assert_equal(resp[0].Status, StatusEnum.Success, 'label write must succeed')


def ClusterCommandFactory(item: dict, cluster: str):
    '''Parse cluster command from yaml test configuration.

    Args:
      'item': Dictionary contains single cluster command test to be parsed
      'cluster': Name of cluster command is targeting.
    Returns:
      ReadAttributeCommand if 'item' is a valid command to be executed.
      None if 'item' was not parsed to a command to be executed for a known reason
        that is none fatal.
    Raises:
      ParsingError: Raised if there is an unexpected parsing error.
    '''
    test = ClusterCommand(item['label'], stringcase.pascalcase(item['command']), cluster)

    try:
        test.request_object = eval(f'Clusters.{test.cluster}.Commands.{test.request_type_name}')()
    except AttributeError:
        pass

    if test.request_object is None:
        return None

    if (item.get('arguments')):
        args = item['arguments']['values']

        test.request_data = YamlUtils.fixup_yaml_arguments(args)

        try:
            test.request_data = YamlUtils.fixup_yaml_types(
                test.request_data, type(test.request_object))
        except ValueError:
            return None

        # Create a cluster object for the request from the provided YAML data.
        test.request_object = test.request_object.FromDict(test.request_data)

    test.expected_raw_response = item.get('response')
    if (test.request_object.response_type is not None and
            test.expected_raw_response is not None and
            test.expected_raw_response.get('values')):
        response_type = stringcase.pascalcase(test.request_object.response_type)
        expected_command = f'Clusters.{test.cluster}.Commands.{response_type}'
        test.expected_response_object = eval(expected_command)
        test.expected_response_data = test.expected_raw_response['values']
        test.expected_response_data = YamlUtils.fixup_yaml_arguments(test.expected_response_data)

        test.expected_response_data = YamlUtils.fixup_yaml_types(
            test.expected_response_data, test.expected_response_object)
        test.expected_response_object = test.expected_response_object.FromDict(
            test.expected_response_data)

    return test


def AttributeReadCommandFactory(item: dict, cluster: str):
    '''Parse read attribute command from yaml test configuration.

    Args:
      'item': Dictionary contains single read attribute test to be parsed
      'cluster': Name of cluster read attribute command is targeting.
    Returns:
      ReadAttributeCommand if 'item' is a valid command to be executed.
      None if 'item' was not parsed to a command to be executed for a known reason
        that is none fatal.
    Raises:
      ParsingError: Raised if there is an unexpected parsing error.
    '''
    test = ReadAttributeCommand(item['label'], stringcase.pascalcase(item['attribute']), cluster)

    try:
        test.request_object = eval(f'Clusters.{test.cluster}.Attributes.{test.request_type_name}')
    except AttributeError:
        pass

    if test.request_object is None:
        raise ParsingError(
            f'ReadAttribute failed to find cluster:{test.cluster} '
            f'Attribute:{test.request_type_name}')

    if (item.get('arguments')):
        raise ParsingError(f'ReadAttribute should not contain arguments. {test.label}')

    if test.request_object.attribute_type is None:
        raise ParsingError(f'ReadAttribute doesnt have valid attribute_type. {test.label}')

    test.expected_raw_response = item.get('response')
    if (test.expected_raw_response is None):
        raise ParsingError(f'ReadAttribute missing expected response. {test.label}')

    if 'optional' in item:
        test.possibly_unsupported = True

    if 'value' in test.expected_raw_response:
        test.expected_response_object = test.request_object.attribute_type.Type
        value = test.expected_raw_response['value']
        test.expected_response_data = YamlUtils.fixup_yaml_types(
            value, test.expected_response_object, use_from_dict=True)

    return test


def AttributeWriteCommandFactory(item: dict, cluster: str):
    '''Parse write attribute command from yaml test configuration.

    Args:
      'item': Dictionary contains single write attribute test to be parsed
      'cluster': Name of cluster write attribute command is targeting.
    Returns:
      WriteAttributeCommand if 'item' is a valid command to be executed.
      None if 'item' was not parsed to a command to be executed for a known reason
        that is none fatal.
    Raises:
      ParsingError: Raised if there is an unexpected parsing error.
    '''
    test = WriteAttributeCommand(item['label'], stringcase.pascalcase(item['attribute']), cluster)

    try:
        test.request_object = eval(f'Clusters.{test.cluster}.Attributes.{test.request_type_name}')
    except AttributeError:
        pass

    if test.request_object is None:
        raise ParsingError(
            f'WriteAttribute failed to find cluster:{test.cluster} '
            f'Attribute:{test.request_type_name}')

    if (item.get('arguments')):
        args = item['arguments']['value']
        try:
            test.request_data = YamlUtils.fixup_yaml_types(
                args, test.request_object.attribute_type.Type)
        except ValueError:
            return None

        # Create a cluster object for the request from the provided YAML data.
        test.request_object = test.request_object(test.request_data)
    else:
        raise ParsingError(f'WriteAttribute command does have arguments {test.label}')

    return test


class YamlTestParser:
    '''Parses the test YAMLs and converts to a more natural Pythonic representation.

    The parser also permits execution of those tests there-after.
    '''
    _name: str
    _node_id: int
    _cluster: str
    _endpoint: int
    _base_command_test_list: typing.List[SingleCommandInterface]
    _raw_data: dict

    def __init__(self, yaml_path: str):
        '''Constructor that parser the given a path to YAML test file.'''
        with open(yaml_path, 'r') as stream:
            try:
                self._raw_data = yaml.safe_load(stream)
            except yaml.YAMLError as exc:
                raise exc

        self._name = self._raw_data['name']
        self._node_id = self._raw_data['config']['nodeId']
        self._cluster = self._raw_data['config']['cluster'].replace(' ', '')
        self._endpoint = self._raw_data['config']['endpoint']
        self._base_command_test_list = []

        for item in self._raw_data['tests']:
            # We only support parsing invoke interactions. As support for write/reads get added,
            # these skips will be removed.
            test = None
            cluster = self._cluster
            # Some of the tests contain 'cluster over-rides' that refer to a different
            # cluster than that specified in 'config'.
            if (item.get('cluster')):
                cluster = item.get('cluster').replace(' ', '').replace('/', '')

            if item.get('disabled'):
                logger.info(f"Test is disabled, skipping {item['label']}")
                continue
            if item['command'] == 'writeAttribute':
                test = AttributeWriteCommandFactory(item, cluster)
            elif item['command'] == 'readAttribute':
                test = AttributeReadCommandFactory(item, cluster)
            else:
                test = ClusterCommandFactory(item, cluster)

            if test is not None:
                self._base_command_test_list.append(test)
            else:
                logger.warn(f"Failed to parse {item['label']}")

    def execute_tests(self, dev_ctrl: ChipDeviceCtrl):
        '''Executes parsed YAML tests.'''
        for idx, test in enumerate(self._base_command_test_list):
            logger.info(f'Test: {idx} -- Executing{test.label}')

            test.run_command(dev_ctrl, self._endpoint, self._node_id)
