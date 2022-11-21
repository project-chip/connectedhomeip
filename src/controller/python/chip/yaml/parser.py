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
from chip.clusters.Types import NullValue
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
from .response_storage import ResponseStorage

_SUCCESS_STATUS_CODE = "SUCCESS"
_NODE_ID_DEFAULT = 0x12345
_ENDPOINT_DETAULT = ''  # TODO why is this an empty string
_CLUSTER_DEFAULT = ''
_TIMEOUT_DEFAULT = 90
logger = logging.getLogger('YamlParser')


@dataclass
class _ReferenceObjects:
    # Data model lookup to get python attribute, cluster, command object.
    data_model_lookup: DataModelLookup = None
    # Where various test action response are stored and loaded from.
    response_storage: ResponseStorage = None
    # Top level configuration values for a yaml test.
    config_values: dict = None


class _ConstraintValue:
    def __init__(self, value, field_type, reference_objects: _ReferenceObjects):
        self._response_storage = reference_objects.response_storage
        self._load_expected_response_key = None
        self._value = None

        if value is None:
            # Default values set above is all we need here.
            return

        if isinstance(value, str) and self._response_storage.is_key_saved(value):
            self._load_expected_response_key = value
        else:
            self._value = Converter.convert_yaml_type(
                value, field_type, reference_objects.config_values)

    def get_value(self):
        if self._load_expected_response_key:
            return self._response_storage.load(self._load_expected_response_key)
        return self._value


