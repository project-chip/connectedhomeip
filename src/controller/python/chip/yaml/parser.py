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

from abc import ABC, abstractmethod
from dataclasses import field
import typing
from chip import ChipDeviceCtrl
from chip.tlv import float32
import yaml
import stringcase
import chip.interaction_model
import asyncio as asyncio
import logging
import math
from chip.yaml.errors import ParsingError, UnexpectedParsingError
from .data_model_lookup import *
import chip.yaml.format_converter as Converter

_SUCCESS_STATUS_CODE = "SUCCESS"
logger = logging.getLogger('YamlParser')


class BaseAction(ABC):
    '''Interface for a single yaml action that is to be executed.'''

    def __init__(self, label):
        self._label = label

    @property
    def label(self):
        return self._label

    @abstractmethod
    def run_action(self, dev_ctrl: ChipDeviceCtrl, endpoint: int, node_id: int):
        pass


class InvokeAction(BaseAction):
    '''Single invoke action to be executed including validation of response.'''

    def __init__(self, item: dict, cluster: str, data_model_lookup: DataModelLookup):
        '''Parse cluster invoke from yaml test configuration.

        Args:
          'item': Dictionary containing single invoke to be parsed.
          'cluster': Name of cluster which to invoke action is targeting.
          'data_model_lookup': Data model lookup to get attribute object.
        Raises:
          ParsingError: Raised if there is a benign error, and there is currently no
            action to perform for this write attribute.
          UnexpectedParsingError: Raised if there is an unexpected parsing error.
        '''
        super().__init__(item['label'])
        self._command_name = stringcase.pascalcase(item['command'])
        self._cluster = cluster
        self._request_object = None
        self._expected_raw_response: dict = field(default_factory=dict)
        self._expected_response_object = None

        command = data_model_lookup.get_command(self._cluster, self._command_name)

        if command is None:
            raise ParsingError(
                f'Failed to find cluster:{self._cluster} Command:{self._command_name}')

        command_object = command()
        if (item.get('arguments')):
            args = item['arguments']['values']

            request_data_as_dict = Converter.convert_name_value_pair_to_dict(args)

            try:
                request_data = Converter.convert_yaml_type(
                    request_data_as_dict, type(command_object))
            except ValueError:
                raise ParsingError('Could not covert yaml type')

            # Create a cluster object for the request from the provided YAML data.
            self._request_object = command_object.FromDict(request_data)
        else:
            self._request_object = command_object

        self._expected_raw_response = item.get('response')

        if (self._request_object.response_type is not None and
                self._expected_raw_response is not None and
                self._expected_raw_response.get('values')):
            response_type = stringcase.pascalcase(self._request_object.response_type)
            expected_command = data_model_lookup.get_command(self._cluster, response_type)
            expected_response_args = self._expected_raw_response['values']
            expected_response_data_as_dict = Converter.convert_name_value_pair_to_dict(
                expected_response_args)
            expected_response_data = Converter.convert_yaml_type(
                expected_response_data_as_dict, expected_command)
            self._expected_response_object = expected_command.FromDict(expected_response_data)

    def run_action(self, dev_ctrl: ChipDeviceCtrl, endpoint: int, node_id: int):
        try:
            resp = asyncio.run(dev_ctrl.SendCommand(node_id, endpoint, self._request_object))
        except chip.interaction_model.InteractionModelError:
            if self._expected_raw_response is None:
                raise

            expected_status_code = self._expected_raw_response.get('error')
            if expected_status_code is not None and expected_status_code != _SUCCESS_STATUS_CODE:
                logger.debug('Got error response, but was expected')
            else:
                raise

        if (self._expected_response_object is not None):
            if (self._expected_response_object != resp):
                logger.error(f'Expected response {self._expected_response_object} did not match '
                             f'actual object {resp}')


