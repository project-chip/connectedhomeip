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
import queue
from abc import ABC, abstractmethod
from dataclasses import dataclass, field
from enum import Enum, IntEnum

import chip.interaction_model
import chip.yaml.format_converter as Converter
import stringcase
from chip import ChipDeviceCtrl
from chip.clusters.Attribute import AttributeStatus, SubscriptionTransaction, TypedAttributePath, ValueDecodeFailure
from chip.yaml.errors import ParsingError, UnexpectedParsingError

from .data_model_lookup import *

logger = logging.getLogger('YamlParser')


class _ActionStatus(Enum):
    SUCCESS = 'success',
    ERROR = 'error'


class _TestFabricId(IntEnum):
    ALPHA = 1,
    BETA = 2,
    GAMMA = 3


@dataclass
class _ActionResult:
    status: _ActionStatus
    response: object


@dataclass
class _AttributeSubscriptionCallbackResult:
    name: str
    attribute_path: TypedAttributePath
    result: _ActionResult


@dataclass
class _ExecutionContext:
    ''' Objects that is commonly passed around this file that are vital to test execution.'''
    # Data model lookup to get python attribute, cluster, command object.
    data_model_lookup: DataModelLookup = None
    # List of subscriptions.
    subscriptions: list = field(default_factory=list)
    # The key is the attribute/event name, and the value is a queue of subscription callback results
    # that been sent by device under test. For attribute subscription the queue is of type
    # _AttributeSubscriptionCallbackResult.
    subscription_callback_result_queue: dict = field(default_factory=dict)


class BaseAction(ABC):
    '''Interface for a single YAML action that is to be executed.'''

    def __init__(self, label, identity):
        self._label = label
        self._identity = identity

    @property
    def label(self):
        return self._label

    @property
    def identity(self):
        return self._identity

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
        super().__init__(test_step.label, test_step.identity)
        self._command_name = stringcase.pascalcase(test_step.command)
        self._cluster = cluster
        self._interation_timeout_ms = test_step.timed_interaction_timeout_ms
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
            resp = asyncio.run(dev_ctrl.SendCommand(
                self._node_id, self._endpoint, self._request_object,
                timedRequestTimeoutMs=self._interation_timeout_ms))
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
        super().__init__(test_step.label, test_step.identity)
        self._attribute_name = stringcase.pascalcase(test_step.attribute)
        self._cluster = cluster
        self._endpoint = test_step.endpoint
        self._node_id = test_step.node_id
        self._cluster_object = None
        self._request_object = None
        self._fabric_filtered = True

        if test_step.fabric_filtered is not None:
            self._fabric_filtered = test_step.fabric_filtered

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
                                                          [(self._endpoint, self._request_object)],
                                                          fabricFiltered=self._fabric_filtered))
        except chip.interaction_model.InteractionModelError as error:
            return _ActionResult(status=_ActionStatus.ERROR, response=error)

        return self.parse_raw_response(raw_resp)

    def parse_raw_response(self, raw_resp) -> _ActionResult:
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


class WaitForCommissioneeAction(BaseAction):
    ''' Wait for commissionee action to be executed.'''

    def __init__(self, test_step):
        super().__init__(test_step.label, test_step.identity)
        self._node_id = test_step.node_id
        self._expire_existing_session = False
        # This is the default when no timeout is provided.
        _DEFAULT_TIMEOUT_MS = 10 * 1000
        self._timeout_ms = _DEFAULT_TIMEOUT_MS

        if test_step.arguments is None:
            # Nothing left for us to do the default values are what we want
            return

        args = test_step.arguments['values']
        request_data_as_dict = Converter.convert_list_of_name_value_pair_to_dict(args)

        self._expire_existing_session = request_data_as_dict.get('expireExistingSession', False)
        if 'timeout' in request_data_as_dict:
            # Timeout is provided in seconds we need to conver to milliseconds.
            self._timeout_ms = request_data_as_dict['timeout'] * 1000

    def run_action(self, dev_ctrl: ChipDeviceCtrl) -> _ActionResult:
        try:
            if self._expire_existing_session:
                dev_ctrl.ExpireSessions(self._node_id)
            dev_ctrl.GetConnectedDeviceSync(self._node_id, timeoutMs=self._timeout_ms)
        except TimeoutError:
            return _ActionResult(status=_ActionStatus.ERROR, response=None)

        return _ActionResult(status=_ActionStatus.SUCCESS, response=None)


