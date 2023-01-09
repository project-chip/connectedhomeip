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

import asyncio as asyncio
import logging
from abc import ABC, abstractmethod
from dataclasses import dataclass
from enum import Enum

import chip.interaction_model
import chip.yaml.format_converter as Converter
import stringcase
from chip import ChipDeviceCtrl
from chip.clusters.Attribute import AttributeStatus, ValueDecodeFailure
from chip.yaml.errors import ParsingError, UnexpectedParsingError

from .data_model_lookup import *

logger = logging.getLogger('YamlParser')


class _ActionStatus(Enum):
    SUCCESS = 'success',
    ERROR = 'error'


@dataclass
class _ActionResult:
    status: _ActionStatus
    response: object


@dataclass
class _ExecutionContext:
    ''' Objects that is commonly passed around this file that are vital to test execution.'''
    # Data model lookup to get python attribute, cluster, command object.
    data_model_lookup: DataModelLookup = None


class BaseAction(ABC):
    '''Interface for a single YAML action that is to be executed.'''

    def __init__(self, label):
        self._label = label

    @property
    def label(self):
        return self._label

    @abstractmethod
    def run_action(self, dev_ctrl: ChipDeviceCtrl) -> _ActionResult:
        pass


class InvokeAction(BaseAction):
    '''Single invoke action to be executed.'''

    def __init__(self, test_step, cluster: str, context: _ExecutionContext):
        '''Converts 'test_step' to invoke command action that can execute with ChipDeviceCtrl.

        Args:
          'test_step': Step containing information required to run invoke command action.
          'cluster': Name of cluster which to invoke action is targeting.
          'context': Contains test-wide common objects such as DataModelLookup instance.
        Raises:
          ParsingError: Raised if there is a benign error, and there is currently no
            action to perform for this write attribute.
          UnexpectedParsingError: Raised if there is an unexpected parsing error.
        '''
        super().__init__(test_step.label)
        self._command_name = stringcase.pascalcase(test_step.command)
        self._cluster = cluster
        self._request_object = None
        self._expected_response_object = None
        self._endpoint = test_step.endpoint
        self._node_id = test_step.node_id

        command = context.data_model_lookup.get_command(self._cluster, self._command_name)

        if command is None:
            raise ParsingError(
                f'Failed to find cluster:{self._cluster} Command:{self._command_name}')

        command_object = command()
        if (test_step.arguments):
            args = test_step.arguments['values']
            request_data_as_dict = Converter.convert_list_of_name_value_pair_to_dict(args)

            try:
                request_data = Converter.convert_to_data_model_type(
                    request_data_as_dict, type(command_object))
            except ValueError:
                # TODO after allowing out of bounds enums to be written this should be changed to
                # UnexpectedParsingError.
                raise ParsingError('Could not covert yaml type')

            self._request_object = command_object.FromDict(request_data)
        else:
            self._request_object = command_object

    def run_action(self, dev_ctrl: ChipDeviceCtrl) -> _ActionResult:
        try:
            resp = asyncio.run(dev_ctrl.SendCommand(self._node_id, self._endpoint,
                                                    self._request_object))
        except chip.interaction_model.InteractionModelError as error:
            return _ActionResult(status=_ActionStatus.ERROR, response=error)

        # Commands with no response give a None response. In those cases we return a success
        return _ActionResult(status=_ActionStatus.SUCCESS, response=resp)


