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
from dataclasses import dataclass, field
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
from .variable_storage import VariableStorage
from .constraints import get_constraints

_SUCCESS_STATUS_CODE = "SUCCESS"
_NODE_ID_DEFAULT = 0x12345
_ENDPOINT_DETAULT = ''  # TODO why is this an empty string
_CLUSTER_DEFAULT = ''
_TIMEOUT_DEFAULT = 90
logger = logging.getLogger('YamlParser')


@dataclass
class _ExecutionContext:
    ''' Objects that is commonly passed around this file that are vital to test execution.'''
    # Data model lookup to get python attribute, cluster, command object.
    data_model_lookup: DataModelLookup = None
    # Where various test action response are stored and loaded from.
    variable_storage: VariableStorage = None
    # Top level configuration values for a yaml test.
    config_values: dict = None


class _VariableToSave:
    def __init__(self, variable_name: str, variable_storage: VariableStorage):
        self._variable_name = variable_name
        self._variable_storage = variable_storage
        self._variable_storage.save(self._variable_name, None)

    def save_response(self, value):
        self._variable_storage.save(self._variable_name, value)


class _ExpectedResponse:
    def __init__(self, value, response_type, context: _ExecutionContext):
        self._load_expected_response_in_verify = None
        self._expected_response_type = response_type
        self._expected_response = None
        self._variable_storage = context.variable_storage
        if isinstance(value, str) and self._variable_storage.is_key_saved(value):
            self._load_expected_response_in_verify = value
        else:
            self._expected_response = Converter.parse_and_convert_yaml_value(
                value, response_type, context.config_values, inline_cast_dict_to_struct=True)

    def verify(self, response):
        if (self._expected_response_type is None):
            return True

        if self._load_expected_response_in_verify is not None:
            self._expected_response = self._variable_storage.load(
                self._load_expected_response_in_verify)

        if isinstance(self._expected_response_type, float32):
            if not math.isclose(self._expected_response, response, rel_tol=1e-6):
                logger.error(f"Expected response {self._expected_response} didn't match "
                             f"actual object {response}")
                return False

        if (self._expected_response != response):
            logger.error(f"Expected response {self._expected_response} didn't match "
                         f"actual object {response}")
            return False
        return True


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

    def __init__(self, item: dict, cluster: str, context: _ExecutionContext):
        '''Parse cluster invoke from yaml test configuration.

        Args:
          'item': Dictionary containing single invoke to be parsed.
          'cluster': Name of cluster which to invoke action is targeting.
          'context': Contains test-wide common objects such as DataModelLookup instance, storage
            for device responses and top level test configurations variable.
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

        command = context.data_model_lookup.get_command(
            self._cluster, self._command_name)

        if command is None:
            raise ParsingError(
                f'Failed to find cluster:{self._cluster} Command:{self._command_name}')

        command_object = command()
        if (item.get('arguments')):
            args = item['arguments']['values']

            request_data_as_dict = Converter.convert_name_value_pair_to_dict(args)

            try:
                request_data = Converter.parse_and_convert_yaml_value(
                    request_data_as_dict, type(command_object), context.config_values)
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
            expected_command = context.data_model_lookup.get_command(self._cluster,
                                                                     response_type)
            expected_response_args = self._expected_raw_response['values']
            expected_response_data_as_dict = Converter.convert_name_value_pair_to_dict(
                expected_response_args)
            expected_response_data = Converter.parse_and_convert_yaml_value(
                expected_response_data_as_dict, expected_command, context.config_values)
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

    def __init__(self, item: dict, cluster: str, context: _ExecutionContext):
        '''Parse read attribute action from yaml test configuration.

        Args:
          'item': Dictionary contains single read attribute action to be parsed.
          'cluster': Name of cluster read attribute action is targeting.
          'context': Contains test-wide common objects such as DataModelLookup instance, storage
            for device responses and top level test configurations variable.
        Raises:
          ParsingError: Raised if there is a benign error, and there is currently no
            action to perform for this read attribute.
          UnexpectedParsingError: Raised if there is an unexpected parsing error.
        '''
        super().__init__(item['label'])
        self._attribute_name = stringcase.pascalcase(item['attribute'])
        self._constraints = []
        self._cluster = cluster
        self._cluster_object = None
        self._request_object = None
        self._expected_raw_response: dict = field(default_factory=dict)
        self._expected_response: _ExpectedResponse = None
        self._possibly_unsupported = False
        self._variable_to_save = None

        self._cluster_object = context.data_model_lookup.get_cluster(self._cluster)
        if self._cluster_object is None:
            raise UnexpectedParsingError(
                f'ReadAttribute failed to find cluster object:{self._cluster}')

        self._request_object = context.data_model_lookup.get_attribute(
            self._cluster, self._attribute_name)
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

        if 'optional' in item:
            self._possibly_unsupported = True

        self._expected_raw_response = item.get('response')
        if (self._expected_raw_response is None):
            # TODO actually if response is missing it typically means that we need to confirm
            # that we got a successful response. This will be implemented later to consider all
            # possible corner cases around that (if there are corner cases).
            raise UnexpectedParsingError(f'ReadAttribute missing expected response. {self.label}')

        variable_name = self._expected_raw_response.get('saveAs')
        if variable_name:
            self._variable_to_save = _VariableToSave(variable_name, context.variable_storage)

        if 'value' in self._expected_raw_response:
            expected_response_value = self._expected_raw_response['value']
            self._expected_response = _ExpectedResponse(expected_response_value,
                                                        self._request_object.attribute_type.Type,
                                                        context)

        constraints = self._expected_raw_response.get('constraints')
        if constraints:
            self._constraints = get_constraints(constraints,
                                                self._request_object.attribute_type.Type,
                                                context.variable_storage,
                                                context.config_values)

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

        # TODO Currently there are no checks that this indexing won't fail. Need to add some
        # initial validity checks. Coming soon an a future PR.
        parsed_resp = resp[endpoint][self._cluster_object][self._request_object]

        if self._variable_to_save is not None:
            self._variable_to_save.save_response(parsed_resp)

        if not all([constraint.is_met(parsed_resp) for constraint in self._constraints]):
            logger.error(f'Constraints check failed')
            # TODO how should we fail the test here?

        if self._expected_response is not None:
            self._expected_response.verify(parsed_resp)


class WriteAttributeAction(BaseAction):
    '''Single write attribute action to be executed including validation.'''

    def __init__(self, item: dict, cluster: str, context: _ExecutionContext):
        '''Parse write attribute action from yaml test configuration.

        Args:
          'item': Dictionary contains single write attribute action to be parsed.
          'cluster': Name of cluster write attribute action is targeting.
          'context': Contains test-wide common objects such as DataModelLookup instance, storage
            for device responses and top level test configurations variable.
        Raises:
          ParsingError: Raised if there is a benign error, and there is currently no
            action to perform for this write attribute.
          UnexpectedParsingError: Raised if there is an unexpected parsing error.
        '''
        super().__init__(item['label'])
        self._attribute_name = stringcase.pascalcase(item['attribute'])
        self._cluster = cluster
        self._request_object = None

        attribute = context.data_model_lookup.get_attribute(
            self._cluster, self._attribute_name)
        if attribute is None:
            raise ParsingError(
                f'WriteAttribute failed to find cluster:{self._cluster} '
                f'Attribute:{self._attribute_name}')

        if (item.get('arguments')):
            args = item['arguments']['value']
            try:
                request_data = Converter.parse_and_convert_yaml_value(
                    args, attribute.attribute_type.Type, context.config_values)
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

        if 'name' not in self._raw_data:
            raise UnexpectedParsingError("YAML expected to have 'name'")
        self._name = self._raw_data['name']

        if 'config' not in self._raw_data:
            raise UnexpectedParsingError("YAML expected to have 'config'")
        self._config = self._raw_data['config']

        self._config.setdefault('nodeId', _NODE_ID_DEFAULT)
        self._config.setdefault('endpoint', _ENDPOINT_DETAULT)
        self._config.setdefault('cluster', _CLUSTER_DEFAULT)
        # TODO timeout is currently not used
        self._config.setdefault('timeout', _TIMEOUT_DEFAULT)

        self._config['cluster'] = self._config['cluster'].replace(' ', '').replace('/', '')
        self._base_action_test_list = []
        self._context = _ExecutionContext(data_model_lookup=PreDefinedDataModelLookup(),
                                          variable_storage=VariableStorage(),
                                          config_values=self._config)

        for item in self._raw_data['tests']:
            # This currently behaves differently than the c++ version. We are evaluating if test
            # is disabled before anything else, allowing for incorrectly named commands.
            if item.get('disabled'):
                logger.info(f"Test is disabled, skipping {item['label']}")
                continue

            action = None
            cluster = self._config['cluster']
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
            return InvokeAction(item, cluster, self._context)
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
            return ReadAttributeAction(item, cluster, self._context)
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
            return WriteAttributeAction(item, cluster, self._context)
        except ParsingError:
            return None

    def execute_tests(self, dev_ctrl: ChipDeviceCtrl):
        '''Executes parsed YAML tests.'''
        self._context.variable_storage.clear()
        for idx, action in enumerate(self._base_action_test_list):
            logger.info(f'test: {idx} -- Executing{action.label}')

            action.run_action(dev_ctrl, self._config['endpoint'], self._config['nodeId'])
