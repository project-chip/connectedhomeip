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
"""Module for errors thrown from snippet client objects."""
# TODO(mhaoli): Package `mobly.snippet` should not import errors from
# android_device_lib. However, android_device_lib.DeviceError is the base error
# for the errors thrown from Android snippet clients and device controllers.
# We should resolve this legacy problem.
from mobly.controllers.android_device_lib import errors


class Error(errors.DeviceError):
  """Root error type for snippet clients."""


class ServerRestoreConnectionError(Error):
  """Raised when failed to restore the connection with the snippet server."""


class ServerStartError(Error):
  """Raised when failed to start the snippet server."""


class ServerStartProtocolError(ServerStartError):
  """Raised when protocol reported by the server startup process is unknown."""


class ServerStartPreCheckError(Error):
  """Raised when prechecks for starting the snippet server failed.

  Here are some precheck examples:
  * Whether the required software is installed on the device.
  * Whether the configuration file required by the server startup process
    is available.
  """


class ApiError(Error):
  """Raised when remote API reported an error."""


class ProtocolError(Error):
  """Raised when there was an error in exchanging data with server."""

  NO_RESPONSE_FROM_HANDSHAKE = 'No response from handshake.'
  NO_RESPONSE_FROM_SERVER = (
      'No response from server. Check the device logcat for crashes.'
  )
  MISMATCHED_API_ID = 'RPC request-response ID mismatch.'
  RESPONSE_MISSING_FIELD = 'Missing required field in the RPC response: %s.'


class ServerDiedError(Error):
  """Raised if the snippet server died before all tests finish."""


# Error types for callback handlers
class CallbackHandlerBaseError(errors.DeviceError):
  """Base error type for snippet clients."""


class CallbackHandlerTimeoutError(Error):
  """Raised if the expected event does not occur within the time limit."""
