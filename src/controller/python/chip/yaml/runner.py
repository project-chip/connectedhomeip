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

import logging
import queue
from abc import ABC, abstractmethod
from dataclasses import dataclass, field
from enum import Enum, IntEnum
from typing import Any, Optional, Tuple

import chip.interaction_model
import chip.yaml.format_converter as Converter
from chip.ChipDeviceCtrl import ChipDeviceController, discovery
from chip.clusters import ClusterObjects
from chip.clusters.Attribute import (AttributeStatus, EventReadResult, SubscriptionTransaction, TypedAttributePath,
                                     ValueDecodeFailure)
from chip.exceptions import ChipStackError
from chip.yaml.data_model_lookup import DataModelLookup
from chip.yaml.errors import ActionCreationError, UnexpectedActionCreationError
from matter_idl.generators.filters import to_pascal_case, to_snake_case
from matter_yamltests.pseudo_clusters.pseudo_clusters import get_default_pseudo_clusters

from .data_model_lookup import PreDefinedDataModelLookup

_PSEUDO_CLUSTERS = get_default_pseudo_clusters()
logger = logging.getLogger('YamlParser')


class _ActionStatus(Enum):
    SUCCESS = 'success',
    ERROR = 'error'


class _TestFabricId(IntEnum):
    ALPHA = 1,
    BETA = 2,
    GAMMA = 3


@dataclass
class _GetCommissionerNodeIdResult:
    node_id: int


@dataclass
class EventResponse:
    event_result_list: list[EventReadResult]


@dataclass
class _ActionResult:
    status: _ActionStatus
    response: Any


@dataclass
class _AttributeSubscriptionCallbackResult:
    name: str
    attribute_path: TypedAttributePath
    result: _ActionResult


@dataclass
class _EventSubscriptionCallbackResult:
    name: str
    result: _ActionResult


@dataclass
class _ExecutionContext:
    ''' Objects that is commonly passed around this file that are vital to test execution.'''
    # Data model lookup to get python attribute, cluster, command object.
    data_model_lookup: DataModelLookup
    # List of subscriptions.
    subscriptions: list = field(default_factory=list)
    # The key is the attribute/event name, and the value is a queue of subscription callback results
    # that been sent by device under test. For attribute subscription the queue is of type
    # _AttributeSubscriptionCallbackResult, for event the queue is of type
    # _EventSubscriptionCallbackResult.
    subscription_callback_result_queue: dict = field(default_factory=dict)


class BaseAction(ABC):
    '''Interface for a single YAML action that is to be executed.'''

    def __init__(self, test_step):
        self._label = test_step.label
        self._identity = test_step.identity
        self._pics_enabled = test_step.is_pics_enabled

    @property
    def label(self):
        return self._label

    @property
    def identity(self):
        return self._identity

    @property
    def pics_enabled(self):
        return self._pics_enabled

    @abstractmethod
    async def run_action(self, dev_ctrl: ChipDeviceController) -> _ActionResult:
        pass


class DefaultPseudoCluster(BaseAction):
    def __init__(self, test_step):
        super().__init__(test_step)
        self._test_step = test_step
        if not _PSEUDO_CLUSTERS.supports(test_step):
            raise ActionCreationError(f'Default cluster {test_step.cluster} {test_step.command}, not supported')

    async def run_action(self, dev_ctrl: ChipDeviceController) -> _ActionResult:
        response = await _PSEUDO_CLUSTERS.execute(self._test_step)
        return _ActionResult(status=_ActionStatus.SUCCESS, response=response[0])