class ReadAttributeAction(BaseAction):
    '''Single read attribute action to be executed.'''

    def __init__(self, test_step, cluster: str, context: _ExecutionContext):
        '''Converts 'test_step' to read attribute action that can execute with ChipDeviceCtrl.

        Args:
          'test_step': Step containing information required to run read attribute action.
          'cluster': Name of cluster read attribute action is targeting.
          'context': Contains test-wide common objects such as DataModelLookup instance.
        Raises:
          ParsingError: Raised if there is a benign error, and there is currently no
            action to perform for this read attribute.
          UnexpectedParsingError: Raised if there is an unexpected parsing error.
        '''
        super().__init__(test_step.label)
        self._attribute_name = stringcase.pascalcase(test_step.attribute)
        self._cluster = cluster
        self._endpoint = test_step.endpoint
        self._node_id = test_step.node_id
        self._cluster_object = None
        self._request_object = None

        self._possibly_unsupported = bool(test_step.optional)

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

        if test_step.arguments:
            raise UnexpectedParsingError(
                f'ReadAttribute should not contain arguments. {self.label}')

        if self._request_object.attribute_type is None:
            raise UnexpectedParsingError(
                f'ReadAttribute doesnt have valid attribute_type. {self.label}')

    def run_action(self, dev_ctrl: ChipDeviceCtrl) -> _ActionResult:
        try:
            raw_resp = asyncio.run(dev_ctrl.ReadAttribute(self._node_id,
                                                          [(self._endpoint, self._request_object)]))
        except chip.interaction_model.InteractionModelError as error:
            return _ActionResult(status=_ActionStatus.ERROR, response=error)

        if self._possibly_unsupported and not raw_resp:
            # We have found an unsupported attribute. TestStep provided did specify that it might be
            # unsupported, so nothing left to validate. We just return a failure here.
            return _ActionResult(status=_ActionStatus.ERROR, response=None)

        # TODO Currently there are no checks that this indexing won't fail. Need to add some
        # initial validity checks. Coming soon in a future PR.
        resp = raw_resp[self._endpoint][self._cluster_object][self._request_object]

        if isinstance(resp, ValueDecodeFailure):
            # response.Reason is of type chip.interaction_model.Status.
            return _ActionResult(status=_ActionStatus.ERROR, response=resp.Reason)

        # decode() is expecting to get a DataModelLookup Object type to grab certain attributes
        # like cluster id.
        return_val = self._request_object(resp)
        return _ActionResult(status=_ActionStatus.SUCCESS, response=return_val)


class WriteAttributeAction(BaseAction):
    '''Single write attribute action to be executed.'''

    def __init__(self, test_step, cluster: str, context: _ExecutionContext):
        '''Converts 'test_step' to write attribute action that can execute with ChipDeviceCtrl.

        Args:
          'test_step': Step containing information required to run write attribute action.
          'cluster': Name of cluster write attribute action is targeting.
          'context': Contains test-wide common objects such as DataModelLookup instance.
        Raises:
          ParsingError: Raised if there is a benign error, and there is currently no
            action to perform for this write attribute.
          UnexpectedParsingError: Raised if there is an unexpected parsing error.
        '''
        super().__init__(test_step.label)
        self._attribute_name = stringcase.pascalcase(test_step.attribute)
        self._cluster = cluster
        self._endpoint = test_step.endpoint
        self._node_id = test_step.node_id
        self._request_object = None

        attribute = context.data_model_lookup.get_attribute(
            self._cluster, self._attribute_name)
        if attribute is None:
            raise ParsingError(
                f'WriteAttribute failed to find cluster:{self._cluster} '
                f'Attribute:{self._attribute_name}')

        if not test_step.arguments:
            raise UnexpectedParsingError(f'WriteAttribute action does have arguments {self.label}')

        args = test_step.arguments['values']
        if len(args) != 1:
            raise UnexpectedParsingError(f'WriteAttribute is trying to write multiple values')
        request_data_as_dict = args[0]
        try:
            # TODO this is an ugly hack
            request_data = Converter.convert_to_data_model_type(
                request_data_as_dict['value'], attribute.attribute_type.Type)
        except ValueError:
            raise ParsingError('Could not covert yaml type')

        # Create a cluster object for the request from the provided YAML data.
        self._request_object = attribute(request_data)

    def run_action(self, dev_ctrl: ChipDeviceCtrl) -> _ActionResult:
        try:
            resp = asyncio.run(
                dev_ctrl.WriteAttribute(self._node_id, [(self._endpoint, self._request_object)]))
        except chip.interaction_model.InteractionModelError:
            # TODO Should we be doing the same thing as InvokeAction on InteractionModelError?
            raise
        if len(resp) == 1 and isinstance(resp[0], AttributeStatus):
            if resp[0].Status == chip.interaction_model.Status.Success:
                return _ActionResult(status=_ActionStatus.SUCCESS, response=None)
            else:
                return _ActionResult(status=_ActionStatus.ERROR, response=resp[0].Status)

        # We always expecte the response to be a list of length 1, for that reason we return error
        # here.
        return _ActionResult(status=_ActionStatus.ERROR, response=None)