class _Constraints:
    def __init__(self, constraints: dict, field_type, reference_objects: _ReferenceObjects):
        self._response_storage = reference_objects.response_storage
        self._has_value = constraints.get('hasValue')
        self._type = constraints.get('type')
        self._starts_with = constraints.get('startsWith')
        self._ends_with = constraints.get('endsWith')
        self._is_upper_case = constraints.get('isUpperCase')
        self._is_lower_case = constraints.get('isLowerCase')
        self._min_value = _ConstraintValue(constraints.get('minValue'), field_type, reference_objects)
        self._max_value = _ConstraintValue(constraints.get('maxValue'), field_type, reference_objects)
        self._contains = constraints.get('contains')
        self._excludes = constraints.get('excludes')
        self._has_masks_set = constraints.get('hasMasksSet')
        self._has_masks_clear = constraints.get('hasMasksClear')
        self._not_value = _ConstraintValue(constraints.get('notValue'), field_type, reference_objects)

    def are_constrains_met(self, response) -> bool:
        return_value = True

        if self._has_value:
            logger.warn(f'HasValue constraint currently not implemented, forcing failure')
            return_value = False

        if self._type:
            logger.warn(f'Type constraint currently not implemented, forcing failure')
            return_value = False

        if self._starts_with and not response.startswith(self._starts_with):
            return_value = False

        if self._ends_with and not response.endswith(self._ends_with):
            return_value = False

        if self._is_upper_case and not response.isupper():
            return_value = False

        if self._is_lower_case and not response.islower():
            return_value = False

        min_value = self._min_value.get_value()
        if response is not NullValue and min_value and response < min_value:
            return_value = False

        max_value = self._max_value.get_value()
        if response is not NullValue and max_value and response > max_value:
            return_value = False

        if self._contains and not set(self._contains).issubset(response):
            return_value = False

        if self._excludes and not set(self._excludes).isdisjoint(response):
            return_value = False

        if self._has_masks_set:
            for mask in self._has_masks_set:
                if not (mask & response):
                    return_value = False

        if self._has_masks_clear:
            for mask in self._has_masks_clear:
                if mask & response:
                    return_value = False

        not_value = self._not_value.get_value()
        if not_value and response == not_value:
            return_value = False

        return return_value


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

    def __init__(self, item: dict, cluster: str, reference_objects: _ReferenceObjects):
        '''Parse cluster invoke from yaml test configuration.

        Args:
          'item': Dictionary containing single invoke to be parsed.
          'cluster': Name of cluster which to invoke action is targeting.
          'reference_objects': Contains global common objects such has data model lookup, storage
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

        command = reference_objects.data_model_lookup.get_command(self._cluster, self._command_name)

        if command is None:
            raise ParsingError(
                f'Failed to find cluster:{self._cluster} Command:{self._command_name}')

        command_object = command()
        if (item.get('arguments')):
            args = item['arguments']['values']

            request_data_as_dict = Converter.convert_name_value_pair_to_dict(args)

            try:
                request_data = Converter.convert_yaml_type(
                    request_data_as_dict, type(command_object), reference_objects.config_values)
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
            expected_command = reference_objects.data_model_lookup.get_command(self._cluster,
                                                                               response_type)
            expected_response_args = self._expected_raw_response['values']
            expected_response_data_as_dict = Converter.convert_name_value_pair_to_dict(
                expected_response_args)
            expected_response_data = Converter.convert_yaml_type(
                expected_response_data_as_dict, expected_command, reference_objects.config_values)
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

    def __init__(self, item: dict, cluster: str, reference_objects: _ReferenceObjects):
        '''Parse read attribute action from yaml test configuration.

        Args:
          'item': Dictionary contains single read attribute action to be parsed.
          'cluster': Name of cluster read attribute action is targeting.
          'reference_objects': Contains global common objects such has data model lookup, storage
            for device responses and top level test configurations variable.
        Raises:
          ParsingError: Raised if there is a benign error, and there is currently no
            action to perform for this read attribute.
          UnexpectedParsingError: Raised if there is an unexpected parsing error.
        '''
        super().__init__(item['label'])
        self._attribute_name = stringcase.pascalcase(item['attribute'])
        self._constraints = None
        self._cluster = cluster
        self._cluster_object = None
        self._load_expected_response_key = None
        self._request_object = None
        self._expected_raw_response: dict = field(default_factory=dict)
        self._expected_response_object: None = None
        self._possibly_unsupported = False
        self._response_storage = reference_objects.response_storage
        self._save_response_key = None

        self._cluster_object = reference_objects.data_model_lookup.get_cluster(self._cluster)
        if self._cluster_object is None:
            raise UnexpectedParsingError(
                f'ReadAttribute failed to find cluster object:{self._cluster}')

        self._request_object = reference_objects.data_model_lookup.get_attribute(self._cluster,
                                                                                 self._attribute_name)
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
            raise UnexpectedParsingError(f'ReadAttribute missing expected response. {self.label}')

        self._save_response_key = self._expected_raw_response.get('saveAs')
        if self._save_response_key:
            self._response_storage.save(self._save_response_key, None)

        if 'value' in self._expected_raw_response:
            self._expected_response_object = self._request_object.attribute_type.Type
            expected_response_value = self._expected_raw_response['value']

            if (isinstance(expected_response_value, str) and
                    self._response_storage.is_key_saved(expected_response_value)):
                # Value provided is actually a key to use with storage to load saved data, since
                # this parser, parses all the action first and then runs them we will load the
                # saved expected response during validation time as we can rely on a previous
                # action to save a value that we are comparing against.
                self._load_expected_response_key = expected_response_value
            else:
                self._expected_response_data = Converter.convert_yaml_type(
                    expected_response_value, self._expected_response_object,
                    reference_objects.config_values, use_from_dict=True)

        constraints = self._expected_raw_response.get('constraints')
        if constraints:
            self._constraints = _Constraints(constraints,
                                             self._request_object.attribute_type.Type,
                                             reference_objects)

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

        parsed_resp = resp[endpoint][self._cluster_object][self._request_object]
        if self._constraints and not self._constraints.are_constrains_met(parsed_resp):
            logger.error(f'Constraints check failed')
            # TODO how should we fail the test here?

        # TODO: There is likely an issue here with Optional fields since None
        if (self._expected_response_object is not None):

            if self._load_expected_response_key is not None:
                self._expected_response_data = self._response_storage.load(
                    self._load_expected_response_key)

            if (self._expected_response_data != parsed_resp):
                # TODO: It is debatable if this is the right thing to be doing here. This might
                # need a follow up cleanup.
                if (self._expected_response_object != float32 or
                        not math.isclose(self._expected_response_data, parsed_resp, rel_tol=1e-6)):
                    logger.error(f'Expected response {self._expected_response_data} didnt match '
                                 f'actual object {parsed_resp}')

        if self._save_response_key is not None:
            self._response_storage.save(self._save_response_key, parsed_resp)


class WriteAttributeAction(BaseAction):
    '''Single write attribute action to be executed including validation.'''

    def __init__(self, item: dict, cluster: str, reference_objects: _ReferenceObjects):
        '''Parse write attribute action from yaml test configuration.

        Args:
          'item': Dictionary contains single write attribute action to be parsed.
          'cluster': Name of cluster write attribute action is targeting.
          'reference_objects': Contains global common objects such has data model lookup, storage
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

        attribute = reference_objects.data_model_lookup.get_attribute(
            self._cluster, self._attribute_name)
        if attribute is None:
            raise ParsingError(
                f'WriteAttribute failed to find cluster:{self._cluster} '
                f'Attribute:{self._attribute_name}')

        if (item.get('arguments')):
            args = item['arguments']['value']
            try:
                request_data = Converter.convert_yaml_type(
                    args, attribute.attribute_type.Type, reference_objects.config_values)
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
        self._config = self._raw_data['config']
        self._config.setdefault('nodeId', _NODE_ID_DEFAULT)
        self._config.setdefault('endpoint', _ENDPOINT_DETAULT)
        self._config.setdefault('cluster', _CLUSTER_DEFAULT)
        # TODO timeout is currently not used
        self._config.setdefault('timeout', _TIMEOUT_DEFAULT)

        self._config['cluster'] = self._config['cluster'].replace(' ', '').replace('/', '')
        self._base_action_test_list = []
        self._reference_objects = _ReferenceObjects(data_model_lookup=PreDefinedDataModelLookup(),
                                                    response_storage=ResponseStorage(),
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
            return InvokeAction(item, cluster, self._reference_objects)
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
            return ReadAttributeAction(item, cluster, self._reference_objects)
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
            return WriteAttributeAction(item, cluster, self._reference_objects)
        except ParsingError:
            return None

    def execute_tests(self, dev_ctrl: ChipDeviceCtrl):
        '''Executes parsed YAML tests.'''
        self._reference_objects.response_storage.clear()
        for idx, action in enumerate(self._base_action_test_list):
            logger.info(f'test: {idx} -- Executing{action.label}')

            action.run_action(dev_ctrl, self._config['endpoint'], self._config['nodeId'])