class InvokeAction(BaseAction):
    '''Single invoke action to be executed.'''

    def __init__(self, test_step, cluster: str, context: _ExecutionContext):
        '''Converts 'test_step' to invoke command action that can execute with ChipDeviceController.

        Args:
          'test_step': Step containing information required to run invoke command action.
          'cluster': Name of cluster which to invoke action is targeting.
          'context': Contains test-wide common objects such as DataModelLookup instance.
        Raises:
          ActionCreationError: Raised if there is a benign error. This occurs when we
            cannot find the action to invoke for the provided cluster. When this happens
            it is expected that the action to invoke and the provided cluster is an action
            to be invoked on a pseudo cluster.
          UnexpectedActionCreationError: Raised if there is an unexpected parsing error.
        '''
        super().__init__(test_step)
        self._busy_wait_ms = test_step.busy_wait_ms
        self._command_name = to_pascal_case(test_step.command)
        self._cluster = cluster
        self._interation_timeout_ms = test_step.timed_interaction_timeout_ms
        self._request_object = None
        self._expected_response_object = None
        self._endpoint = test_step.endpoint
        self._node_id = test_step.node_id
        self._group_id = test_step.group_id

        if self._node_id is None and self._group_id is None:
            raise UnexpectedActionCreationError(
                'Both node_id and group_id are None, at least one needs to be provided')

        command = context.data_model_lookup.get_command(self._cluster, self._command_name)

        if command is None:
            # If we have not found a command it could me that it is a pseudo cluster command.
            raise ActionCreationError(
                f'Failed to find cluster:{self._cluster} Command:{self._command_name}')

        command_object = command()
        if (test_step.arguments):
            args = test_step.arguments['values']
            request_data_as_dict = Converter.convert_list_of_name_value_pair_to_dict(args)

            try:
                request_data = Converter.convert_to_data_model_type(
                    request_data_as_dict, type(command_object))
            except ValueError:
                raise UnexpectedActionCreationError('Could not covert yaml type')

            self._request_object = command_object.FromDict(request_data)
        else:
            self._request_object = command_object

    async def run_action(self, dev_ctrl: ChipDeviceController) -> _ActionResult:
        try:
            if self._group_id:
                resp = dev_ctrl.SendGroupCommand(
                    self._group_id, self._request_object,
                    busyWaitMs=self._busy_wait_ms)
            else:
                resp = await dev_ctrl.SendCommand(
                    self._node_id, self._endpoint, self._request_object,
                    timedRequestTimeoutMs=self._interation_timeout_ms,
                    busyWaitMs=self._busy_wait_ms)
        except chip.interaction_model.InteractionModelError as error:
            return _ActionResult(status=_ActionStatus.ERROR, response=error)

        # Commands with no response give a None response. In those cases we return a success
        return _ActionResult(status=_ActionStatus.SUCCESS, response=resp)


class ReadAttributeAction(BaseAction):
    '''Single read attribute action to be executed.'''

    def __init__(self, test_step, cluster: str, context: _ExecutionContext):
        '''Converts 'test_step' to read attribute action that can execute with ChipDeviceController.

        Args:
          'test_step': Step containing information required to run read attribute action.
          'cluster': Name of cluster read attribute action is targeting.
          'context': Contains test-wide common objects such as DataModelLookup instance.
        Raises:
          UnexpectedActionCreationError: Raised if there is an unexpected parsing error.
        '''
        super().__init__(test_step)
        self._attribute_name = to_pascal_case(test_step.attribute)
        self._cluster = cluster
        self._endpoint = test_step.endpoint
        self._node_id = test_step.node_id
        self._cluster_object = None
        self._request_object = None
        self._fabric_filtered = True

        if test_step.fabric_filtered is not None:
            self._fabric_filtered = test_step.fabric_filtered

        self._cluster_object = context.data_model_lookup.get_cluster(self._cluster)
        if self._cluster_object is None:
            raise UnexpectedActionCreationError(
                f'ReadAttribute failed to find cluster object:{self._cluster}')

        self._request_object = context.data_model_lookup.get_attribute(
            self._cluster, self._attribute_name)
        if self._request_object is None:
            raise UnexpectedActionCreationError(
                f'ReadAttribute failed to find cluster:{self._cluster} '
                f'Attribute:{self._attribute_name}')

        if test_step.arguments:
            raise UnexpectedActionCreationError(
                f'ReadAttribute should not contain arguments. {self.label}')

        if self._request_object.attribute_type is None:
            raise UnexpectedActionCreationError(
                f'ReadAttribute doesnt have valid attribute_type. {self.label}')

    async def run_action(self, dev_ctrl: ChipDeviceController) -> _ActionResult:
        try:
            raw_resp = await dev_ctrl.ReadAttribute(self._node_id,
                                                    [(self._endpoint, self._request_object)],
                                                    fabricFiltered=self._fabric_filtered)
        except chip.interaction_model.InteractionModelError as error:
            return _ActionResult(status=_ActionStatus.ERROR, response=error)
        except ChipStackError as error:
            _CHIP_TIMEOUT_ERROR = 50
            if error.err == _CHIP_TIMEOUT_ERROR:
                return _ActionResult(status=_ActionStatus.ERROR, response=error)
            # For now it is unsure if all ChipStackError are supposed to be intentional.
            # As a result we simply re-raise the error.
            raise error

        return self.parse_raw_response(raw_resp)

    def parse_raw_response(self, raw_resp) -> _ActionResult:
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