class ReadAttributeAction(BaseAction):
    '''Single read attribute action to be executed including validation.'''

    def __init__(self, item: dict, cluster: str, data_model_lookup: DataModelLookup):
        '''Parse read attribute action from yaml test configuration.

        Args:
          'item': Dictionary contains single read attribute action to be parsed.
          'cluster': Name of cluster read attribute action is targeting.
          'data_model_lookup': Data model lookup to get attribute object.
        Raises:
          ParsingError: Raised if there is a benign error, and there is currently no
            action to perform for this read attribute.
          UnexpectedParsingError: Raised if there is an unexpected parsing error.
        '''
        super().__init__(item['label'])
        self._attribute_name = stringcase.pascalcase(item['attribute'])
        self._cluster = cluster
        self._cluster_object = None
        self._request_object = None
        self._expected_raw_response: dict = field(default_factory=dict)
        self._expected_response_object: None = None
        self._possibly_unsupported = False

        self._cluster_object = data_model_lookup.get_cluster(self._cluster)
        if self._cluster_object is None:
            raise UnexpectedParsingError(
                f'ReadAttribute failed to find cluster object:{self._cluster}')

        self._request_object = data_model_lookup.get_attribute(self._cluster, self._attribute_name)
        if self._request_object is None:
            raise ParsingError(
                f'ReadAttribute failed to find cluster:{self._cluster} '
                f'Attribute:{self._attribute_name}')

        if (item.get('arguments')):
            raise UnexpectedParsingError(
                f'ReadAttribute should not contain arguments. {self.label}')

        if self._request_object.attribute_type is None:
            raise UnexpectedParsingError(
                f'ReadAttribute doesnt have valid attribute_type. {self.label}')

        self._expected_raw_response = item.get('response')
        if (self._expected_raw_response is None):
            raise UnexpectedParsingError(f'ReadAttribute missing expected response. {self.label}')

        if 'optional' in item:
            self._possibly_unsupported = True

        if 'value' in self._expected_raw_response:
            self._expected_response_object = self._request_object.attribute_type.Type
            expected_response_value = self._expected_raw_response['value']
            self._expected_response_data = Converter.convert_yaml_type(
                expected_response_value, self._expected_response_object, use_from_dict=True)

    def run_action(self, dev_ctrl: ChipDeviceCtrl, endpoint: int, node_id: int):
        try:
            resp = asyncio.run(dev_ctrl.ReadAttribute(node_id, [(self._request_object)]))
        except chip.interaction_model.InteractionModelError:
            if self._expected_raw_response is None:
                raise

            expected_status_code = self._expected_raw_response.get('error')
            if expected_status_code is not None and expected_status_code != _SUCCESS_STATUS_CODE:
                logger.debug('Got error response, but was expected')
            else:
                raise

        if self._possibly_unsupported and not resp:
            # We have found an unsupported attribute. Parsed test did specify that it might be
            # unsupported, so nothing left to validate.
            return

        # TODO: There is likely an issue here with Optional fields since None
        if (self._expected_response_object is not None):
            parsed_resp = resp[endpoint][self._cluster_object][self._request_object]

            if (self._expected_response_data != parsed_resp):
                # TODO: It is debatable if this is the right thing to be doing here. This might
                # need a follow up cleanup.
                if (self._expected_response_object != float32 or
                        not math.isclose(self._expected_response_data, parsed_resp, rel_tol=1e-6)):
                    logger.error(f'Expected response {self._expected_response_data} didnt match '
                                 f'actual object {parsed_resp}')


class WriteAttributeAction(BaseAction):
    '''Single write attribute action to be executed including validation.'''

    def __init__(self, item: dict, cluster: str, data_model_lookup: DataModelLookup):
        '''Parse write attribute action from yaml test configuration.

        Args:
          'item': Dictionary contains single write attribute action to be parsed.
          'cluster': Name of cluster write attribute action is targeting.
          'data_model_lookup': Data model lookup to get attribute object.
        Raises:
          ParsingError: Raised if there is a benign error, and there is currently no
            action to perform for this write attribute.
          UnexpectedParsingError: Raised if there is an unexpected parsing error.
        '''
        super().__init__(item['label'])
        self._attribute_name = stringcase.pascalcase(item['attribute'])
        self._cluster = cluster
        self._request_object = None

        attribute = data_model_lookup.get_attribute(self._cluster, self._attribute_name)
        if attribute is None:
            raise ParsingError(
                f'WriteAttribute failed to find cluster:{self._cluster} '
                f'Attribute:{self._attribute_name}')

        if (item.get('arguments')):
            args = item['arguments']['value']
            try:
                request_data = Converter.convert_yaml_type(
                    args, attribute.attribute_type.Type)
            except ValueError:
                raise ParsingError('Could not covert yaml type')

            # Create a cluster object for the request from the provided YAML data.
            self._request_object = attribute(request_data)
        else:
            raise UnexpectedParsingError(f'WriteAttribute action does have arguments {self.label}')

    def run_action(self, dev_ctrl: ChipDeviceCtrl, endpoint: int, node_id: int):
        try:
            resp = asyncio.run(
                dev_ctrl.WriteAttribute(node_id, [(endpoint, self._request_object)]))
        except chip.interaction_model.InteractionModelError:
            if (self.expected_raw_response is not None and
                    self.expected_raw_response.get('error')):
                logger.debug('Got error, but was expected')
            else:
                raise

        # TODO: confirm resp give a Success value, although not all write action are expected
        # to succeed, hence why this is a todo and not simply just done. Below is example of
        # what success check might look like.
        # asserts.assert_equal(resp[0].Status, StatusEnum.Success, 'label write must succeed')