class ReplTestRunner:
    '''Test runner to encode/decode values from YAML test Parser for executing the TestStep.

    Uses ChipDeviceCtrl from chip-repl to execute parsed YAML TestSteps.
    '''

    def __init__(self, test_spec_definition, dev_ctrl):
        self._test_spec_definition = test_spec_definition
        self._dev_ctrl = dev_ctrl
        self._context = _ExecutionContext(data_model_lookup=PreDefinedDataModelLookup())

    def _invoke_action_factory(self, test_step, cluster: str):
        '''Creates cluster invoke action command from TestStep.

        Args:
          'test_step': Step containing information required to run an invoke command action.
          'cluster': Name of cluster action is targeting.
        Returns:
          InvokeAction if 'test_step' is a valid action to be executed.
          None if we were unable to use the provided 'test_step' for a known reason that is not
          fatal to test execution.
        '''
        try:
            return InvokeAction(test_step, cluster, self._context)
        except ParsingError:
            return None

    def _attribute_read_action_factory(self, test_step, cluster: str):
        '''Creates read attribute command TestStep.

        Args:
          'test_step': Step containing information required to run read attribute action.
          'cluster': Name of cluster read attribute action is targeting.
        Returns:
          ReadAttributeAction if 'test_step' is a valid read attribute to be executed.
          None if we were unable to use the provided 'test_step' for a known reason that is not
          fatal to test execution.
        '''
        try:
            return ReadAttributeAction(test_step, cluster, self._context)
        except ParsingError:
            return None

    def _attribute_write_action_factory(self, test_step, cluster: str):
        '''Creates write attribute command TestStep.

        Args:
          'test_step': Step containing information required to run write attribute action.
          'cluster': Name of cluster write attribute action is targeting.
        Returns:
          WriteAttributeAction if 'test_step' is a valid write attribute to be executed.
          None if we were unable to use the provided 'test_step' for a known reason that is not
          fatal to test execution.
        '''
        try:
            return WriteAttributeAction(test_step, cluster, self._context)
        except ParsingError:
            return None

    def encode(self, request) -> BaseAction:
        action = None
        cluster = request.cluster.replace(' ', '').replace('/', '')
        command = request.command
        # Some of the tests contain 'cluster over-rides' that refer to a different
        # cluster than that specified in 'config'.
        if command == 'writeAttribute':
            action = self._attribute_write_action_factory(request, cluster)
        elif command == 'readAttribute':
            action = self._attribute_read_action_factory(request, cluster)
        elif command == 'readEvent':
            action = self._event_read_action_factory(request, cluster)
        else:
            action = self._invoke_action_factory(request, cluster)

        if action is None:
            logger.warn(f"Failed to parse {request.label}")
        return action

    def decode(self, result: _ActionResult):
        # If this is a generic response, there is nothing to do.
        if result.response is None:
            # TODO Once yamltest and idl python packages are properly packaged as a single module
            # the type we are returning will be formalized. For now TestStep.post_process_response
            # expects this particular case to be sent as a string.
            return 'success' if result.status == _ActionStatus.SUCCESS else 'failure'

        response = result.response

        decoded_response = {}
        if isinstance(response, chip.interaction_model.InteractionModelError):
            decoded_response['error'] = stringcase.snakecase(response.status.name).upper()
            return decoded_response

        if isinstance(response, chip.interaction_model.Status):
            decoded_response['error'] = stringcase.snakecase(response.name).upper()
            return decoded_response

        cluster_name = self._test_spec_definition.get_cluster_name(response.cluster_id)
        decoded_response['clusterId'] = cluster_name

        if hasattr(response, 'command_id'):
            decoded_response['command'] = self._test_spec_definition.get_response_name(
                response.cluster_id, response.command_id)
            response_definition = self._test_spec_definition.get_response_by_name(
                cluster_name, decoded_response['command'])
            decoded_response['value'] = Converter.from_data_model_to_test_definition(
                self._test_spec_definition, cluster_name, response_definition.fields, response)

        if hasattr(response, 'attribute_id'):
            decoded_response['attribute'] = self._test_spec_definition.get_attribute_name(
                response.cluster_id, response.attribute_id)
            attribute = self._test_spec_definition.get_attribute_by_name(
                cluster_name, decoded_response['attribute'])
            # TODO Once we fix the issue of not being able to find the global attribute properly
            # we should be able to remove this if/else statement below.
            if attribute is None:
                # When we cannot find the attribute it is because it is a global attribute like
                # FeatureMap. Fortunately for these types we can get away with using
                # 'response.value' directly for the time being.
                decoded_response['value'] = response.value
            else:
                decoded_response['value'] = Converter.from_data_model_to_test_definition(
                    self._test_spec_definition, cluster_name, attribute.definition, response.value)

        return decoded_response

    def execute(self, action: BaseAction):
        return action.run_action(self._dev_ctrl)