class ReadEventAction(BaseAction):
    ''' Read Event action to be executed.'''

    def __init__(self, test_step, cluster: str, context: _ExecutionContext):
        '''Converts 'test_step' to read event action that can execute with ChipDeviceController.

        Args:
          'test_step': Step containing information required to run read event action.
          'cluster': Name of cluster read event action is targeting.
          'context': Contains test-wide common objects such as DataModelLookup instance.
        Raises:
          UnexpectedActionCreationError: Raised if there is an unexpected parsing error.
        '''
        super().__init__(test_step)
        self._event_name = to_pascal_case(test_step.event)
        self._cluster = cluster
        self._endpoint = test_step.endpoint
        self._node_id = test_step.node_id
        self._cluster_object = None
        self._request_object = None
        self._event_number_filter = test_step.event_number
        self._fabric_filtered = False

        if test_step.fabric_filtered is not None:
            self._fabric_filtered = test_step.fabric_filtered

        self._request_object = context.data_model_lookup.get_event(self._cluster,
                                                                   self._event_name)
        if self._request_object is None:
            raise UnexpectedActionCreationError(
                f'ReadEvent failed to find cluster:{self._cluster} Event:{self._event_name}')

        if test_step.arguments:
            raise UnexpectedActionCreationError(
                f'ReadEvent should not contain arguments. {self.label}')

    async def run_action(self, dev_ctrl: ChipDeviceController) -> _ActionResult:
        try:
            urgent = 0
            request = [(self._endpoint, self._request_object, urgent)]
            resp = await dev_ctrl.ReadEvent(self._node_id, events=request, eventNumberFilter=self._event_number_filter,
                                            fabricFiltered=self._fabric_filtered)
        except chip.interaction_model.InteractionModelError as error:
            return _ActionResult(status=_ActionStatus.ERROR, response=error)

        parsed_resp = EventResponse(event_result_list=resp)
        return _ActionResult(status=_ActionStatus.SUCCESS, response=parsed_resp)


class WaitForCommissioneeAction(BaseAction):
    ''' Wait for commissionee action to be executed.'''

    def __init__(self, test_step):
        super().__init__(test_step)
        self._expire_existing_session = False
        # This is the default when no timeout is provided.
        _DEFAULT_TIMEOUT_MS = 10 * 1000
        self._timeout_ms = _DEFAULT_TIMEOUT_MS

        if test_step.arguments is None:
            # Nothing left for us to do the default values are what we want
            return

        args = test_step.arguments['values']
        request_data_as_dict = Converter.convert_list_of_name_value_pair_to_dict(args)

        # There's a chance the commissionee may have rebooted before this call here as part of a
        # test flow or is just starting out fresh outright. Unless expireExistingSession is
        # explicitly set, the default behaviour it to make sure we're not re-using any cached CASE
        # sessions that will now be stale and mismatched with the peer, causing subsequent
        # interactions to fail.
        self._expire_existing_session = request_data_as_dict.get('expireExistingSession', True)
        self._node_id = request_data_as_dict['nodeId']
        if 'timeout' in request_data_as_dict:
            # Timeout is provided in seconds we need to conver to milliseconds.
            self._timeout_ms = request_data_as_dict['timeout'] * 1000

    async def run_action(self, dev_ctrl: ChipDeviceController) -> _ActionResult:
        try:
            if self._expire_existing_session:
                dev_ctrl.ExpireSessions(self._node_id)
            dev_ctrl.GetConnectedDeviceSync(self._node_id, timeoutMs=self._timeout_ms)
        except TimeoutError:
            return _ActionResult(status=_ActionStatus.ERROR, response=None)

        return _ActionResult(status=_ActionStatus.SUCCESS, response=None)