class YamlTestParser:
    '''Parses the test YAMLs and converts to a more natural Pythonic representation.

    The parser also permits execution of those tests there-after.
    '''

    def __init__(self, yaml_path: str):
        '''Constructor that parser the given a path to YAML test file.'''
        with open(yaml_path, 'r') as stream:
            try:
                self._raw_data = yaml.safe_load(stream)
            except yaml.YAMLError as exc:
                raise exc

        self._name = self._raw_data['name']
        self._node_id = self._raw_data['config']['nodeId']
        self._cluster = self._raw_data['config'].get('cluster')
        if self._cluster:
            self._cluster = self._cluster.replace(' ', '')
        self._endpoint = self._raw_data['config']['endpoint']
        self._base_action_test_list = []
        self._data_model_lookup = PreDefinedDataModelLookup()

        for item in self._raw_data['tests']:
            # This currently behaves differently than the c++ version. We are evaluating if test
            # is disabled before anything else, allowing for incorrectly named commands.
            if item.get('disabled'):
                logger.info(f"Test is disabled, skipping {item['label']}")
                continue

            action = None
            cluster = self._cluster
            # Some of the tests contain 'cluster over-rides' that refer to a different
            # cluster than that specified in 'config'.
            if (item.get('cluster')):
                cluster = item.get('cluster').replace(' ', '').replace('/', '')
            if item['command'] == 'writeAttribute':
                action = self._attribute_write_action_factory(item, cluster)
            elif item['command'] == 'readAttribute':
                action = self._attribute_read_action_factory(item, cluster)
            else:
                action = self._invoke_action_factory(item, cluster)

            if action is not None:
                self._base_action_test_list.append(action)
            else:
                logger.warn(f"Failed to parse {item['label']}")

    def _invoke_action_factory(self, item: dict, cluster: str):
        '''Parse cluster command from yaml test configuration.

        Args:
          'item': Dictionary contains single cluster action test to be parsed
          'cluster': Name of cluster action is targeting.
        Returns:
          InvokeAction if 'item' is a valid action to be executed.
          None if 'item' was not parsed for a known reason that is not fatal.
        '''
        try:
            return InvokeAction(item, cluster, self._data_model_lookup)
        except ParsingError:
            return None

    def _attribute_read_action_factory(self, item: dict, cluster: str):
        '''Parse read attribute action from yaml test configuration.

        Args:
          'item': Dictionary contains single read attribute action to be parsed.
          'cluster': Name of cluster read attribute action is targeting.
        Returns:
          ReadAttributeAction if 'item' is a valid action to be executed.
          None if 'item' was not parsed for a known reason that is not fatal.
        '''
        try:
            return ReadAttributeAction(item, cluster, self._data_model_lookup)
        except ParsingError:
            return None

    def _attribute_write_action_factory(self, item: dict, cluster: str):
        '''Parse write attribute action from yaml test configuration.

        Args:
          'item': Dictionary contains single write attribute action to be parsed.
          'cluster': Name of cluster write attribute action is targeting.
        Returns:
          WriteAttributeAction if 'item' is a valid action to be executed.
          None if 'item' was not parsed for a known reason that is not fatal.
        '''
        try:
            return WriteAttributeAction(item, cluster, self._data_model_lookup)
        except ParsingError:
            return None

    def execute_tests(self, dev_ctrl: ChipDeviceCtrl):
        '''Executes parsed YAML tests.'''
        for idx, action in enumerate(self._base_action_test_list):
            logger.info(f'test: {idx} -- Executing{action.label}')

            action.run_action(dev_ctrl, self._endpoint, self._node_id)
