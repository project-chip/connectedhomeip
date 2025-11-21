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
"""Module for Mobly controller management."""
import collections
import copy
import logging
import yaml

from mobly import expects
from mobly import records
from mobly import signals


def verify_controller_module(module):
  """Verifies a module object follows the required interface for
  controllers.

  The interface is explained in the docstring of
  `base_test.BaseTestClass.register_controller`.

  Args:
    module: An object that is a controller module. This is usually
      imported with import statements or loaded by importlib.

  Raises:
    ControllerError: if the module does not match the Mobly controller
      interface, or one of the required members is null.
  """
  required_attributes = ('create', 'destroy', 'MOBLY_CONTROLLER_CONFIG_NAME')
  for attr in required_attributes:
    if not hasattr(module, attr):
      raise signals.ControllerError(
          'Module %s missing required controller module attribute %s.'
          % (module.__name__, attr)
      )
    if not getattr(module, attr):
      raise signals.ControllerError(
          'Controller interface %s in %s cannot be null.'
          % (attr, module.__name__)
      )


class ControllerManager:
  """Manages the controller objects for Mobly tests.

  This manages the life cycles and info retrieval of all controller objects
  used in a test.

  Attributes:
    controller_configs: dict, controller configs provided by the user via
      test bed config.
  """

  def __init__(self, class_name, controller_configs):
    # Controller object management.
    self._controller_objects = (
        collections.OrderedDict()
    )  # controller_name: objects
    self._controller_modules = {}  # controller_name: module
    self._class_name = class_name
    self.controller_configs = controller_configs

  def register_controller(self, module, required=True, min_number=1):
    """Loads a controller module and returns its loaded devices.

    This is to be used in a mobly test class.

    Args:
      module: A module that follows the controller module interface.
      required: A bool. If True, failing to register the specified
        controller module raises exceptions. If False, the objects
        failed to instantiate will be skipped.
      min_number: An integer that is the minimum number of controller
        objects to be created. Default is one, since you should not
        register a controller module without expecting at least one
        object.

    Returns:
      A list of controller objects instantiated from controller_module, or
      None if no config existed for this controller and it was not a
      required controller.

    Raises:
      ControllerError:
        * The controller module has already been registered.
        * The actual number of objects instantiated is less than the
        * `min_number`.
        * `required` is True and no corresponding config can be found.
        * Any other error occurred in the registration process.
    """
    verify_controller_module(module)
    # Use the module's name as the ref name
    module_ref_name = module.__name__.split('.')[-1]
    if module_ref_name in self._controller_objects:
      raise signals.ControllerError(
          'Controller module %s has already been registered. It cannot '
          'be registered again.' % module_ref_name
      )
    # Create controller objects.
    module_config_name = module.MOBLY_CONTROLLER_CONFIG_NAME
    if module_config_name not in self.controller_configs:
      if required:
        raise signals.ControllerError(
            'No corresponding config found for %s' % module_config_name
        )
      logging.warning(
          'No corresponding config found for optional controller %s',
          module_config_name,
      )
      return None
    try:
      # Make a deep copy of the config to pass to the controller module,
      # in case the controller module modifies the config internally.
      original_config = self.controller_configs[module_config_name]
      controller_config = copy.deepcopy(original_config)
      objects = module.create(controller_config)
    except Exception:
      logging.exception(
          'Failed to initialize objects for controller %s, abort!',
          module_config_name,
      )
      raise
    if not isinstance(objects, list):
      raise signals.ControllerError(
          'Controller module %s did not return a list of objects, abort.'
          % module_ref_name
      )
    # Check we got enough controller objects to continue.
    actual_number = len(objects)
    if actual_number < min_number:
      module.destroy(objects)
      raise signals.ControllerError(
          'Expected to get at least %d controller objects, got %d.'
          % (min_number, actual_number)
      )
    # Save a shallow copy of the list for internal usage, so tests can't
    # affect internal registry by manipulating the object list.
    self._controller_objects[module_ref_name] = copy.copy(objects)
    logging.debug(
        'Found %d objects for controller %s', len(objects), module_config_name
    )
    self._controller_modules[module_ref_name] = module
    return objects

  def unregister_controllers(self):
    """Destroy controller objects and clear internal registry.

    This will be called after each test class.
    """
    # TODO(xpconanfan): actually record these errors instead of just
    # logging them.
    for name, module in self._controller_modules.items():
      logging.debug('Destroying %s.', name)
      with expects.expect_no_raises('Exception occurred destroying %s.' % name):
        module.destroy(self._controller_objects[name])
    self._controller_objects = collections.OrderedDict()
    self._controller_modules = {}

  def _create_controller_info_record(self, controller_module_name):
    """Creates controller info record for a particular controller type.

    Info is retrieved from all the controller objects spawned from the
    specified module, using the controller module's `get_info` function.

    Args:
      controller_module_name: string, the name of the controller module
        to retrieve info from.

    Returns:
      A records.ControllerInfoRecord object.
    """
    module = self._controller_modules[controller_module_name]
    controller_info = None
    try:
      controller_info = module.get_info(
          copy.copy(self._controller_objects[controller_module_name])
      )
    except AttributeError:
      logging.warning(
          'No optional debug info found for controller '
          '%s. To provide it, implement `get_info`.',
          controller_module_name,
      )
    try:
      yaml.dump(controller_info)
    except TypeError:
      logging.warning(
          'The info of controller %s in class "%s" is not '
          'YAML serializable! Coercing it to string.',
          controller_module_name,
          self._class_name,
      )
      controller_info = str(controller_info)
    return records.ControllerInfoRecord(
        self._class_name, module.MOBLY_CONTROLLER_CONFIG_NAME, controller_info
    )

  def get_controller_info_records(self):
    """Get the info records for all the controller objects in the manager.

    New info records for each controller object are created for every call
    so the latest info is included.

    Returns:
      List of records.ControllerInfoRecord objects. Each opject conatins
      the info of a type of controller
    """
    info_records = []
    for controller_module_name in self._controller_objects.keys():
      with expects.expect_no_raises(
          'Failed to collect controller info from %s' % controller_module_name
      ):
        record = self._create_controller_info_record(controller_module_name)
        if record:
          info_records.append(record)
    return info_records