class AttributeChangeAccumulator:
    def __init__(self, name: str, expected_attribute: Clusters.ClusterAttributeDescriptor,
                 output_queue: queue.SimpleQueue):
        self._name = name
        self._expected_attribute = expected_attribute
        self._output_queue = output_queue

    def __call__(self, path: TypedAttributePath, transaction: SubscriptionTransaction):
        if path.AttributeType == self._expected_attribute:
            data = transaction.GetAttribute(path)
            result = _ActionResult(status=_ActionStatus.SUCCESS, response=path.AttributeType(data))

            item = _AttributeSubscriptionCallbackResult(self._name, path, result)
            logging.debug(
                f'Got subscription report on client {self.name} for {path.AttributeType}: {data}')
            self._output_queue.put(item)

    @property
    def name(self) -> str:
        return self._name


class SubscribeAttributeAction(ReadAttributeAction):
    '''Single subscribe attribute action to be executed.'''

    def __init__(self, test_step, cluster: str, context: _ExecutionContext):
        '''Converts 'test_step' to subscribe attribute action that can execute with ChipDeviceCtrl.

        Args:
          'test_step': Step containing information required to run write attribute action.
          'cluster': Name of cluster write attribute action is targeting.
          'context': Contains test-wide common objects such as DataModelLookup instance.
        Raises:
          ParsingError: Raised if there is a benign error, and there is currently no
            action to perform for this write attribute.
          UnexpectedParsingError: Raised if there is an unexpected parsing error.
        '''
        super().__init__(test_step, cluster, context)
        self._context = context
        if test_step.min_interval is None:
            raise UnexpectedParsingError(
                f'SubscribeAttribute action does not have min_interval {self.label}')
        self._min_interval = test_step.min_interval

        if test_step.max_interval is None:
            raise UnexpectedParsingError(
                f'SubscribeAttribute action does not have max_interval {self.label}')
        self._max_interval = test_step.max_interval

    def run_action(self, dev_ctrl: ChipDeviceCtrl) -> _ActionResult:
        try:
            subscription = asyncio.run(
                dev_ctrl.ReadAttribute(self._node_id, [(self._endpoint, self._request_object)],
                                       reportInterval=(self._min_interval, self._max_interval),
                                       keepSubscriptions=False))
        except chip.interaction_model.InteractionModelError as error:
            return _ActionResult(status=_ActionStatus.ERROR, response=error)

        self._context.subscriptions.append(subscription)
        output_queue = self._context.subscription_callback_result_queue.get(self._attribute_name,
                                                                            None)
        if output_queue is None:
            output_queue = queue.SimpleQueue()
            self._context.subscription_callback_result_queue[self._attribute_name] = output_queue

        while not output_queue.empty():
            output_queue.get(block=False)

        subscription_handler = AttributeChangeAccumulator(self.label, self._request_object,
                                                          output_queue)

        subscription.SetAttributeUpdateCallback(subscription_handler)

        raw_resp = subscription.GetAttributes()
        return self.parse_raw_response(raw_resp)


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
        super().__init__(test_step.label, test_step.identity)
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


class WaitForReportAction(BaseAction):
    '''Single WaitForReport action to be executed.'''

    def __init__(self, test_step, context: _ExecutionContext):
        '''Converts 'test_step' to wait for report action.

        Args:
          'test_step': Step containing information required to run wait for report action.
          'context': Contains test-wide common objects such as DataModelLookup instance.
        Raises:
          UnexpectedParsingError: Raised if the expected queue does not exist.
        '''
        super().__init__(test_step.label, test_step.identity)
        self._attribute_name = stringcase.pascalcase(test_step.attribute)
        self._output_queue = context.subscription_callback_result_queue.get(self._attribute_name,
                                                                            None)
        if self._output_queue is None:
            raise UnexpectedParsingError(f'Could not find output queue')

    def run_action(self, dev_ctrl: ChipDeviceCtrl) -> _ActionResult:
        try:
            # While there should be a timeout here provided by the test, the current codegen version
            # of YAML tests doesn't have a per test step timeout, only a global timeout for the
            # entire test. For that reason we default to a 30 second timeout.
            item = self._output_queue.get(block=True, timeout=30)
        except queue.Empty:
            return _ActionResult(status=_ActionStatus.ERROR, response=None)

        return item.result


class CommissionerCommandAction(BaseAction):
    '''Single Commissioner Command action to be executed.'''

    def __init__(self, test_step):
        '''Converts 'test_step' to commissioner command action.

        Args:
          'test_step': Step containing information required to run wait for report action.
        Raises:
          UnexpectedParsingError: Raised if the expected queue does not exist.
        '''
        super().__init__(test_step.label, test_step.identity)
        if test_step.command != 'PairWithCode':
            raise UnexpectedParsingError(f'Unexpected CommisionerCommand {test_step.command}')

        args = test_step.arguments['values']
        request_data_as_dict = Converter.convert_list_of_name_value_pair_to_dict(args)
        self._setup_payload = request_data_as_dict['payload']
        self._node_id = request_data_as_dict['nodeId']

    def run_action(self, dev_ctrl: ChipDeviceCtrl) -> _ActionResult:
        resp = dev_ctrl.CommissionWithCode(self._setup_payload, self._node_id)

        if resp:
            return _ActionResult(status=_ActionStatus.SUCCESS, response=None)
        else:
            return _ActionResult(status=_ActionStatus.ERROR, response=None)


