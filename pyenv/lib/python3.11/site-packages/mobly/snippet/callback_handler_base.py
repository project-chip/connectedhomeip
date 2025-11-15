# Copyright 2022 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
"""Module for the base class to handle Mobly Snippet Lib's callback events."""
import abc
import time

from mobly.snippet import callback_event
from mobly.snippet import errors


class CallbackHandlerBase(abc.ABC):
  """Base class for handling Mobly Snippet Lib's callback events.

  All the events handled by a callback handler are originally triggered by one
  async RPC call. All the events are tagged with a callback_id specific to a
  call to an async RPC method defined on the server side.

  The raw message representing an event looks like:

  .. code-block:: python

    {
      'callbackId': <string, callbackId>,
      'name': <string, name of the event>,
      'time': <long, epoch time of when the event was created on the
        server side>,
      'data': <dict, extra data from the callback on the server side>
    }

  Each message is then used to create a CallbackEvent object on the client
  side.

  Attributes:
    ret_value: any, the direct return value of the async RPC call.
  """

  def __init__(
      self,
      callback_id,
      event_client,
      ret_value,
      method_name,
      device,
      rpc_max_timeout_sec,
      default_timeout_sec=120,
  ):
    """Initializes a callback handler base object.

    Args:
      callback_id: str, the callback ID which associates with a group of
        callback events.
      event_client: SnippetClientV2, the client object used to send RPC to the
        server and receive response.
      ret_value: any, the direct return value of the async RPC call.
      method_name: str, the name of the executed Async snippet function.
      device: DeviceController, the device object associated with this handler.
      rpc_max_timeout_sec: float, maximum time for sending a single RPC call.
      default_timeout_sec: float, the default timeout for this handler. It
        must be no longer than rpc_max_timeout_sec.
    """
    self._id = callback_id
    self.ret_value = ret_value
    self._device = device
    self._event_client = event_client
    self._method_name = method_name

    if rpc_max_timeout_sec < default_timeout_sec:
      raise ValueError(
          'The max timeout of a single RPC must be no smaller '
          'than the default timeout of the callback handler. '
          f'Got rpc_max_timeout_sec={rpc_max_timeout_sec}, '
          f'default_timeout_sec={default_timeout_sec}.'
      )
    self._rpc_max_timeout_sec = rpc_max_timeout_sec
    self._default_timeout_sec = default_timeout_sec

  @property
  def rpc_max_timeout_sec(self):
    """Maximum time for sending a single RPC call."""
    return self._rpc_max_timeout_sec

  @property
  def default_timeout_sec(self):
    """Default timeout used by this callback handler."""
    return self._default_timeout_sec

  @property
  def callback_id(self):
    """The callback ID which associates a group of callback events."""
    return self._id

  @abc.abstractmethod
  def callEventWaitAndGetRpc(self, callback_id, event_name, timeout_sec):
    """Calls snippet lib's RPC to wait for a callback event.

    Override this method to use this class with various snippet lib
    implementations.

    This function waits and gets a CallbackEvent with the specified identifier
    from the server. It will raise a timeout error if the expected event does
    not occur within the time limit.

    Args:
      callback_id: str, the callback identifier.
      event_name: str, the callback name.
      timeout_sec: float, the number of seconds to wait for the event. It is
        already checked that this argument is no longer than the max timeout
        of a single RPC.

    Returns:
      The event dictionary.

    Raises:
      errors.CallbackHandlerTimeoutError: Raised if the expected event does not
        occur within the time limit.
    """

  @abc.abstractmethod
  def callEventGetAllRpc(self, callback_id, event_name):
    """Calls snippet lib's RPC to get all existing snippet events.

    Override this method to use this class with various snippet lib
    implementations.

    This function gets all existing events in the server with the specified
    identifier without waiting.

    Args:
      callback_id: str, the callback identifier.
      event_name: str, the callback name.

    Returns:
      A list of event dictionaries.
    """

  def waitAndGet(self, event_name, timeout=None):
    """Waits and gets a CallbackEvent with the specified identifier.

    It will raise a timeout error if the expected event does not occur within
    the time limit.

    Args:
      event_name: str, the name of the event to get.
      timeout: float, the number of seconds to wait before giving up. If None,
        it will be set to self.default_timeout_sec.

    Returns:
      CallbackEvent, the oldest entry of the specified event.

    Raises:
      errors.CallbackHandlerBaseError: If the specified timeout is longer than
        the max timeout supported.
      errors.CallbackHandlerTimeoutError: The expected event does not occur
        within the time limit.
    """
    if timeout is None:
      timeout = self.default_timeout_sec

    if timeout:
      if timeout > self.rpc_max_timeout_sec:
        raise errors.CallbackHandlerBaseError(
            self._device,
            f'Specified timeout {timeout} is longer than max timeout '
            f'{self.rpc_max_timeout_sec}.',
        )

    raw_event = self.callEventWaitAndGetRpc(self._id, event_name, timeout)
    return callback_event.from_dict(raw_event)

  def waitForEvent(self, event_name, predicate, timeout=None):
    """Waits for an event of the specific name that satisfies the predicate.

    This call will block until the expected event has been received or time
    out.

    The predicate function defines the condition the event is expected to
    satisfy. It takes an event and returns True if the condition is
    satisfied, False otherwise.

    Note all events of the same name that are received but don't satisfy
    the predicate will be discarded and not be available for further
    consumption.

    Args:
      event_name: str, the name of the event to wait for.
      predicate: function, a function that takes an event (dictionary) and
        returns a bool.
      timeout: float, the number of seconds to wait before giving up. If None,
        it will be set to self.default_timeout_sec.

    Returns:
      dictionary, the event that satisfies the predicate if received.

    Raises:
      errors.CallbackHandlerTimeoutError: raised if no event that satisfies the
        predicate is received after timeout seconds.
    """
    if timeout is None:
      timeout = self.default_timeout_sec

    deadline = time.perf_counter() + timeout
    while time.perf_counter() <= deadline:
      single_rpc_timeout = deadline - time.perf_counter()
      if single_rpc_timeout < 0:
        break

      single_rpc_timeout = min(single_rpc_timeout, self.rpc_max_timeout_sec)
      try:
        event = self.waitAndGet(event_name, single_rpc_timeout)
      except errors.CallbackHandlerTimeoutError:
        # Ignoring errors.CallbackHandlerTimeoutError since we need to throw
        # one with a more specific message.
        break
      if predicate(event):
        return event

    raise errors.CallbackHandlerTimeoutError(
        self._device,
        f'Timed out after {timeout}s waiting for an "{event_name}" event that '
        f'satisfies the predicate "{predicate.__name__}".',
    )

  def getAll(self, event_name):
    """Gets all existing events in the server with the specified identifier.

    This is a non-blocking call.

    Args:
      event_name: str, the name of the event to get.

    Returns:
      A list of CallbackEvent, each representing an event from the Server side.
    """
    raw_events = self.callEventGetAllRpc(self._id, event_name)
    return [callback_event.from_dict(msg) for msg in raw_events]
