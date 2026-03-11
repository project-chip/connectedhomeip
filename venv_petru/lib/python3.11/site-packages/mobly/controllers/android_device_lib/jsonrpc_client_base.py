# Copyright 2016 Google Inc.
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
"""Base class for clients that communicate with apps over a JSON RPC interface.

The JSON protocol expected by this module is:

.. code-block:: json

  Request:
  {
    "id": <monotonically increasing integer containing the ID of
         this request>
    "method": <string containing the name of the method to execute>
    "params": <JSON array containing the arguments to the method>
  }

  Response:
  {
    "id": <int id of request that this response maps to>,
    "result": <Arbitrary JSON object containing the result of
           executing the method. If the method could not be
           executed or returned void, contains 'null'.>,
    "error": <String containing the error thrown by executing the
          method. If no error occurred, contains 'null'.>
    "callback": <String that represents a callback ID used to
           identify events associated with a particular
           CallbackHandler object.>
  }
"""

# When the Python library `socket.create_connection` call is made, it indirectly
# calls `import encodings.idna` through the `socket.getaddrinfo` method.
# However, this chain of function calls is apparently not thread-safe in
# embedded Python environments. So, pre-emptively import and cache the encoder.
# See https://bugs.python.org/issue17305 for more details.
try:
  import encodings.idna
except ImportError:
  # Some implementations of Python (e.g. IronPython) do not support the`idna`
  # encoding, so ignore import failures based on that.
  pass

import abc
import json
import socket
import threading

from mobly.controllers.android_device_lib import callback_handler
from mobly.snippet import errors

# UID of the 'unknown' jsonrpc session. Will cause creation of a new session.
UNKNOWN_UID = -1

# Maximum time to wait for the socket to open on the device.
_SOCKET_CONNECTION_TIMEOUT = 60

# Maximum time to wait for a response message on the socket.
_SOCKET_READ_TIMEOUT = callback_handler.MAX_TIMEOUT

# Maximum logging length of Rpc response in DEBUG level when verbose logging is
# off.
_MAX_RPC_RESP_LOGGING_LENGTH = 1024

# Aliases of error types for backward compatibility.
Error = errors.Error
AppStartError = errors.ServerStartError
AppRestoreConnectionError = errors.ServerRestoreConnectionError
ApiError = errors.ApiError
ProtocolError = errors.ProtocolError


class JsonRpcCommand:
  """Commands that can be invoked on all jsonrpc clients.

  INIT: Initializes a new session.
  CONTINUE: Creates a connection.
  """

  INIT = 'initiate'
  CONTINUE = 'continue'


