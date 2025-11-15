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
"""JSON RPC interface to Mobly Snippet Lib."""

import logging
import re
import time

from mobly import utils
from mobly.controllers.android_device_lib import adb
from mobly.controllers.android_device_lib import errors
from mobly.controllers.android_device_lib import jsonrpc_client_base
from mobly.snippet import errors as snippet_errors

logging.warning(
    'The module mobly.controllers.android_device_lib.snippet_client'
    ' is deprecated and will be removed in a future version. Use'
    ' module mobly.controllers.android_device_lib.snippet_client_v2'
    ' instead.'
)

_INSTRUMENTATION_RUNNER_PACKAGE = (
    'com.google.android.mobly.snippet.SnippetRunner'
)

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

_LAUNCH_CMD = (
    '{shell_cmd} am instrument {user} -w -e action start {snippet_package}/'
    + _INSTRUMENTATION_RUNNER_PACKAGE
)

_STOP_CMD = (
    'am instrument {user} -w -e action stop {snippet_package}/'
    + _INSTRUMENTATION_RUNNER_PACKAGE
)

# Test that uses UiAutomation requires the shell session to be maintained while
# test is in progress. However, this requirement does not hold for the test that
# deals with device USB disconnection (Once device disconnects, the shell
# session that started the instrument ends, and UiAutomation fails with error:
# "UiAutomation not connected"). To keep the shell session and redirect
# stdin/stdout/stderr, use "setsid" or "nohup" while launching the
# instrumentation test. Because these commands may not be available in every
# android system, try to use them only if exists.
_SETSID_COMMAND = 'setsid'

_NOHUP_COMMAND = 'nohup'

# Aliases of error types for backward compatibility.
AppStartPreCheckError = snippet_errors.ServerStartPreCheckError
ProtocolVersionError = snippet_errors.ServerStartProtocolError


