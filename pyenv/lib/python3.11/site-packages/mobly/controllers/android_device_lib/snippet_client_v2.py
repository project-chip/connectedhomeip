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
"""Snippet Client V2 for Interacting with Snippet Server on Android Device."""

import dataclasses
import enum
import json
import re
import socket
from typing import Dict, Union

from mobly import utils
from mobly.controllers.android_device_lib import adb
from mobly.controllers.android_device_lib import callback_handler_v2
from mobly.controllers.android_device_lib import errors as android_device_lib_errors
from mobly.snippet import client_base
from mobly.snippet import errors

# The package of the instrumentation runner used for mobly snippet
_INSTRUMENTATION_RUNNER_PACKAGE = (
    'com.google.android.mobly.snippet.SnippetRunner'
)

# The command template to start the snippet server
_LAUNCH_CMD = (
    '{shell_cmd} am instrument {user} -w -e action start'
    ' {instrument_options}'
    f' {{snippet_package}}/{_INSTRUMENTATION_RUNNER_PACKAGE}'
)

_SNIPPET_SERVER_START_ERROR_DEBUG_TIP = """
Invalid instrumentation result log received during snippet server start:
{instrumentation_result}

For debugging, please check the following:
1. Check the server process stdout attached below.
2. The snippet server logs within the device's logcat file. Search for
   "SNIPPET START" to locate the relevant process ID.

Server process stdout:
{server_start_stdout}
"""

# The command template to stop the snippet server
_STOP_CMD = (
    'am instrument {user} -w -e action stop {snippet_package}/'
    f'{_INSTRUMENTATION_RUNNER_PACKAGE}'
)

# The default timeout for running `_STOP_CMD`.
_STOP_CMD_TIMEOUT_SEC = 30

# Major version of the launch and communication protocol being used by this
# client.
# Incrementing this means that compatibility with clients using the older
# version is broken. Avoid breaking compatibility unless there is no other
# choice.
_PROTOCOL_MAJOR_VERSION = 1

# Minor version of the launch and communication protocol.
# Increment this when new features are added to the launch and communication
# protocol that are backwards compatible with the old protocol and don't break
# existing clients.
_PROTOCOL_MINOR_VERSION = 0

# Test that uses UiAutomation requires the shell session to be maintained while
# test is in progress. However, this requirement does not hold for the test that
# deals with device disconnection (Once device disconnects, the shell session
# that started the instrument ends, and UiAutomation fails with error:
# "UiAutomation not connected"). To keep the shell session and redirect
# stdin/stdout/stderr, use "setsid" or "nohup" while launching the
# instrumentation test. Because these commands may not be available in every
# Android system, try to use it only if at least one exists.
_SETSID_COMMAND = 'setsid'

_NOHUP_COMMAND = 'nohup'

# UID of the 'unknown' JSON RPC session. Will cause creation of a new session
# in the snippet server.
UNKNOWN_UID = -1

# Maximum time to wait for the socket to open on the device.
_SOCKET_CONNECTION_TIMEOUT = 60

# Maximum time to wait for a response message on the socket.
_SOCKET_READ_TIMEOUT = 60 * 10

# The default timeout for callback handlers returned by this client
_CALLBACK_DEFAULT_TIMEOUT_SEC = 60 * 2


@dataclasses.dataclass
class Config:
  """A configuration class.

  Attributes:
    am_instrument_options: The Android am instrument options used for
      controlling the `onCreate` process of the app under test. Note that this
      should only be used for controlling the app launch process, options for
      other purposes may not take effect and you should use snippet RPCs. This
      is because Mobly snippet runner changes the subsequent instrumentation
      process.
    user_id: The user id under which to launch the snippet process.
  """

  am_instrument_options: Dict[str, str] = dataclasses.field(
      default_factory=dict
  )
  user_id: Union[int, None] = None


class ConnectionHandshakeCommand(enum.Enum):
  """Commands to send to the server when sending the handshake request.

  After creating the socket connection, the client must send a handshake request
  to the server. When receiving the handshake request, the server will prepare
  to communicate with the client. According to the command in the request,
  the server will create a new session or reuse the current session.

  INIT: Initiates a new session and makes a connection with this session.
  CONTINUE: Makes a connection with the current session.
  """

  INIT = 'initiate'
  CONTINUE = 'continue'


