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
"""The callback handler V2 module for Android Mobly Snippet Lib."""

from mobly.snippet import callback_handler_base
from mobly.snippet import errors

# The timeout error meesage when pulling events from the server
TIMEOUT_ERROR_MESSAGE = 'EventSnippetException: timeout.'


class CallbackHandlerV2(callback_handler_base.CallbackHandlerBase):
  """The callback handler V2 class for Android Mobly Snippet Lib."""

  def callEventWaitAndGetRpc(self, callback_id, event_name, timeout_sec):
    """Waits and returns an existing CallbackEvent for the specified identifier.

    This function calls snippet lib's eventWaitAndGet RPC.

    Args:
      callback_id: str, the callback identifier.
      event_name: str, the callback name.
      timeout_sec: float, the number of seconds to wait for the event.

    Returns:
      The event dictionary.

    Raises:
      errors.CallbackHandlerTimeoutError: The expected event does not occur
        within the time limit.
    """
    timeout_ms = int(timeout_sec * 1000)
    try:
      return self._event_client.eventWaitAndGet(
          callback_id, event_name, timeout_ms
      )
    except Exception as e:
      if TIMEOUT_ERROR_MESSAGE in str(e):
        raise errors.CallbackHandlerTimeoutError(
            self._device,
            (
                f'Timed out after waiting {timeout_sec}s for event '
                f'"{event_name}" triggered by {self._method_name} '
                f'({self.callback_id}).'
            ),
        ) from e
      raise

  def callEventGetAllRpc(self, callback_id, event_name):
    """Gets all existing events for the specified identifier without waiting.

    This function calls snippet lib's eventGetAll RPC.

    Args:
      callback_id: str, the callback identifier.
      event_name: str, the callback name.

    Returns:
      A list of event dictionaries.
    """
    return self._event_client.eventGetAll(callback_id, event_name)