class ReplTestRunner:
    '''Test runner to encode/decode values from YAML test Parser for executing the TestStep.

    Uses ChipDeviceCtrl from chip-repl to execute parsed YAML TestSteps.
    '''

    def __init__(self, test_spec_definition, certificate_authority_manager, alpha_dev_ctrl):
        self._test_spec_definition = test_spec_definition
        self._context = _ExecutionContext(data_model_lookup=PreDefinedDataModelLookup())
        self._certificate_authority_manager = certificate_authority_manager
        self._dev_ctrls = {}

        self._dev_ctrls['alpha'] = alpha_dev_ctrl

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

    def _attribute_subscribe_action_factory(self, test_step, cluster: str):
        '''Creates subscribe attribute command from TestStep provided.

        Args:
          'test_step': Step containing information required to run subscribe attribute action.
          'cluster': Name of cluster write attribute action is targeting.
        Returns:
          SubscribeAttributeAction if 'test_step' is a valid subscribe attribute to be executed.
          None if we were unable to use the provided 'test_step' for a known reason that is not
          fatal to test execution.
        '''
        try:
            return SubscribeAttributeAction(test_step, cluster, self._context)
        except ParsingError:
            # TODO For now, ParsingErrors are largely issues that will be addressed soon. Once this
            # runner has matched parity of the codegen YAML test, this exception should be
            # propogated.
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

    def _wait_for_commissionee_action_factory(self, test_step):
        try:
            return WaitForCommissioneeAction(test_step)
        except ParsingError:
            # TODO For now, ParsingErrors are largely issues that will be addressed soon. Once this
            # runner has matched parity of the codegen YAML test, this exception should be
            # propogated.
            return None

    def _wait_for_report_action_factory(self, test_step):
        try:
            return WaitForReportAction(test_step, self._context)
        except ParsingError:
            # TODO For now, ParsingErrors are largely issues that will be addressed soon. Once this
            # runner has matched parity of the codegen YAML test, this exception should be
            # propogated.
            return None

    def _commissioner_command_action_factory(self, test_step):
        try:
            return CommissionerCommandAction(test_step)
        except ParsingError:
            return None

    def encode(self, request) -> BaseAction:
        action = None
        cluster = request.cluster.replace(' ', '').replace('/', '')
        command = request.command
        if cluster == 'CommissionerCommands':
            return self._commissioner_command_action_factory(request)
        # Some of the tests contain 'cluster over-rides' that refer to a different
        # cluster than that specified in 'config'.

        if cluster == 'DelayCommands' and command == 'WaitForCommissionee':
            action = self._wait_for_commissionee_action_factory(request)
        elif command == 'writeAttribute':
            action = self._attribute_write_action_factory(request, cluster)
        elif command == 'readAttribute':
            action = self._attribute_read_action_factory(request, cluster)
        elif command == 'readEvent':
            # TODO need to implement _event_read_action_factory
            # action = self._event_read_action_factory(request, cluster)
            pass
        elif command == 'subscribeAttribute':
            action = self._attribute_subscribe_action_factory(request, cluster)
        elif command == 'waitForReport':
            action = self._wait_for_report_action_factory(request)
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

    def _get_fabric_id(self, id):
        return _TestFabricId[id.upper()].value

    def _get_dev_ctrl(self, action: BaseAction):
        if action.identity is not None:
            dev_ctrl = self._dev_ctrls.get(action.identity, None)
            if dev_ctrl is None:
                fabric_id = self._get_fabric_id(action.identity)
                certificate_authority = self._certificate_authority_manager.activeCaList[0]
                fabric = None
                for existing_admin in certificate_authority.adminList:
                    if existing_admin.fabricId == fabric_id:
                        fabric = existing_admin

                if fabric is None:
                    fabric = certificate_authority.NewFabricAdmin(vendorId=0xFFF1,
                                                                  fabricId=fabric_id)
                dev_ctrl = fabric.NewController()
                self._dev_ctrls[action.identity] = dev_ctrl
        else:
            dev_ctrl = self._dev_ctrls['alpha']

        return dev_ctrl

    def execute(self, action: BaseAction):
        dev_ctrl = self._get_dev_ctrl(action)
        return action.run_action(dev_ctrl)

    def shutdown(self):
        for subscription in self._context.subscriptions:
            subscription.Shutdown()