class JsonRpcClientBase(abc.ABC):
  """Base class for jsonrpc clients that connect to remote servers.

  Connects to a remote device running a jsonrpc-compatible app. Before opening
  a connection a port forward must be setup to go over usb. This be done using
  adb.forward([local, remote]). Once the port has been forwarded it can be
  used in this object as the port of communication.

  Attributes:
    host_port: (int) The host port of this RPC client.
    device_port: (int) The device port of this RPC client.
    app_name: (str) The user-visible name of the app being communicated
          with.
    uid: (int) The uid of this session.
  """

  def __init__(self, app_name, ad):
    """
    Args:
      app_name: (str) The user-visible name of the app being communicated
        with.
      ad: (AndroidDevice) The device object associated with a client.
    """
    self.host_port = None
    self.device_port = None
    self.app_name = app_name
    self._ad = ad
    self.log = self._ad.log
    self.uid = None
    self._client = None  # prevent close errors on connect failure
    self._conn = None
    self._counter = None
    self._lock = threading.Lock()
    self._event_client = None
    self.verbose_logging = True

  def __del__(self):
    self.disconnect()

  # Methods to be implemented by subclasses.

  def start_app_and_connect(self):
    """Starts the server app on the android device and connects to it.

    After this, the self.host_port and self.device_port attributes must be
    set.

    Must be implemented by subclasses.

    Raises:
      AppStartError: When the app was not able to be started.
    """

  def stop_app(self):
    """Kills any running instance of the app.

    Must be implemented by subclasses.
    """

  def restore_app_connection(self, port=None):
    """Reconnects to the app after device USB was disconnected.

    Instead of creating new instance of the client:
      - Uses the given port (or finds a new available host_port if none is
      given).
      - Tries to connect to remote server with selected port.

    Must be implemented by subclasses.

    Args:
      port: If given, this is the host port from which to connect to remote
        device port. If not provided, find a new available port as host
        port.

    Raises:
      AppRestoreConnectionError: When the app was not able to be
      reconnected.
    """

  def _start_event_client(self):
    """Starts a separate JsonRpc client to the same session for propagating
    events.

    This is an optional function that should only implement if the client
    utilizes the snippet event mechanism.

    Returns:
      A JsonRpc Client object that connects to the same session as the
      one on which this function is called.
    """

  # Rest of the client methods.

  def connect(self, uid=UNKNOWN_UID, cmd=JsonRpcCommand.INIT):
    """Opens a connection to a JSON RPC server.

    Opens a connection to a remote client. The connection attempt will time
    out if it takes longer than _SOCKET_CONNECTION_TIMEOUT seconds. Each
    subsequent operation over this socket will time out after
    _SOCKET_READ_TIMEOUT seconds as well.

    Args:
      uid: int, The uid of the session to join, or UNKNOWN_UID to start a
        new session.
      cmd: JsonRpcCommand, The command to use for creating the connection.

    Raises:
      IOError: Raised when the socket times out from io error
      socket.timeout: Raised when the socket waits to long for connection.
      ProtocolError: Raised when there is an error in the protocol.
    """
    self._counter = self._id_counter()
    try:
      self._conn = socket.create_connection(
          ('localhost', self.host_port), _SOCKET_CONNECTION_TIMEOUT
      )
    except ConnectionRefusedError as err:
      # Retry using '127.0.0.1' for IPv4 enabled machines that only resolve
      # 'localhost' to '[::1]'.
      self.log.debug(
          'Failed to connect to localhost, trying 127.0.0.1: {}'.format(
              str(err)
          )
      )
      self._conn = socket.create_connection(
          ('127.0.0.1', self.host_port), _SOCKET_CONNECTION_TIMEOUT
      )

    self._conn.settimeout(_SOCKET_READ_TIMEOUT)
    self._client = self._conn.makefile(mode='brw')

    resp = self._cmd(cmd, uid)
    if not resp:
      raise ProtocolError(self._ad, ProtocolError.NO_RESPONSE_FROM_HANDSHAKE)
    result = json.loads(str(resp, encoding='utf8'))
    if result['status']:
      self.uid = result['uid']
    else:
      self.uid = UNKNOWN_UID

  def disconnect(self):
    """Close the connection to the snippet server on the device.

    This is a unilateral disconnect from the client side, without tearing down
    the snippet server running on the device.

    The connection to the snippet server can be re-established by calling
    `SnippetClient.restore_app_connection`.
    """
    try:
      self.close_socket_connection()
    finally:
      # Always clear the host port as part of the disconnect step.
      self.clear_host_port()

  def close_socket_connection(self):
    """Closes the socket connection to the server."""
    if self._conn:
      self._conn.close()
      self._conn = None

  def clear_host_port(self):
    """Stops the adb port forwarding of the host port used by this client."""
    if self.host_port:
      self._ad.adb.forward(['--remove', 'tcp:%d' % self.host_port])
      self.host_port = None

  def _client_send(self, msg):
    """Sends an Rpc message through the connection.

    Args:
      msg: string, the message to send.

    Raises:
      Error: a socket error occurred during the send.
    """
    try:
      self._client.write(msg.encode('utf8') + b'\n')
      self._client.flush()
      self.log.debug('Snippet sent %s.', msg)
    except socket.error as e:
      raise Error(
          self._ad,
          'Encountered socket error "%s" sending RPC message "%s"' % (e, msg),
      )

  def _client_receive(self):
    """Receives the server's response of an Rpc message.

    Returns:
      Raw byte string of the response.

    Raises:
      Error: a socket error occurred during the read.
    """
    try:
      response = self._client.readline()
      if self.verbose_logging:
        self.log.debug('Snippet received: %s', response)
      else:
        if _MAX_RPC_RESP_LOGGING_LENGTH >= len(response):
          self.log.debug('Snippet received: %s', response)
        else:
          self.log.debug(
              'Snippet received: %s... %d chars are truncated',
              response[:_MAX_RPC_RESP_LOGGING_LENGTH],
              len(response) - _MAX_RPC_RESP_LOGGING_LENGTH,
          )
      return response
    except socket.error as e:
      raise Error(
          self._ad, 'Encountered socket error reading RPC response "%s"' % e
      )

  def _cmd(self, command, uid=None):
    """Send a command to the server.

    Args:
      command: str, The name of the command to execute.
      uid: int, the uid of the session to send the command to.

    Returns:
      The line that was written back.
    """
    if not uid:
      uid = self.uid
    self._client_send(json.dumps({'cmd': command, 'uid': uid}))
    return self._client_receive()

  def _rpc(self, method, *args):
    """Sends an rpc to the app.

    Args:
      method: str, The name of the method to execute.
      args: any, The args of the method.

    Returns:
      The result of the rpc.

    Raises:
      ProtocolError: Something went wrong with the protocol.
      ApiError: The rpc went through, however executed with errors.
    """
    with self._lock:
      apiid = next(self._counter)
      data = {'id': apiid, 'method': method, 'params': args}
      request = json.dumps(data)
      self._client_send(request)
      response = self._client_receive()
    if not response:
      raise ProtocolError(self._ad, ProtocolError.NO_RESPONSE_FROM_SERVER)
    result = json.loads(str(response, encoding='utf8'))
    if result['error']:
      raise ApiError(self._ad, result['error'])
    if result['id'] != apiid:
      raise ProtocolError(self._ad, ProtocolError.MISMATCHED_API_ID)
    if result.get('callback') is not None:
      if self._event_client is None:
        self._event_client = self._start_event_client()
      return callback_handler.CallbackHandler(
          callback_id=result['callback'],
          event_client=self._event_client,
          ret_value=result['result'],
          method_name=method,
          ad=self._ad,
      )
    return result['result']

  def disable_hidden_api_blacklist(self):
    """If necessary and possible, disables hidden api blacklist."""
    version_codename = self._ad.build_info['build_version_codename']
    sdk_version = int(self._ad.build_info['build_version_sdk'])
    # we check version_codename in addition to sdk_version because P builds
    # in development report sdk_version 27, but still enforce the blacklist.
    if self._ad.is_rootable and (sdk_version >= 28 or version_codename == 'P'):
      self._ad.adb.shell(
          'settings put global hidden_api_blacklist_exemptions "*"'
      )

  def __getattr__(self, name):
    """Wrapper for python magic to turn method calls into RPC calls."""

    def rpc_call(*args):
      return self._rpc(name, *args)

    return rpc_call

  def _id_counter(self):
    i = 0
    while True:
      yield i
      i += 1

  def set_snippet_client_verbose_logging(self, verbose):
    """Switches verbose logging. True for logging full RPC response.

    By default it will only write max_rpc_return_value_length for Rpc return
    strings. If you need to see full message returned from Rpc, please turn
    on verbose logging.

    max_rpc_return_value_length will set to 1024 by default, the length
    contains full Rpc response in Json format, included 1st element "id".

    Args:
      verbose: bool. If True, turns on verbose logging, if False turns off
    """
    self._ad.log.info('Set verbose logging to %s.', verbose)
    self.verbose_logging = verbose
