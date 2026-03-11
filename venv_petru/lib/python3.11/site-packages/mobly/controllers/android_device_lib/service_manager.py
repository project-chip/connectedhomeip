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
"""Module for the manager of services."""
# TODO(xpconanfan: move the device errors to a more generic location so
# other device controllers like iOS can share it.
import collections
import inspect

from mobly import expects
from mobly.controllers.android_device_lib import errors
from mobly.controllers.android_device_lib.services import base_service


class Error(errors.DeviceError):
  """Root error type for this module."""


class ServiceManager:
  """Manager for services of AndroidDevice.

  A service is a long running process that involves an Android device, like
  adb logcat or Snippet.
  """

  def __init__(self, device):
    self._service_objects = collections.OrderedDict()
    self._device = device

  def has_service_by_name(self, name):
    """Checks if the manager has a service registered with a specific name.

    Args:
      name: string, the name to look for.

    Returns:
      True if a service is registered with the specified name, False
      otherwise.
    """
    return name in self._service_objects

  @property
  def is_any_alive(self):
    """True if any service is alive; False otherwise."""
    for service in self._service_objects.values():
      if service.is_alive:
        return True
    return False

  def register(self, alias, service_class, configs=None, start_service=True):
    """Registers a service.

    This will create a service instance, starts the service, and adds the
    instance to the mananger.

    Args:
      alias: string, the alias for this instance.
      service_class: class, the service class to instantiate.
      configs: (optional) config object to pass to the service class's
        constructor.
      start_service: bool, whether to start the service instance or not.
        Default is True.
    """
    if not inspect.isclass(service_class):
      raise Error(self._device, '"%s" is not a class!' % service_class)
    if not issubclass(service_class, base_service.BaseService):
      raise Error(
          self._device,
          'Class %s is not a subclass of BaseService!' % service_class,
      )
    if alias in self._service_objects:
      raise Error(
          self._device,
          'A service is already registered with alias "%s".' % alias,
      )
    service_obj = service_class(self._device, configs)
    service_obj.alias = alias
    if start_service:
      service_obj.start()
    self._service_objects[alias] = service_obj

  def unregister(self, alias):
    """Unregisters a service instance.

    Stops a service and removes it from the manager.

    Args:
      alias: string, the alias of the service instance to unregister.
    """
    if alias not in self._service_objects:
      raise Error(
          self._device, 'No service is registered with alias "%s".' % alias
      )
    service_obj = self._service_objects.pop(alias)
    if service_obj.is_alive:
      with expects.expect_no_raises(
          'Failed to stop service instance "%s".' % alias
      ):
        service_obj.stop()

  def for_each(self, func):
    """Executes a function with all registered services.

    Args:
      func: function, the function to execute. This function should take
        a service object as args.
    """
    aliases = list(self._service_objects.keys())
    for alias in aliases:
      with expects.expect_no_raises(
          'Failed to execute "%s" for service "%s".' % (func.__name__, alias)
      ):
        func(self._service_objects[alias])

  def get_service_alias_by_class(self, service_class):
    """Gets the aslias name of a registered service.

    The same service class can be registered multiple times with different
    aliases. When not well managed, duplication and race conditions can arise.
    One can use this API to de-duplicate as needed.

    Args:
      service_class: class, the class of a service type.

    Returns:
      list of strings, the aliases the service is registered with.
    """
    aliases = []
    for alias, service_object in self._service_objects.items():
      if isinstance(service_object, service_class):
        aliases.append(alias)
    return aliases

  def list_live_services(self):
    """Lists the aliases of all the services that are alive.

    Order of this list is determined by the order the services are
    registered in.

    Returns:
      list of strings, the aliases of the services that are running.
    """
    aliases = []
    self.for_each(
        lambda service: aliases.append(service.alias)
        if service.is_alive
        else None
    )
    return aliases

  def create_output_excerpts_all(self, test_info):
    """Creates output excerpts from all services.

    This calls `create_output_excerpts` on all registered services.

    Args:
      test_info: RuntimeTestInfo, the test info associated with the scope
        of the excerpts.

    Returns:
      Dict, keys are the names of the services, values are the paths to
        the excerpt files created by the corresponding services.
    """
    excerpt_paths = {}

    def create_output_excerpts_for_one(service):
      if not service.is_alive:
        return
      paths = service.create_output_excerpts(test_info)
      excerpt_paths[service.alias] = paths

    self.for_each(create_output_excerpts_for_one)
    return excerpt_paths

  def unregister_all(self):
    """Safely unregisters all active instances.

    Errors occurred here will be recorded but not raised.
    """
    aliases = list(self._service_objects.keys())
    for alias in aliases:
      self.unregister(alias)

  def start_all(self):
    """Starts all inactive service instances.

    Services will be started in the order they were registered.
    """
    for alias, service in self._service_objects.items():
      if not service.is_alive:
        with expects.expect_no_raises('Failed to start service "%s".' % alias):
          service.start()

  def start_services(self, service_alises):
    """Starts the specified services.

    Services will be started in the order specified by the input list.
    No-op for services that are already running.

    Args:
      service_alises: list of strings, the aliases of services to start.
    """
    for name in service_alises:
      if name not in self._service_objects:
        raise Error(
            self._device,
            'No service is registered under the name "%s", cannot start.'
            % name,
        )
      service = self._service_objects[name]
      if not service.is_alive:
        service.start()

  def stop_all(self):
    """Stops all active service instances.

    Services will be stopped in the reverse order they were registered.
    """
    # OrdereDict#items does not return a sequence in Python 3.4, so we have
    # to do a list conversion here.
    for alias, service in reversed(list(self._service_objects.items())):
      if service.is_alive:
        with expects.expect_no_raises('Failed to stop service "%s".' % alias):
          service.stop()

  def pause_all(self):
    """Pauses all service instances.

    Services will be paused in the reverse order they were registered.
    """
    # OrdereDict#items does not return a sequence in Python 3.4, so we have
    # to do a list conversion here.
    for alias, service in reversed(list(self._service_objects.items())):
      with expects.expect_no_raises('Failed to pause service "%s".' % alias):
        service.pause()

  def resume_all(self):
    """Resumes all service instances.

    Services will be resumed in the order they were registered.
    """
    for alias, service in self._service_objects.items():
      with expects.expect_no_raises('Failed to resume service "%s".' % alias):
        service.resume()

  def resume_services(self, service_alises):
    """Resumes the specified services.

    Services will be resumed in the order specified by the input list.

    Args:
      service_alises: list of strings, the names of services to start.
    """
    for name in service_alises:
      if name not in self._service_objects:
        raise Error(
            self._device,
            'No service is registered under the name "%s", cannot resume.'
            % name,
        )
      service = self._service_objects[name]
      service.resume()

  def __getattr__(self, name):
    """Syntactic sugar to enable direct access of service objects by alias.

    Args:
      name: string, the alias a service object was registered under.
    """
    if self.has_service_by_name(name):
      return self._service_objects[name]
    return self.__getattribute__(name)