class AttributeChangeAccumulator:
    def __init__(self, name: str, expected_attribute: ClusterObjects.ClusterAttributeDescriptor,
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


class EventChangeAccumulator:
    def __init__(self, name: str, expected_event, output_queue: queue.SimpleQueue):
        self._name = name
        self._expected_event = expected_event
        self._output_queue = output_queue

    def __call__(self, event_result: EventReadResult, transaction: SubscriptionTransaction):
        if (self._expected_event.cluster_id == event_result.Header.ClusterId and
                self._expected_event.event_id == event_result.Header.EventId):
            event_response = EventResponse(event_result_list=[event_result])
            result = _ActionResult(status=_ActionStatus.SUCCESS, response=event_response)

            item = _EventSubscriptionCallbackResult(self._name, result)
            logging.debug(f'Got subscription report on client {self.name}')
            self._output_queue.put(item)

    @property
    def name(self) -> str:
        return self._name


class SubscribeAttributeAction(ReadAttributeAction):
    '''Single subscribe attribute action to be executed.'''

    def __init__(self, test_step, cluster: str, context: _ExecutionContext):
        '''Converts 'test_step' to subscribe attribute action that can execute with ChipDeviceController.

        Args:
          'test_step': Step containing information required to run write attribute action.
          'cluster': Name of cluster write attribute action is targeting.
          'context': Contains test-wide common objects such as DataModelLookup instance.
        Raises:
          UnexpectedActionCreationError: Raised if there is an unexpected parsing error.
        '''
        super().__init__(test_step, cluster, context)
        self._context = context
        if test_step.min_interval is None:
            raise UnexpectedActionCreationError(
                f'SubscribeAttribute action does not have min_interval {self.label}')
        self._min_interval = test_step.min_interval

        if test_step.max_interval is None:
            raise UnexpectedActionCreationError(
                f'SubscribeAttribute action does not have max_interval {self.label}')
        self._max_interval = test_step.max_interval

    async def run_action(self, dev_ctrl: ChipDeviceController) -> _ActionResult:
        try:
            subscription = await dev_ctrl.ReadAttribute(self._node_id, [(self._endpoint, self._request_object)],
                                                        reportInterval=(self._min_interval, self._max_interval),
                                                        keepSubscriptions=False)
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


class SubscribeEventAction(ReadEventAction):
    '''Single subscribe event action to be executed.'''

    def __init__(self, test_step, cluster: str, context: _ExecutionContext):
        '''Converts 'test_step' to subscribe event action that can execute with ChipDeviceController.

        Args:
          'test_step': Step containing information required to run subscribe event action.
          'cluster': Name of cluster subscribe event action is targeting.
          'context': Contains test-wide common objects such as DataModelLookup instance.
        Raises:
          UnexpectedActionCreationError: Raised if there is an unexpected parsing error.
        '''
        super().__init__(test_step, cluster, context)
        self._context = context
        if test_step.min_interval is None:
            raise UnexpectedActionCreationError(
                f'SubscribeEvent action does not have min_interval {self.label}')
        self._min_interval = test_step.min_interval

        if test_step.max_interval is None:
            raise UnexpectedActionCreationError(
                f'SubscribeEvent action does not have max_interval {self.label}')
        self._max_interval = test_step.max_interval

    async def run_action(self, dev_ctrl: ChipDeviceController) -> _ActionResult:
        try:
            urgent = 0
            request = [(self._endpoint, self._request_object, urgent)]
            subscription = await dev_ctrl.ReadEvent(self._node_id, events=request, eventNumberFilter=self._event_number_filter,
                                                    reportInterval=(self._min_interval, self._max_interval),
                                                    keepSubscriptions=False)
        except chip.interaction_model.InteractionModelError as error:
            return _ActionResult(status=_ActionStatus.ERROR, response=error)

        self._context.subscriptions.append(subscription)
        output_queue = self._context.subscription_callback_result_queue.get(self._event_name,
                                                                            None)
        if output_queue is None:
            output_queue = queue.SimpleQueue()
            self._context.subscription_callback_result_queue[self._event_name] = output_queue

        while not output_queue.empty():
            output_queue.get(block=False)

        subscription_handler = EventChangeAccumulator(self.label, self._request_object, output_queue)

        subscription.SetEventUpdateCallback(subscription_handler)

        events = subscription.GetEvents()
        response = EventResponse(event_result_list=events)
        return _ActionResult(status=_ActionStatus.SUCCESS, response=response)


class WriteAttributeAction(BaseAction):
    '''Single write attribute action to be executed.'''

    def __init__(self, test_step, cluster: str, context: _ExecutionContext):
        '''Converts 'test_step' to write attribute action that can execute with ChipDeviceController.

        Args:
          'test_step': Step containing information required to run write attribute action.
          'cluster': Name of cluster write attribute action is targeting.
          'context': Contains test-wide common objects such as DataModelLookup instance.
        Raises:
          UnexpectedActionCreationError: Raised if there is an unexpected parsing error.
        '''
        super().__init__(test_step)
        self._attribute_name = to_pascal_case(test_step.attribute)
        self._busy_wait_ms = test_step.busy_wait_ms
        self._cluster = cluster
        self._endpoint = test_step.endpoint
        self._interation_timeout_ms = test_step.timed_interaction_timeout_ms
        self._node_id = test_step.node_id
        self._group_id = test_step.group_id
        self._request_object = None

        if self._node_id is None and self._group_id is None:
            raise UnexpectedActionCreationError(
                'Both node_id and group_id are None, at least one needs to be provided')

        attribute = context.data_model_lookup.get_attribute(
            self._cluster, self._attribute_name)
        if attribute is None:
            raise UnexpectedActionCreationError(
                f'WriteAttribute failed to find cluster:{self._cluster} '
                f'Attribute:{self._attribute_name}')

        if not test_step.arguments:
            raise UnexpectedActionCreationError(f'WriteAttribute action does have arguments {self.label}')

        args = test_step.arguments['values']
        if len(args) != 1:
            raise UnexpectedActionCreationError('WriteAttribute is trying to write multiple values')
        request_data_as_dict = args[0]
        try:
            # TODO this is an ugly hack
            request_data = Converter.convert_to_data_model_type(
                request_data_as_dict['value'], attribute.attribute_type.Type)
        except ValueError:
            raise UnexpectedActionCreationError('Could not covert yaml type')

        # Create a cluster object for the request from the provided YAML data.
        self._request_object = attribute(request_data)

    async def run_action(self, dev_ctrl: ChipDeviceController) -> _ActionResult:
        try:
            if self._group_id:
                resp = dev_ctrl.WriteGroupAttribute(self._group_id, [(self._request_object,)],
                                                    busyWaitMs=self._busy_wait_ms)
            else:
                resp = await dev_ctrl.WriteAttribute(self._node_id, [(self._endpoint, self._request_object)],
                                                     timedRequestTimeoutMs=self._interation_timeout_ms,
                                                     busyWaitMs=self._busy_wait_ms)
        except chip.interaction_model.InteractionModelError as error:
            return _ActionResult(status=_ActionStatus.ERROR, response=error)

        # Group writes are expected to have no response upon success.
        if self._group_id and len(resp) == 0:
            return _ActionResult(status=_ActionStatus.SUCCESS, response=None)

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
          UnexpectedActionCreationError: Raised if the expected queue does not exist.
        '''
        super().__init__(test_step)
        if test_step.attribute is not None:
            queue_name = to_pascal_case(test_step.attribute)
        elif test_step.event is not None:
            queue_name = to_pascal_case(test_step.event)
        else:
            raise UnexpectedActionCreationError(
                'WaitForReport needs to wait on either attribute or event, neither were provided')

        self._output_queue = context.subscription_callback_result_queue.get(queue_name, None)
        if self._output_queue is None:
            raise UnexpectedActionCreationError('Could not find output queue')

    async def run_action(self, dev_ctrl: ChipDeviceController) -> _ActionResult:
        try:
            # While there should be a timeout here provided by the test, the current codegen version
            # of YAML tests doesn't have a per test step timeout, only a global timeout for the
            # entire test. For that reason we default to a 30 second timeout.
            item = self._output_queue.get(block=True, timeout=30)
        except queue.Empty:
            return _ActionResult(status=_ActionStatus.ERROR, response=None)

        if isinstance(item, _AttributeSubscriptionCallbackResult):
            return item.result
        return item.result


class CommissionerCommandAction(BaseAction):
    '''Single Commissioner Command action to be executed.'''

    def __init__(self, test_step):
        '''Converts 'test_step' to commissioner command action.

        Args:
          'test_step': Step containing information required to run wait for report action.
        Raises:
          UnexpectedActionCreationError: Raised if the expected queue does not exist.
        '''
        super().__init__(test_step)
        self._command = test_step.command
        if test_step.command == 'GetCommissionerNodeId':
            # Just setting the self._command is enough for run_action below.
            pass
        elif test_step.command in ('PairWithCode', 'EstablishPASESession'):
            args = test_step.arguments['values']
            request_data_as_dict = Converter.convert_list_of_name_value_pair_to_dict(args)
            self._setup_payload = request_data_as_dict['payload']
            self._node_id = request_data_as_dict['nodeId']
        else:
            raise UnexpectedActionCreationError(f'Unexpected CommisionerCommand {test_step.command}')

    async def run_action(self, dev_ctrl: ChipDeviceController) -> _ActionResult:
        if self._command == 'GetCommissionerNodeId':
            return _ActionResult(status=_ActionStatus.SUCCESS, response=_GetCommissionerNodeIdResult(dev_ctrl.nodeId))

        try:
            if self._command == 'PairWithCode':
                await dev_ctrl.CommissionWithCode(self._setup_payload, self._node_id)
            elif self._command == 'EstablishPASESession':
                await dev_ctrl.EstablishPASESession(self._setup_payload, self._node_id)
            return _ActionResult(status=_ActionStatus.SUCCESS, response=None)
        except ChipStackError:
            return _ActionResult(status=_ActionStatus.ERROR, response=None)


class DiscoveryCommandAction(BaseAction):
    """DiscoveryCommand implementation (FindCommissionable* methods)."""

    @staticmethod
    def _filter_for_step(test_step) -> Tuple[discovery.FilterType, Any]:
        """Given a test step, figure out the correct filters to give to
           DiscoverCommissionableNodes.
        """

        if test_step.command == 'FindCommissionable':
            return discovery.FilterType.NONE, None

        if test_step.command == 'FindCommissionableByCommissioningMode':
            # this is just a "_CM" subtype
            return discovery.FilterType.COMMISSIONING_MODE, None

        # all the items below require a "value" to use for filtering
        args = test_step.arguments['values']
        request_data_as_dict = Converter.convert_list_of_name_value_pair_to_dict(args)

        filter = request_data_as_dict['value']

        if test_step.command == 'FindCommissionableByDeviceType':
            return discovery.FilterType.DEVICE_TYPE, filter

        if test_step.command == 'FindCommissionableByLongDiscriminator':
            return discovery.FilterType.LONG_DISCRIMINATOR, filter

        if test_step.command == 'FindCommissionableByShortDiscriminator':
            return discovery.FilterType.SHORT_DISCRIMINATOR, filter

        if test_step.command == 'FindCommissionableByVendorId':
            return discovery.FilterType.VENDOR_ID, filter

        raise UnexpectedActionCreationError(f'Invalid command: {test_step.command}')

    def __init__(self, test_step):
        super().__init__(test_step)
        self.filterType, self.filter = DiscoveryCommandAction._filter_for_step(test_step)

    async def run_action(self, dev_ctrl: ChipDeviceController) -> _ActionResult:
        devices = await dev_ctrl.DiscoverCommissionableNodes(
            filterType=self.filterType, filter=self.filter, stopOnFirst=True, timeoutSecond=5)

        # Devices will be a list: [CommissionableNode(), ...]
        logging.info("Discovered devices: %r" % devices)

        if not devices:
            logging.error("No devices found")
            return _ActionResult(status=_ActionStatus.ERROR, response="NO DEVICES FOUND")
        elif len(devices) > 1:
            logging.warning("Commissionable discovery found multiple results!")

        return _ActionResult(status=_ActionStatus.SUCCESS, response=devices[0])


class NotImplementedAction(BaseAction):
    """Raises a "NOT YET IMPLEMENTED" exception when run."""

    def __init__(self, test_step, cluster, command):
        super().__init__(test_step)
        self.cluster = cluster
        self.command = command

    async def run_action(self, dev_ctrl: ChipDeviceController) -> _ActionResult:
        raise Exception(f"NOT YET IMPLEMENTED: {self.cluster}::{self.command}")


class ReplTestRunner:
    '''Test runner to encode/decode values from YAML test Parser for executing the TestStep.

    Uses ChipDeviceController from chip-repl to execute parsed YAML TestSteps.
    '''

    def __init__(self, test_spec_definition, certificate_authority_manager, alpha_dev_ctrl):
        self._test_spec_definition = test_spec_definition
        self._context = _ExecutionContext(data_model_lookup=PreDefinedDataModelLookup())
        self._certificate_authority_manager = certificate_authority_manager
        self._dev_ctrls = {}

        if alpha_dev_ctrl is not None:
            alpha_dev_ctrl.InitGroupTestingData()
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
        except ActionCreationError:
            return None

    def _attribute_read_action_factory(self, test_step, cluster: str):
        '''Creates read attribute command TestStep.

        Args:
          'test_step': Step containing information required to run read attribute action.
          'cluster': Name of cluster read attribute action is targeting.
        Returns:
          ReadAttributeAction if 'test_step' is a valid read attribute to be executed.
        '''
        return ReadAttributeAction(test_step, cluster, self._context)

    def _event_read_action_factory(self, test_step, cluster: str):
        return ReadEventAction(test_step, cluster, self._context)

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
        return SubscribeAttributeAction(test_step, cluster, self._context)

    def _attribute_subscribe_event_factory(self, test_step, cluster: str):
        '''Creates subscribe event command from TestStep provided.

        Args:
          'test_step': Step containing information required to run subscribe attribute action.
          'cluster': Name of cluster write attribute action is targeting.
        Returns:
          SubscribeEventAction if 'test_step' is a valid subscribe attribute to be executed.
        '''
        return SubscribeEventAction(test_step, cluster, self._context)

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
        return WriteAttributeAction(test_step, cluster, self._context)

    def _wait_for_commissionee_action_factory(self, test_step):
        return WaitForCommissioneeAction(test_step)

    def _wait_for_report_action_factory(self, test_step):
        return WaitForReportAction(test_step, self._context)

    def _commissioner_command_action_factory(self, test_step):
        return CommissionerCommandAction(test_step)

    def _default_pseudo_cluster(self, test_step):
        try:
            return DefaultPseudoCluster(test_step)
        except ActionCreationError as e:
            logger.warn(f"Failed create default pseudo cluster: {e}")
            return None

    def encode(self, request) -> Optional[BaseAction]:
        action: Optional[BaseAction] = None
        cluster = request.cluster.replace(' ', '').replace('/', '').replace('.', '')
        command = request.command
        if cluster == 'CommissionerCommands':
            return self._commissioner_command_action_factory(request)
        # Some of the tests contain 'cluster over-rides' that refer to a different
        # cluster than that specified in 'config'.

        elif cluster == 'DiscoveryCommands':
            return DiscoveryCommandAction(request)
        elif cluster == 'DelayCommands' and command == 'WaitForCommissionee':
            action = self._wait_for_commissionee_action_factory(request)
        elif command == 'writeAttribute':
            action = self._attribute_write_action_factory(request, cluster)
        elif command == 'readAttribute':
            action = self._attribute_read_action_factory(request, cluster)
        elif command == 'readEvent':
            action = self._event_read_action_factory(request, cluster)
        elif command == 'subscribeAttribute':
            action = self._attribute_subscribe_action_factory(request, cluster)
        elif command == 'subscribeEvent':
            action = self._attribute_subscribe_event_factory(request, cluster)
        elif command == 'waitForReport':
            action = self._wait_for_report_action_factory(request)
        else:
            action = self._invoke_action_factory(request, cluster)

        if action is None:
            # Now we try to create a default pseudo cluster.
            action = self._default_pseudo_cluster(request)

        if action is None:
            logger.warn(f"Failed to parse {request.label}")
        return action

    def decode(self, result: _ActionResult):
        # If this is a generic response, there is nothing to do.
        if result.response is None:
            # TODO Once yamltest and idl python packages are properly packaged as a single module
            # the type we are returning will be formalized. For now TestStep.post_process_response
            # expects this particular case to be sent an empty dict or a dict with an error.
            return {} if result.status == _ActionStatus.SUCCESS else {'error': 'FAILURE'}

        response = result.response

        decoded_response = {}
        if isinstance(response, dict):
            return response

        if isinstance(response, chip.interaction_model.InteractionModelError):
            decoded_response['error'] = to_snake_case(response.status.name).upper()
            decoded_response['clusterError'] = response.clusterStatus
            return decoded_response

        if isinstance(response, chip.interaction_model.Status):
            decoded_response['error'] = to_snake_case(response.name).upper()
            return decoded_response

        if isinstance(response, _GetCommissionerNodeIdResult):
            decoded_response['value'] = {'nodeId': response.node_id}
            return decoded_response

        if isinstance(response, chip.discovery.CommissionableNode):
            decoded_response['value'] = {
                'instanceName': response.instanceName,
                'hostName': response.hostName,
                'port': response.port,
                'longDiscriminator': response.longDiscriminator,
                'vendorId': response.vendorId,
                'productId': response.productId,
                'commissioningMode': response.commissioningMode,
                'deviceType': response.deviceType,
                'deviceName': response.deviceName,
                'pairingInstruction': response.pairingInstruction,
                'pairingHint': response.pairingHint,
                'mrpRetryIntervalIdle': response.mrpRetryIntervalIdle,
                'mrpRetryIntervalActive': response.mrpRetryIntervalActive,
                'mrpRetryActiveThreshold': response.mrpRetryActiveThreshold,
                'supportsTcpClient': response.supportsTcpClient,
                'supportsTcpServer': response.supportsTcpServer,
                'isICDOperatingAsLIT': response.isICDOperatingAsLIT,
                'addresses': response.addresses,
                'rotatingId': response.rotatingId,

                # derived values
                'rotatingIdLen': 0 if not response.rotatingId else len(response.rotatingId),
                'numIPs': len(response.addresses),

            }
            return decoded_response

        if isinstance(response, EventResponse):
            if not response.event_result_list:
                # This means that the event result we got back was empty, below is how we
                # represent this.
                decoded_response = [{}]
                return decoded_response
            decoded_response = []
            for event in response.event_result_list:
                if event.Status != chip.interaction_model.Status.Success:
                    error_message = to_snake_case(event.Status.name).upper()
                    decoded_response.append({'error': error_message})
                    continue
                cluster_id = event.Header.ClusterId
                cluster_name = self._test_spec_definition.get_cluster_name(cluster_id)
                event_id = event.Header.EventId
                event_number = event.Header.EventNumber
                event_name = self._test_spec_definition.get_event_name(cluster_id, event_id)
                event_definition = self._test_spec_definition.get_event_by_name(cluster_name, event_name)
                is_fabric_scoped = bool(event_definition.is_fabric_sensitive)
                decoded_event = Converter.from_data_model_to_test_definition(
                    self._test_spec_definition, cluster_name, event_definition.fields, event.Data, is_fabric_scoped)
                decoded_response.append({'value': decoded_event, 'eventNumber': event_number})
            return decoded_response

        if isinstance(response, ChipStackError):
            decoded_response['error'] = 'FAILURE'
            return decoded_response

        cluster_name = self._test_spec_definition.get_cluster_name(response.cluster_id)
        if cluster_name is None:
            raise Exception("Cannot find cluster name for id 0x%0X / %d" % (response.cluster_id, response.cluster_id))

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
                # 'response.value' directly if it is a list and mapping to int if not a list.
                if isinstance(response.value, list):
                    decoded_response['value'] = response.value
                else:
                    decoded_response['value'] = Converter.from_data_model_to_test_definition(
                        self._test_spec_definition, cluster_name, int, response.value)

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
                dev_ctrl.InitGroupTestingData()
                self._dev_ctrls[action.identity] = dev_ctrl
        else:
            dev_ctrl = self._dev_ctrls['alpha']

        return dev_ctrl

    async def execute(self, action: BaseAction):
        dev_ctrl = self._get_dev_ctrl(action)
        return await action.run_action(dev_ctrl)

    def shutdown(self):
        for subscription in self._context.subscriptions:
            subscription.Shutdown()