class SnippetClient(jsonrpc_client_base.JsonRpcClientBase):
  """A client for interacting with snippet APKs using Mobly Snippet Lib.

  DEPRECATED: Use
  mobly.controllers.android_device_lib.snippet_client_v2.SnippetClientV2
  instead.

  See superclass documentation for a list of public attributes.

  For a description of the launch protocols, see the documentation in
  mobly-snippet-lib, SnippetRunner.java.
  """

  def __init__(self, package, ad):
    """Initializes a SnippetClient.

    Args:
      package: (str) The package name of the apk where the snippets are
        defined.
      ad: (AndroidDevice) the device object associated with this client.
    """
    super().__init__(app_name=package, ad=ad)
    self.package = package
    self._ad = ad
    self._adb = ad.adb
    self._proc = None
    self._user_id = None

  @property
  def is_alive(self):
    """Does the client have an active connection to the snippet server."""
    return self._conn is not None

  @property
  def user_id(self):
    """The user id to use for this snippet client.

    This value is cached and, once set, does not change through the lifecycles
    of this snippet client object. This caching also reduces the number of adb
    calls needed.

    Because all the operations of the snippet client should be done for a
    partucular user.
    """
    if self._user_id is None:
      self._user_id = self._adb.current_user_id
    return self._user_id

  def _get_user_command_string(self):
    """Gets the appropriate command argument for specifying user IDs.

    By default, `SnippetClient` operates within the current user.

    We don't add the `--user {ID}` arg when Android's SDK is below 24,
    where multi-user support is not well implemented.

    Returns:
      String, the command param section to be formatted into the adb
      commands.
    """
    sdk_int = int(self._ad.build_info['build_version_sdk'])
    if sdk_int < 24:
      return ''
    return f'--user {self.user_id}'

  def start_app_and_connect(self):
    """Starts snippet apk on the device and connects to it.

    This wraps the main logic with safe handling

    Raises:
      AppStartPreCheckError, when pre-launch checks fail.
    """
    try:
      self._start_app_and_connect()
    except AppStartPreCheckError:
      # Precheck errors don't need cleanup, directly raise.
      raise
    except Exception as e:
      # Log the stacktrace of `e` as re-raising doesn't preserve trace.
      self._ad.log.exception('Failed to start app and connect.')
      # If errors happen, make sure we clean up before raising.
      try:
        self.stop_app()
      except Exception:
        self._ad.log.exception(
            'Failed to stop app after failure to start and connect.'
        )
      # Explicitly raise the original error from starting app.
      raise e

  def _start_app_and_connect(self):
    """Starts snippet apk on the device and connects to it.

    After prechecks, this launches the snippet apk with an adb cmd in a
    standing subprocess, checks the cmd response from the apk for protocol
    version, then sets up the socket connection over adb port-forwarding.

    Args:
      ProtocolVersionError, if protocol info or port info cannot be
        retrieved from the snippet apk.
    """
    self._check_app_installed()
    self.disable_hidden_api_blacklist()

    persists_shell_cmd = self._get_persist_command()
    # Use info here so people can follow along with the snippet startup
    # process. Starting snippets can be slow, especially if there are
    # multiple, and this avoids the perception that the framework is hanging
    # for a long time doing nothing.
    self.log.info(
        'Launching snippet apk %s with protocol %d.%d',
        self.package,
        _PROTOCOL_MAJOR_VERSION,
        _PROTOCOL_MINOR_VERSION,
    )
    cmd = _LAUNCH_CMD.format(
        shell_cmd=persists_shell_cmd,
        user=self._get_user_command_string(),
        snippet_package=self.package,
    )
    start_time = time.perf_counter()
    self._proc = self._do_start_app(cmd)

    # Check protocol version and get the device port
    line = self._read_protocol_line()
    match = re.match('^SNIPPET START, PROTOCOL ([0-9]+) ([0-9]+)$', line)
    if not match or match.group(1) != '1':
      raise ProtocolVersionError(self._ad, line)

    line = self._read_protocol_line()
    match = re.match('^SNIPPET SERVING, PORT ([0-9]+)$', line)
    if not match:
      raise ProtocolVersionError(self._ad, line)
    self.device_port = int(match.group(1))

    # Forward the device port to a new host port, and connect to that port
    self.host_port = utils.get_available_host_port()
    self._adb.forward(['tcp:%d' % self.host_port, 'tcp:%d' % self.device_port])
    self.connect()

    # Yaaay! We're done!
    self.log.debug(
        'Snippet %s started after %.1fs on host port %s',
        self.package,
        time.perf_counter() - start_time,
        self.host_port,
    )

  def restore_app_connection(self, port=None):
    """Restores the app after device got reconnected.

    Instead of creating new instance of the client:
      - Uses the given port (or find a new available host_port if none is
      given).
      - Tries to connect to remote server with selected port.

    Args:
      port: If given, this is the host port from which to connect to remote
        device port. If not provided, find a new available port as host
        port.

    Raises:
      AppRestoreConnectionError: When the app was not able to be started.
    """
    self.host_port = port or utils.get_available_host_port()
    self._adb.forward(['tcp:%d' % self.host_port, 'tcp:%d' % self.device_port])
    try:
      self.connect()
    except Exception:
      # Log the original error and raise AppRestoreConnectionError.
      self.log.exception('Failed to re-connect to app.')
      raise jsonrpc_client_base.AppRestoreConnectionError(
          self._ad,
          (
              'Failed to restore app connection for %s at host port %s, '
              'device port %s'
          )
          % (self.package, self.host_port, self.device_port),
      )

    # Because the previous connection was lost, update self._proc
    self._proc = None
    self._restore_event_client()

  def stop_app(self):
    # Kill the pending 'adb shell am instrument -w' process if there is one.
    # Although killing the snippet apk would abort this process anyway, we
    # want to call stop_standing_subprocess() to perform a health check,
    # print the failure stack trace if there was any, and reap it from the
    # process table.
    self.log.debug('Stopping snippet apk %s', self.package)
    # Close the socket connection.
    self.disconnect()
    if self._proc:
      utils.stop_standing_subprocess(self._proc)
      self._proc = None
    out = self._adb.shell(
        _STOP_CMD.format(
            snippet_package=self.package, user=self._get_user_command_string()
        )
    ).decode('utf-8')
    if 'OK (0 tests)' not in out:
      raise errors.DeviceError(
          self._ad, 'Failed to stop existing apk. Unexpected output: %s' % out
      )

    self._stop_event_client()

  def _start_event_client(self):
    """Overrides superclass."""
    event_client = SnippetClient(package=self.package, ad=self._ad)
    event_client.host_port = self.host_port
    event_client.device_port = self.device_port
    event_client.connect(self.uid, jsonrpc_client_base.JsonRpcCommand.CONTINUE)
    return event_client

  def _stop_event_client(self):
    """Releases all the resources acquired in `_start_event_client`."""
    if self._event_client:
      self._event_client.close_socket_connection()
      # Without cleaning host_port of event_client, the event client will try to
      # stop the port forwarding when deconstructed, which should only be
      # stopped by the corresponding snippet client.
      self._event_client.host_port = None
      self._event_client.device_port = None
      self._event_client = None

  def _restore_event_client(self):
    """Restores previously created event client."""
    if not self._event_client:
      self._event_client = self._start_event_client()
      return
    self._event_client.host_port = self.host_port
    self._event_client.device_port = self.device_port
    self._event_client.connect()

  def _check_app_installed(self):
    # Check that the Mobly Snippet app is installed for the current user.
    out = self._adb.shell(f'pm list package --user {self.user_id}')
    if not utils.grep('^package:%s$' % self.package, out):
      raise AppStartPreCheckError(
          self._ad, f'{self.package} is not installed for user {self.user_id}.'
      )
    # Check that the app is instrumented.
    out = self._adb.shell('pm list instrumentation')
    matched_out = utils.grep(
        f'^instrumentation:{self.package}/{_INSTRUMENTATION_RUNNER_PACKAGE}',
        out,
    )
    if not matched_out:
      raise AppStartPreCheckError(
          self._ad, f'{self.package} is installed, but it is not instrumented.'
      )
    match = re.search(
        r'^instrumentation:(.*)\/(.*) \(target=(.*)\)$', matched_out[0]
    )
    target_name = match.group(3)
    # Check that the instrumentation target is installed if it's not the
    # same as the snippet package.
    if target_name != self.package:
      out = self._adb.shell(f'pm list package --user {self.user_id}')
      if not utils.grep('^package:%s$' % target_name, out):
        raise AppStartPreCheckError(
            self._ad,
            f'Instrumentation target {target_name} is not installed for user '
            f'{self.user_id}.',
        )

  def _do_start_app(self, launch_cmd):
    adb_cmd = [adb.ADB]
    if self._adb.serial:
      adb_cmd += ['-s', self._adb.serial]
    adb_cmd += ['shell', launch_cmd]
    return utils.start_standing_subprocess(adb_cmd, shell=False)

  def _read_protocol_line(self):
    """Reads the next line of instrumentation output relevant to snippets.

    This method will skip over lines that don't start with 'SNIPPET' or
    'INSTRUMENTATION_RESULT'.

    Returns:
      (str) Next line of snippet-related instrumentation output, stripped.

    Raises:
      jsonrpc_client_base.AppStartError: If EOF is reached without any
        protocol lines being read.
    """
    while True:
      line = self._proc.stdout.readline().decode('utf-8')
      if not line:
        raise jsonrpc_client_base.AppStartError(
            self._ad, 'Unexpected EOF waiting for app to start'
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
      self.log.debug('Discarded line from instrumentation output: "%s"', line)

  def _get_persist_command(self):
    """Check availability and return path of command if available."""
    for command in [_SETSID_COMMAND, _NOHUP_COMMAND]:
      try:
        if command in self._adb.shell(['which', command]).decode('utf-8'):
          return command
      except adb.AdbError:
        continue
    self.log.warning(
        'No %s and %s commands available to launch instrument '
        'persistently, tests that depend on UiAutomator and '
        'at the same time performs USB disconnection may fail',
        _SETSID_COMMAND,
        _NOHUP_COMMAND,
    )
    return ''

  def help(self, print_output=True):
    """Calls the help RPC, which returns the list of RPC calls available.

    This RPC should normally be used in an interactive console environment
    where the output should be printed instead of returned. Otherwise,
    newlines will be escaped, which will make the output difficult to read.

    Args:
      print_output: A bool for whether the output should be printed.

    Returns:
      A str containing the help output otherwise None if print_output
        wasn't set.
    """
    help_text = self._rpc('help')
    if print_output:
      print(help_text)
    else:
      return help_text
