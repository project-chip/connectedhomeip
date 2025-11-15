# Copyright 2018 Google Inc.
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
"""Module for the snippet management service."""
from mobly.controllers.android_device_lib import errors
from mobly.controllers.android_device_lib import snippet_client_v2
from mobly.controllers.android_device_lib.services import base_service

MISSING_SNIPPET_CLIENT_MSG = 'No snippet client is registered with name "%s".'


class Error(errors.ServiceError):
  """Root error type for snippet management service."""

  SERVICE_TYPE = 'SnippetManagementService'


class SnippetManagementService(base_service.BaseService):
  """Management service of snippet clients.

  This service manages all the snippet clients associated with an Android
  device.
  """

  def __init__(self, device, configs=None):
    del configs  # Unused param.
    self._device = device
    self._is_alive = False
    self._snippet_clients = {}
    super().__init__(device)

  @property
  def is_alive(self):
    """True if any client is running, False otherwise."""
    return any([client.is_alive for client in self._snippet_clients.values()])

  def get_snippet_client(self, name):
    """Gets the snippet client managed under a given name.

    Args:
      name: string, the name of the snippet client under management.

    Returns:
      SnippetClient.
    """
    if name in self._snippet_clients:
      return self._snippet_clients[name]

  def add_snippet_client(self, name, package, config=None):
    """Adds a snippet client to the management.

    Args:
      name: string, the attribute name to which to attach the snippet
        client. E.g. `name='maps'` attaches the snippet client to
        `ad.maps`.
      package: string, the package name of the snippet apk to connect to.
      config: snippet_client_v2.Config, the configuration object for
        controlling the snippet behaviors. See the docstring of the `Config`
        class for supported configurations.

    Raises:
      Error, if a duplicated name or package is passed in.
    """
    # Should not load snippet with the same name more than once.
    if name in self._snippet_clients:
      raise Error(
          self,
          'Name "%s" is already registered with package "%s", it cannot '
          'be used again.' % (name, self._snippet_clients[name].client.package),
      )
    # Should not load the same snippet package more than once.
    for snippet_name, client in self._snippet_clients.items():
      if package == client.package:
        raise Error(
            self,
            'Snippet package "%s" has already been loaded under name "%s".'
            % (package, snippet_name),
        )

    client = snippet_client_v2.SnippetClientV2(
        package=package,
        ad=self._device,
        config=config,
    )
    client.initialize()
    self._snippet_clients[name] = client

  def remove_snippet_client(self, name):
    """Removes a snippet client from management.

    Args:
      name: string, the name of the snippet client to remove.

    Raises:
      Error: if no snippet client is managed under the specified name.
    """
    if name not in self._snippet_clients:
      raise Error(self._device, MISSING_SNIPPET_CLIENT_MSG % name)
    client = self._snippet_clients.pop(name)
    client.stop()

  def start(self):
    """Starts all the snippet clients under management."""
    for client in self._snippet_clients.values():
      if not client.is_alive:
        self._device.log.debug('Starting SnippetClient<%s>.', client.package)
        client.initialize()
      else:
        self._device.log.debug(
            'Not startng SnippetClient<%s> because it is already alive.',
            client.package,
        )

  def stop(self):
    """Stops all the snippet clients under management."""
    for client in self._snippet_clients.values():
      if client.is_alive:
        self._device.log.debug('Stopping SnippetClient<%s>.', client.package)
        client.stop()
      else:
        self._device.log.debug(
            'Not stopping SnippetClient<%s> because it is not alive.',
            client.package,
        )

  def pause(self):
    """Pauses all the snippet clients under management.

    This clears the host port of a client because a new port will be
    allocated in `resume`.
    """
    for client in self._snippet_clients.values():
      self._device.log.debug('Pausing SnippetClient<%s>.', client.package)
      client.close_connection()

  def resume(self):
    """Resumes all paused snippet clients."""
    for client in self._snippet_clients.values():
      if not client.is_alive:
        self._device.log.debug('Resuming SnippetClient<%s>.', client.package)
        client.restore_server_connection()
      else:
        self._device.log.debug(
            'Not resuming SnippetClient<%s>.', client.package
        )

  def __getattr__(self, name):
    client = self.get_snippet_client(name)
    if client:
      return client
    return self.__getattribute__(name)