class SnippetClientV2(client_base.ClientBase):
  """Snippet client V2 for interacting with snippet server on Android Device.

  For a description of the launch protocols, see the documentation in
  mobly-snippet-lib, SnippetRunner.java.

  We only list the public attributes introduced in this class. See base class
  documentation for other public attributes and communication protocols.

  Attributes:
    host_port: int, the host port used for communicating with the snippet
      server.
    device_port: int, the device port listened by the snippet server.
    uid: int, the uid of the server session with which this client communicates.
      Default is `UNKNOWN_UID` and it will be set to a positive number after
      the connection to the server is made successfully.
  """

  def __init__(self, package, ad, config=None):
    """Initializes the instance of Snippet Client V2.

    Args:
      package: str, see base class.
      ad: AndroidDevice, the android device object associated with this client.
      config: Config, the configuration object. See the docstring of the
        `Config` class for supported configurations.
    """
    super().__init__(package=package, device=ad)
    self.host_port = None
    self.device_port = None
    self.uid = UNKNOWN_UID
    self._adb = ad.adb
    self._user_id = None if config is None else config.user_id
    self._proc = None
    self._client = None  # keep it to prevent close errors on connect failure
    self._conn = None
    self._event_client = None
    self._config = config or Config()
    self._server_start_stdout = []

  @property
  def user_id(self):
    """The user id to use for this snippet client.

    All the operations of the snippet client should be used for a particular
    user. For more details, see the Android documentation of testing
    multiple users.

    Thus this value is cached and, once set, does not change through the
    lifecycles of this snippet client object. This caching also reduces the
    number of adb calls needed.

    Although for now self._user_id won't be modified once set, we use
    `property` to avoid issuing adb commands in the constructor.

    Returns:
      An integer of the user id.
    """
    if self._user_id is None:
      self._user_id = self._adb.current_user_id
    return self._user_id

  @property
  def is_alive(self):
    """Does the client have an active connection to the snippet server."""
    return self._conn is not None

  def before_starting_server(self):
    """Performs the preparation steps before starting the remote server.

    This function performs following preparation steps:
    * Validate that the Mobly Snippet app is available on the device.
    * Disable hidden api blocklist if necessary and possible.

    Raises:
      errors.ServerStartPreCheckError: if the server app is not installed
        for the current user.
    """
    self._validate_snippet_app_on_device()
    self._disable_hidden_api_blocklist()

  def _validate_snippet_app_on_device(self):
    """Validates the Mobly Snippet app is available on the device.

    To run as an instrumentation test, the Mobly Snippet app must already be
    installed and instrumented on the Android device.

    Raises:
      errors.ServerStartPreCheckError: if the server app is not installed
        for the current user.
    """
    # Validate that the Mobly Snippet app is installed for the current user.
    out = self._adb.shell(f'pm list package --user {self.user_id}')
    if not utils.grep(f'^package:{self.package}$', out):
      raise errors.ServerStartPreCheckError(
          self._device,
          f'{self.package} is not installed for user {self.user_id}.',
      )

    # Validate that the app is instrumented.
    out = self._adb.shell('pm list instrumentation')
    matched_out = utils.grep(
        f'^instrumentation:{self.package}/{_INSTRUMENTATION_RUNNER_PACKAGE}',
        out,
    )
    if not matched_out:
      raise errors.ServerStartPreCheckError(
          self._device,
          f'{self.package} is installed, but it is not instrumented.',
      )
    match = re.search(
        r'^instrumentation:(.*)\/(.*) \(target=(.*)\)$', matched_out[0]
    )
    target_name = match.group(3)
    # Validate that the instrumentation target is installed if it's not the
    # same as the snippet package.
    if target_name != self.package:
      out = self._adb.shell(f'pm list package --user {self.user_id}')
      if not utils.grep(f'^package:{target_name}$', out):
        raise errors.ServerStartPreCheckError(
            self._device,
            f'Instrumentation target {target_name} is not installed for user '
            f'{self.user_id}.',
        )

  def _disable_hidden_api_blocklist(self):
    """If necessary and possible, disables hidden api blocklist."""
    sdk_version = int(self._device.build_info['build_version_sdk'])
    if self._device.is_rootable and sdk_version >= 28:
      self._device.adb.shell(
          'settings put global hidden_api_blacklist_exemptions "*"'
      )

  def start_server(self):
    """Starts the server on the remote device.

    This function starts the snippet server with adb command, checks the
    protocol version of the server, parses device port from the server
    output and sets it to self.device_port.

    Raises:
      errors.ServerStartProtocolError: if the protocol reported by the server
        startup process is unknown.
      errors.ServerStartError: if failed to start the server or process the
        server output.
    """
    persists_shell_cmd = self._get_persisting_command()
    self.log.debug(
        'Snippet server for package %s is using protocol %d.%d',
        self.package,
        _PROTOCOL_MAJOR_VERSION,
        _PROTOCOL_MINOR_VERSION,
    )
    option_str = self._get_instrument_options_str()
    cmd = _LAUNCH_CMD.format(
        shell_cmd=persists_shell_cmd,
        user=self._get_user_command_string(),
        snippet_package=self.package,
        instrument_options=option_str,
    )
    self._proc = self._run_adb_cmd(cmd)

    # Check protocol version and get the device port
    self._server_start_stdout = []
    line = self._read_protocol_line()
    match = re.match('^SNIPPET START, PROTOCOL ([0-9]+) ([0-9]+)$', line)
    if not match or int(match.group(1)) != _PROTOCOL_MAJOR_VERSION:
      raise errors.ServerStartProtocolError(self._device, line)

    line = self._read_protocol_line()
    match = re.match('^SNIPPET SERVING, PORT ([0-9]+)$', line)
    if not match:
      message = _SNIPPET_SERVER_START_ERROR_DEBUG_TIP.format(
          instrumentation_result=line,
          server_start_stdout='\n'.join(self._server_start_stdout),
      )
      raise errors.ServerStartProtocolError(self._device, message)
    self.device_port = int(match.group(1))

  def _run_adb_cmd(self, cmd):
    """Starts a long-running adb subprocess and returns it immediately."""
    adb_cmd = [adb.ADB]
    if self._adb.serial:
      adb_cmd += ['-s', self._adb.serial]
    adb_cmd += ['shell', cmd]
    return utils.start_standing_subprocess(adb_cmd, shell=False)

  def _get_persisting_command(self):
    """Returns the path of a persisting command if available."""
    for command in [_SETSID_COMMAND, _NOHUP_COMMAND]:
      try:
        if command in self._adb.shell(['which', command]).decode('utf-8'):
          return command
      except adb.AdbError:
        continue

    self.log.warning(
        'No %s and %s commands available to launch instrument '
        'persistently, tests that depend on UiAutomator and '
        'at the same time perform USB disconnections may fail.',
        _SETSID_COMMAND,
        _NOHUP_COMMAND,
    )
    return ''

  def _get_instrument_options_str(self):
    self.log.debug(
        'Got am instrument options in snippet client for package %s: %s',
        self.package,
        self._config.am_instrument_options,
    )
    if not self._config.am_instrument_options:
      return ''

    return ' '.join(
        f'-e {k} {v}' for k, v in self._config.am_instrument_options.items()
    )

  def _get_user_command_string(self):
    """Gets the appropriate command argument for specifying device user ID.

    By default, this client operates within the current user. We
    don't add the `--user {ID}` argument when Android's SDK is below 24,
    where multi-user support is not well implemented.

    Returns:
      A string of the command argument section to be formatted into
      adb commands.
    """
    sdk_version = int(self._device.build_info['build_version_sdk'])
    if sdk_version < 24:
      return ''
    return f'--user {self.user_id}'

  def _read_protocol_line(self):
    """Reads the next line of instrumentation output relevant to snippets.

    This method will skip over lines that don't start with 'SNIPPET ' or
    'INSTRUMENTATION_RESULT:'.

    Returns:
      A string for the next line of snippet-related instrumentation output,
        stripped.

    Raises:
      errors.ServerStartError: If EOF is reached without any protocol lines
        being read.
    """
    self._server_start_stdout = []
    while True:
      line = self._proc.stdout.readline().decode('utf-8')
      if not line:
        raise errors.ServerStartError(
            self._device, 'Unexpected EOF when waiting for server to start.'
        )

      # readline() uses an empty string to mark EOF, and a single newline
      # to mark regular empty lines in the output. Don't move the strip()
      # call above the truthiness check, or this method will start
      # considering any blank output line to be EOF.
      line = line.strip()
      if line.startswith('INSTRUMENTATION_RESULT:') or line.startswith(
          'SNIPPET '
      ):
        self.log.debug('Accepted line from instrumentation output: "%s"', line)
        return line

      self._server_start_stdout.append(line)
      self.log.debug('Discarded line from instrumentation output: "%s"', line)

  def make_connection(self):
    """Makes a connection to the snippet server on the remote device.

    This function makes a persistent connection to the server. This connection
    will be used for all the RPCs, and must be closed when deconstructing.

    To connect to the Android device, it first forwards the device port to a
    host port. Then, it creates a socket connection to the server on the device.
    Finally, it sends a handshake request to the server, which requests the
    server to prepare for the communication with the client.

    This function uses self.host_port for communicating with the server. If
    self.host_port is 0 or None, this function finds an available host port to
    make the connection and set self.host_port to the found port.
    """
    self._forward_device_port()
    self.create_socket_connection()
    self.send_handshake_request()

  def _forward_device_port(self):
    """Forwards the device port to a host port."""
    if self.host_port and self.host_port in adb.list_occupied_adb_ports():
      raise errors.Error(
          self._device,
          f'Cannot forward to host port {self.host_port} because adb has'
          ' forwarded another device port to it.',
      )

    host_port = self.host_port or 0
    # Example stdout: b'12345\n'
    stdout = self._adb.forward([f'tcp:{host_port}', f'tcp:{self.device_port}'])
    self.host_port = int(stdout.strip())

  def create_socket_connection(self):
    """Creates a socket connection to the server.

    After creating the connection successfully, it sets two attributes:
    * `self._conn`: the created socket object, which will be used when it needs
      to close the connection.
    * `self._client`: the socket file, which will be used to send and receive
      messages.

    This function only creates a socket connection without sending any message
    to the server.
    """
    try:
      self.log.debug(
          'Snippet client is creating socket connection to the snippet server '
          'of %s through host port %d.',
          self.package,
          self.host_port,
      )
      self._conn = socket.create_connection(
          ('localhost', self.host_port), _SOCKET_CONNECTION_TIMEOUT
      )
    except ConnectionRefusedError as err:
      # Retry using '127.0.0.1' for IPv4 enabled machines that only resolve
      # 'localhost' to '[::1]'.
      self.log.debug(
          'Failed to connect to localhost, trying 127.0.0.1: %s', str(err)
      )
      try:
        self._conn = socket.create_connection(
            ('127.0.0.1', self.host_port), _SOCKET_CONNECTION_TIMEOUT
        )
      except ConnectionRefusedError as err2:
        ret, _, _ = utils.run_command(
            f'netstat -tulpn | grep ":{self.host_port}"', shell=True
        )
        if ret != 0:
          raise errors.Error(
              self._device,
              'The Adb forward command execution did not take effect. Please'
              ' check if there are other processes affecting adb forward on the'
              ' host.',
          ) from err2

        raise errors.Error(
            self._device,
            'Failed to establish socket connection from host to snippet server'
            ' running on Android device.',
        ) from err2

    self._conn.settimeout(_SOCKET_READ_TIMEOUT)
    self._client = self._conn.makefile(mode='brw')

  def send_handshake_request(
      self, uid=UNKNOWN_UID, cmd=ConnectionHandshakeCommand.INIT
  ):
    """Sends a handshake request to the server to prepare for the communication.

    Through the handshake response, this function checks whether the server
    is ready for the communication. If ready, it sets `self.uid` to the
    server session id. Otherwise, it sets `self.uid` to `UNKNOWN_UID`.

    Args:
      uid: int, the uid of the server session to continue. It will be ignored
        if the `cmd` requires the server to create a new session.
      cmd: ConnectionHandshakeCommand, the handshake command Enum for the
        server, which requires the server to create a new session or use the
        current session.

    Raises:
      errors.ProtocolError: something went wrong when sending the handshake
        request.
    """
    request = json.dumps({'cmd': cmd.value, 'uid': uid})
    self.log.debug('Sending handshake request %s.', request)
    self._client_send(request)
    response = self._client_receive()

    if not response:
      raise errors.ProtocolError(
          self._device, errors.ProtocolError.NO_RESPONSE_FROM_HANDSHAKE
      )

    response = self._decode_socket_response_bytes(response)

    result = json.loads(response)
    if result['status']:
      self.uid = result['uid']
    else:
      self.uid = UNKNOWN_UID

  def check_server_proc_running(self):
    """See base class.

    This client does nothing at this stage.
    """

  def send_rpc_request(self, request):
    """Sends an RPC request to the server and receives a response.

    Args:
      request: str, the request to send the server.

    Returns:
      The string of the RPC response.

    Raises:
      errors.Error: if failed to send the request or receive a response.
      errors.ProtocolError: if received an empty response from the server.
      UnicodeError: if failed to decode the received response.
    """
    self._client_send(request)
    response = self._client_receive()
    if not response:
      raise errors.ProtocolError(
          self._device, errors.ProtocolError.NO_RESPONSE_FROM_SERVER
      )
    return self._decode_socket_response_bytes(response)

  def _client_send(self, message):
    """Sends an RPC message through the connection.

    Args:
      message: str, the message to send.

    Raises:
      errors.Error: if a socket error occurred during the send.
    """
    try:
      self._client.write(f'{message}\n'.encode('utf8'))
      self._client.flush()
    except socket.error as e:
      raise errors.Error(
          self._device,
          f'Encountered socket error "{e}" sending RPC message "{message}"',
      ) from e

  def _client_receive(self):
    """Receives the server's response of an RPC message.

    Returns:
      Raw bytes of the response.

    Raises:
      errors.Error: if a socket error occurred during the read.
    """
    try:
      return self._client.readline()
    except socket.error as e:
      raise errors.Error(
          self._device, f'Encountered socket error "{e}" reading RPC response'
      ) from e

  def _decode_socket_response_bytes(self, response):
    """Returns a string decoded from the socket response bytes.

    Args:
      response: bytes, the response to be decoded.

    Returns:
      The string decoded from the given bytes.

    Raises:
      UnicodeError: if failed to decode the given bytes using encoding utf8.
    """
    try:
      return str(response, encoding='utf8')
    except UnicodeError:
      self.log.error(
          'Failed to decode socket response bytes using encoding utf8: %s',
          response,
      )
      raise

  def handle_callback(self, callback_id, ret_value, rpc_func_name):
    """Creates the callback handler object.

    If the client doesn't have an event client, it will start an event client
    before creating a callback handler.

    Args:
      callback_id: see base class.
      ret_value: see base class.
      rpc_func_name: see base class.

    Returns:
      The callback handler object.
    """
    if self._event_client is None:
      self._create_event_client()
    return callback_handler_v2.CallbackHandlerV2(
        callback_id=callback_id,
        event_client=self._event_client,
        ret_value=ret_value,
        method_name=rpc_func_name,
        device=self._device,
        rpc_max_timeout_sec=_SOCKET_READ_TIMEOUT,
        default_timeout_sec=_CALLBACK_DEFAULT_TIMEOUT_SEC,
    )

  def _create_event_client(self):
    """Creates a separate client to the same session for propagating events.

    As the server is already started by the snippet server on which this
    function is called, the created event client connects to the same session
    as the snippet server. It also reuses the same host port and device port.
    """
    self._event_client = SnippetClientV2(package=self.package, ad=self._device)
    self._event_client.make_connection_with_forwarded_port(
        self.host_port,
        self.device_port,
        self.uid,
        ConnectionHandshakeCommand.CONTINUE,
    )

  def make_connection_with_forwarded_port(
      self,
      host_port,
      device_port,
      uid=UNKNOWN_UID,
      cmd=ConnectionHandshakeCommand.INIT,
  ):
    """Makes a connection to the server with the given forwarded port.

    This process assumes that a device port has already been forwarded to a
    host port, and it only makes a connection to the snippet server based on
    the forwarded port. This is typically used by clients that share the same
    snippet server, e.g. the snippet client and its event client.

    Args:
      host_port: int, the host port which has already been forwarded.
      device_port: int, the device port listened by the snippet server.
      uid: int, the uid of the server session to continue. It will be ignored
        if the `cmd` requires the server to create a new session.
      cmd: ConnectionHandshakeCommand, the handshake command Enum for the
        server, which requires the server to create a new session or use the
        current session.
    """
    self.host_port = host_port
    self.device_port = device_port
    self._counter = self._id_counter()
    self.create_socket_connection()
    self.send_handshake_request(uid, cmd)

  def stop(self):
    """Releases all the resources acquired in `initialize`.

    This function releases following resources:
    * Close the socket connection.
    * Stop forwarding the device port to host.
    * Stop the standing server subprocess running on the host side.
    * Stop the snippet server running on the device side.
    * Stop the event client and set `self._event_client` to None.

    Raises:
      android_device_lib_errors.DeviceError: if the server exited with errors on
        the device side.
    """
    self.log.debug('Stopping snippet package %s.', self.package)
    self.close_connection()
    self._stop_server()
    self._destroy_event_client()
    self.log.debug('Snippet package %s stopped.', self.package)

  def close_connection(self):
    """Closes the connection to the snippet server on the device.

    This function closes the socket connection and stops forwarding the device
    port to host.
    """
    try:
      if self._conn:
        self._conn.close()
        self._conn = None
    finally:
      # Always clear the host port as part of the close step
      self._stop_port_forwarding()

  def _stop_port_forwarding(self):
    """Stops the adb port forwarding used by this client."""
    if self.host_port:
      self._device.adb.forward(['--remove', f'tcp:{self.host_port}'])
      self.host_port = None

  def _stop_server(self):
    """Releases all the resources acquired in `start_server`.

    Raises:
      android_device_lib_errors.DeviceError: if the server exited with errors on
        the device side.
    """
    # Although killing the snippet server would abort this subprocess anyway, we
    # want to call stop_standing_subprocess() to perform a health check,
    # print the failure stack trace if there was any, and reap it from the
    # process table. Note that it's much more important to ensure releasing all
    # the allocated resources on the host side than on the remote device side.

    # Stop the standing server subprocess running on the host side.
    if self._proc:
      utils.stop_standing_subprocess(self._proc)
      self._proc = None

    # Send the stop signal to the server running on the device side.
    out = self._adb.shell(
        _STOP_CMD.format(
            snippet_package=self.package, user=self._get_user_command_string()
        ),
        timeout=_STOP_CMD_TIMEOUT_SEC,
    ).decode('utf-8')

    if 'OK (0 tests)' not in out:
      raise android_device_lib_errors.DeviceError(
          self._device,
          f'Failed to stop existing apk. Unexpected output: {out}.',
      )

  def _destroy_event_client(self):
    """Releases all the resources acquired in `_create_event_client`."""
    if self._event_client:
      # Without cleaning host_port of event_client first, the close_connection
      # will try to stop the port forwarding, which should only be stopped by
      # the corresponding snippet client.
      self._event_client.host_port = None
      self._event_client.device_port = None
      self._event_client.close_connection()
      self._event_client = None

  def restore_server_connection(self, port=None):
    """Restores the server after the device got reconnected.

    Instead of creating a new instance of the client:
      - Uses the given port (or find a new available host port if none is
      given).
      - Tries to connect to the remote server with the selected port.

    Args:
      port: int, if given, this is the host port from which to connect to the
        remote device port. If not provided, find a new available port as host
        port.

    Raises:
      errors.ServerRestoreConnectionError: when failed to restore the connection
        to the snippet server.
    """
    try:
      # If self.host_port is None, self._make_connection finds a new available
      # port.
      self.host_port = port
      self._make_connection()
    except Exception as e:
      # Log the original error and raise ServerRestoreConnectionError.
      self.log.error('Failed to re-connect to the server.')
      raise errors.ServerRestoreConnectionError(
          self._device,
          (
              f'Failed to restore server connection for {self.package} at '
              f'host port {self.host_port}, device port {self.device_port}.'
          ),
      ) from e

    # Because the previous connection was lost, update self._proc
    self._proc = None
    self._restore_event_client()

  def _restore_event_client(self):
    """Restores the previously created event client or creates a new one.

    This function restores the connection of the previously created event
    client, or creates a new client and makes a connection if it didn't
    exist before.

    The event client to restore reuses the same host port and device port
    with the client on which function is called.
    """
    if self._event_client:
      self._event_client.make_connection_with_forwarded_port(
          self.host_port, self.device_port
      )

  def help(self, print_output=True):
    """Calls the help RPC, which returns the list of RPC calls available.

    This RPC should normally be used in an interactive console environment
    where the output should be printed instead of returned. Otherwise,
    newlines will be escaped, which will make the output difficult to read.

    Args:
      print_output: bool, for whether the output should be printed.

    Returns:
      A string containing the help output otherwise None if `print_output`
        wasn't set.
    """
    help_text = self._rpc('help')
    if print_output:
      print(help_text)
    else:
      return help_text
