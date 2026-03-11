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

import contextlib
import enum
import functools
import logging
import os
import re
import shutil
import time

from mobly import logger as mobly_logger
from mobly import runtime_test_info
from mobly import utils
from mobly.controllers.android_device_lib import adb
from mobly.controllers.android_device_lib import errors
from mobly.controllers.android_device_lib import fastboot
from mobly.controllers.android_device_lib import service_manager
from mobly.controllers.android_device_lib.services import logcat
from mobly.controllers.android_device_lib.services import snippet_management_service

# Convenience constant for the package of Mobly Bundled Snippets
# (http://github.com/google/mobly-bundled-snippets).
MBS_PACKAGE = 'com.google.android.mobly.snippet.bundled'

MOBLY_CONTROLLER_CONFIG_NAME = 'AndroidDevice'

ANDROID_DEVICE_PICK_ALL_TOKEN = '*'
_DEBUG_PREFIX_TEMPLATE = '[AndroidDevice|%s] %s'

# Key name for adb logcat extra params in config file.
ANDROID_DEVICE_ADB_LOGCAT_PARAM_KEY = 'adb_logcat_param'
ANDROID_DEVICE_EMPTY_CONFIG_MSG = 'Configuration is empty, abort!'
ANDROID_DEVICE_NOT_LIST_CONFIG_MSG = 'Configuration should be a list, abort!'

# System properties that are cached by the `AndroidDevice.build_info` property.
# The only properties on this list should be read-only system properties.
CACHED_SYSTEM_PROPS = [
    'ro.build.id',
    'ro.build.type',
    'ro.build.fingerprint',
    'ro.build.version.codename',
    'ro.build.version.incremental',
    'ro.build.version.sdk',
    'ro.build.product',
    'ro.build.characteristics',
    'ro.debuggable',
    'ro.product.name',
    'ro.hardware',
]

# Keys for attributes in configs that alternate the controller module behavior.
# If this is False for a device, errors from that device will be ignored
# during `create`. Default is True.
KEY_DEVICE_REQUIRED = 'required'
DEFAULT_VALUE_DEVICE_REQUIRED = True
# If True, logcat collection will not be started during `create`.
# Default is False.
KEY_SKIP_LOGCAT = 'skip_logcat'
DEFAULT_VALUE_SKIP_LOGCAT = False
SERVICE_NAME_LOGCAT = 'logcat'

# Default name for bug reports taken without a specified test name.
DEFAULT_BUG_REPORT_NAME = 'bugreport'

# Default Timeout to wait for boot completion
DEFAULT_TIMEOUT_BOOT_COMPLETION_SECOND = 15 * 60

# Timeout for the adb command for taking a screenshot
TAKE_SCREENSHOT_TIMEOUT_SECOND = 10

# Aliases of error types for backward compatibility.
Error = errors.Error
DeviceError = errors.DeviceError
SnippetError = snippet_management_service.Error

# Regex to heuristically determine if the device is an emulator.
EMULATOR_SERIAL_REGEX = re.compile(r'emulator-\d+')


def create(configs):
  """Creates AndroidDevice controller objects.

  Args:
    configs: Represents configurations for Android devices, this can take one of
      the following forms:
      * str, only asterisk symbol is accepted, indicating that all connected
        Android devices will be used
      * A list of dict, each representing a configuration for an Android device.
      * A list of str, each representing the serial number of Android device.

  Returns:
    A list of AndroidDevice objects.
  """
  if not configs:
    raise Error(ANDROID_DEVICE_EMPTY_CONFIG_MSG)
  elif configs == ANDROID_DEVICE_PICK_ALL_TOKEN:
    ads = get_all_instances()
  elif not isinstance(configs, list):
    raise Error(ANDROID_DEVICE_NOT_LIST_CONFIG_MSG)
  elif isinstance(configs[0], dict):
    # Configs is a list of dicts.
    ads = get_instances_with_configs(configs)
  elif isinstance(configs[0], str):
    # Configs is a list of strings representing serials.
    ads = get_instances(configs)
  else:
    raise Error('No valid config found in: %s' % configs)
  _start_services_on_ads(ads)
  return ads


def destroy(ads):
  """Cleans up AndroidDevice objects.

  Args:
    ads: A list of AndroidDevice objects.
  """
  for ad in ads:
    try:
      ad.services.stop_all()
    except Exception:
      ad.log.exception('Failed to clean up properly.')


def get_info(ads):
  """Get information on a list of AndroidDevice objects.

  Args:
    ads: A list of AndroidDevice objects.

  Returns:
    A list of dict, each representing info for an AndroidDevice objects.
    Everything in this dict should be yaml serializable.
  """
  infos = []
  # The values of user_added_info can be arbitrary types, so we shall sanitize
  # them here to ensure they are yaml serializable.
  for ad in ads:
    device_info = ad.device_info
    user_added_info = {
        k: str(v) for (k, v) in device_info['user_added_info'].items()
    }
    device_info['user_added_info'] = user_added_info
    infos.append(device_info)
  return infos


def _validate_device_existence(serials):
  """Validate that all the devices specified by the configs can be reached.

  Args:
    serials: list of strings, the serials of all the devices that are expected
      to exist.
  """
  valid_ad_identifiers = (
      list_adb_devices()
      + list_adb_devices_by_usb_id()
      + list_fastboot_devices()
  )
  for serial in serials:
    if serial not in valid_ad_identifiers:
      raise Error(
          f'Android device serial "{serial}" is specified in '
          'config but is not reachable.'
      )


def _start_services_on_ads(ads):
  """Starts long running services on multiple AndroidDevice objects.

  If any one AndroidDevice object fails to start services, cleans up all
  AndroidDevice objects and their services.

  Args:
    ads: A list of AndroidDevice objects whose services to start.
  """
  for ad in ads:
    start_logcat = not getattr(ad, KEY_SKIP_LOGCAT, DEFAULT_VALUE_SKIP_LOGCAT)
    try:
      if start_logcat:
        ad.services.logcat.start()
    except Exception:
      is_required = getattr(
          ad, KEY_DEVICE_REQUIRED, DEFAULT_VALUE_DEVICE_REQUIRED
      )
      if is_required:
        ad.log.exception('Failed to start some services, abort!')
        destroy(ads)
        raise
      else:
        ad.log.exception(
            'Skipping this optional device because some '
            'services failed to start.'
        )


def parse_device_list(device_list_str, key=None):
  """Parses a byte string representing a list of devices.

  The string is generated by calling either adb or fastboot. The tokens in
  each string is tab-separated.

  Args:
    device_list_str: Output of adb or fastboot.
    key: The token that signifies a device in device_list_str. Only devices
      with the specified key in device_list_str are parsed, such as 'device' or
      'fastbootd'. If not specified, all devices listed are parsed.

  Returns:
    A list of android device serial numbers.
  """
  try:
    clean_lines = str(device_list_str, 'utf-8').strip().split('\n')
  except UnicodeDecodeError:
    logging.warning('unicode decode error, origin str: %s', device_list_str)
    raise
  results = []
  for line in clean_lines:
    tokens = line.strip().split('\t')
    if len(tokens) == 2 and (key is None or tokens[1] == key):
      results.append(tokens[0])
  return results


def list_adb_devices():
  """List all android devices connected to the computer that are detected by
  adb.

  Returns:
    A list of android device serials. Empty if there's none.
  """
  out = adb.AdbProxy().devices()
  return parse_device_list(out, 'device')


def list_adb_devices_by_usb_id():
  """List the usb id of all android devices connected to the computer that
  are detected by adb.

  Returns:
    A list of strings that are android device usb ids. Empty if there's
    none.
  """
  out = adb.AdbProxy().devices(['-l'])
  clean_lines = str(out, 'utf-8').strip().split('\n')
  results = []
  for line in clean_lines:
    tokens = line.strip().split()
    if len(tokens) > 2 and tokens[1] == 'device':
      results.append(tokens[2])
  return results


def list_fastboot_devices():
  """List all android devices connected to the computer that are in in
  fastboot mode. These are detected by fastboot.

  This function doesn't raise any error if `fastboot` binary doesn't exist,
  because `FastbootProxy` itself doesn't raise any error.

  Returns:
    A list of android device serials. Empty if there's none.
  """
  out = fastboot.FastbootProxy().devices()
  return parse_device_list(out)


def get_instances(serials):
  """Create AndroidDevice instances from a list of serials.

  Args:
    serials: A list of android device serials.

  Returns:
    A list of AndroidDevice objects.
  """
  _validate_device_existence(serials)

  results = []
  for s in serials:
    results.append(AndroidDevice(s))
  return results


def get_instances_with_configs(configs):
  """Create AndroidDevice instances from a list of dict configs.

  Each config should have the required key-value pair 'serial'.

  Args:
    configs: A list of dicts each representing the configuration of one
      android device.

  Returns:
    A list of AndroidDevice objects.
  """
  # First make sure each config contains a serial, and all the serials'
  # corresponding devices exist.
  serials = []
  for c in configs:
    try:
      serials.append(c['serial'])
    except KeyError:
      raise Error(
          'Required value "serial" is missing in AndroidDevice config %s.' % c
      )
  _validate_device_existence(serials)
  results = []
  for c in configs:
    serial = c.pop('serial')
    is_required = c.get(KEY_DEVICE_REQUIRED, True)
    try:
      ad = AndroidDevice(serial)
      ad.load_config(c)
    except Exception:
      if is_required:
        raise
      ad.log.exception('Skipping this optional device due to error.')
      continue
    results.append(ad)
  return results


def get_all_instances(include_fastboot=False):
  """Create AndroidDevice instances for all attached android devices.

  Args:
    include_fastboot: Whether to include devices in bootloader mode or not.

  Returns:
    A list of AndroidDevice objects each representing an android device
    attached to the computer.
  """
  if include_fastboot:
    serial_list = list_adb_devices() + list_fastboot_devices()
    return get_instances(serial_list)
  return get_instances(list_adb_devices())


def filter_devices(ads, func):
  """Finds the AndroidDevice instances from a list that match certain
  conditions.

  Args:
    ads: A list of AndroidDevice instances.
    func: A function that takes an AndroidDevice object and returns True
      if the device satisfies the filter condition.

  Returns:
    A list of AndroidDevice instances that satisfy the filter condition.
  """
  results = []
  for ad in ads:
    if func(ad):
      results.append(ad)
  return results


def get_devices(ads, **kwargs):
  """Finds a list of AndroidDevice instance from a list that has specific
  attributes of certain values.

  Example:
    get_devices(android_devices, label='foo', phone_number='1234567890')
    get_devices(android_devices, model='angler')

  Args:
    ads: A list of AndroidDevice instances.
    kwargs: keyword arguments used to filter AndroidDevice instances.

  Returns:
    A list of target AndroidDevice instances.

  Raises:
    Error: No devices are matched.
  """

  def _get_device_filter(ad):
    for k, v in kwargs.items():
      if not hasattr(ad, k):
        return False
      elif getattr(ad, k) != v:
        return False
    return True

  filtered = filter_devices(ads, _get_device_filter)
  if not filtered:
    raise Error(
        'Could not find a target device that matches condition: %s.' % kwargs
    )
  else:
    return filtered


def get_device(ads, **kwargs):
  """Finds a unique AndroidDevice instance from a list that has specific
  attributes of certain values.

  Example:
    get_device(android_devices, label='foo', phone_number='1234567890')
    get_device(android_devices, model='angler')

  Args:
    ads: A list of AndroidDevice instances.
    kwargs: keyword arguments used to filter AndroidDevice instances.

  Returns:
    The target AndroidDevice instance.

  Raises:
    Error: None or more than one device is matched.
  """

  filtered = get_devices(ads, **kwargs)
  if len(filtered) == 1:
    return filtered[0]
  else:
    serials = [ad.serial for ad in filtered]
    raise Error('More than one device matched: %s' % serials)


def take_bug_reports(ads, test_name=None, begin_time=None, destination=None):
  """Takes bug reports on a list of android devices.

  If you want to take a bug report, call this function with a list of
  android_device objects in on_fail. But reports will be taken on all the
  devices in the list concurrently. Bug report takes a relative long
  time to take, so use this cautiously.

  Args:
    ads: A list of AndroidDevice instances.
    test_name: Name of the test method that triggered this bug report.
      If None, the default name "bugreport" will be used.
    begin_time: timestamp taken when the test started, can be either
      string or int. If None, the current time will be used.
    destination: string, path to the directory where the bugreport
      should be saved.
  """
  if begin_time is None:
    begin_time = mobly_logger.get_log_file_timestamp()
  else:
    begin_time = mobly_logger.sanitize_filename(str(begin_time))

  def take_br(test_name, begin_time, ad, destination):
    ad.take_bug_report(
        test_name=test_name, begin_time=begin_time, destination=destination
    )

  args = [(test_name, begin_time, ad, destination) for ad in ads]
  utils.concurrent_exec(take_br, args)


class BuildInfoConstants(enum.Enum):
  """Enums for build info constants used for AndroidDevice build info.

  Attributes:
    build_info_key: The key used for the build_info dictionary in AndroidDevice.
    system_prop_key: The key used for getting the build info from system
      properties.
  """

  BUILD_ID = 'build_id', 'ro.build.id'
  BUILD_TYPE = 'build_type', 'ro.build.type'
  BUILD_FINGERPRINT = 'build_fingerprint', 'ro.build.fingerprint'
  BUILD_VERSION_CODENAME = 'build_version_codename', 'ro.build.version.codename'
  BUILD_VERSION_INCREMENTAL = (
      'build_version_incremental',
      'ro.build.version.incremental',
  )
  BUILD_VERSION_SDK = 'build_version_sdk', 'ro.build.version.sdk'
  BUILD_PRODUCT = 'build_product', 'ro.build.product'
  BUILD_CHARACTERISTICS = 'build_characteristics', 'ro.build.characteristics'
  DEBUGGABLE = 'debuggable', 'ro.debuggable'
  PRODUCT_NAME = 'product_name', 'ro.product.name'
  HARDWARE = 'hardware', 'ro.hardware'

  def __init__(self, build_info_key, system_prop_key):
    self.build_info_key = build_info_key
    self.system_prop_key = system_prop_key


class AndroidDevice:
  """Class representing an android device.

  Each object of this class represents one Android device in Mobly. This class
  provides various ways, like adb, fastboot, and Mobly snippets, to control
  an Android device, whether it's a real device or an emulator instance.

  You can also register your own services to the device's service manager.
  See the docs of `service_manager` and `base_service` for details.

  Attributes:
    serial: A string that's the serial number of the Android device.
    log_path: A string that is the path where all logs collected on this
      android device should be stored.
    log: A logger adapted from root logger with an added prefix specific
      to an AndroidDevice instance. The default prefix is
      [AndroidDevice|<serial>]. Use self.debug_tag = 'tag' to use a
      different tag in the prefix.
    adb_logcat_file_path: A string that's the full path to the adb logcat
      file collected, if any.
    adb: An AdbProxy object used for interacting with the device via adb.
    fastboot: A FastbootProxy object used for interacting with the device
      via fastboot.
    services: ServiceManager, the manager of long-running services on the
      device.
  """

  def __init__(self, serial=''):
    self._serial = str(serial)
    # logging.log_path only exists when this is used in an Mobly test run.
    _log_path_base = utils.abs_path(getattr(logging, 'log_path', '/tmp/logs'))
    self._log_path = os.path.join(
        _log_path_base, 'AndroidDevice%s' % self._normalized_serial
    )
    self._debug_tag = self._serial
    self.log = AndroidDeviceLoggerAdapter(
        logging.getLogger(), {'tag': self.debug_tag}
    )
    self._build_info = None
    self._is_rebooting = False
    self.adb = adb.AdbProxy(serial)
    self.fastboot = fastboot.FastbootProxy(serial)
    if self.is_rootable:
      self.root_adb()
    self.services = service_manager.ServiceManager(self)
    self.services.register(
        SERVICE_NAME_LOGCAT, logcat.Logcat, start_service=False
    )
    self.services.register(
        'snippets', snippet_management_service.SnippetManagementService
    )
    # Device info cache.
    self._user_added_device_info = {}

  def __repr__(self):
    return '<AndroidDevice|%s>' % self.debug_tag

  @property
  def adb_logcat_file_path(self):
    if self.services.has_service_by_name(SERVICE_NAME_LOGCAT):
      return self.services.logcat.adb_logcat_file_path

  @property
  def _normalized_serial(self):
    """Normalized serial name for usage in log filename.

    Some Android emulators use ip:port as their serial names, while on
    Windows `:` is not valid in filename, it should be sanitized first.
    """
    if self._serial is None:
      return None
    return mobly_logger.sanitize_filename(self._serial)

  @property
  def device_info(self):
    """Information to be pulled into controller info.

    The latest serial, model, and build_info are included. Additional info
    can be added via `add_device_info`.
    """
    info = {
        'serial': self.serial,
        'model': self.model,
        'build_info': self.build_info,
        'user_added_info': self._user_added_device_info,
    }
    return info

  def add_device_info(self, name, info):
    """Add information of the device to be pulled into controller info.

    Adding the same info name the second time will override existing info.

    Args:
      name: string, name of this info.
      info: serializable, content of the info.
    """
    self._user_added_device_info.update({name: info})

  @property
  def debug_tag(self):
    """A string that represents a device object in debug info. Default value
    is the device serial.

    This will be used as part of the prefix of debugging messages emitted by
    this device object, like log lines and the message of DeviceError.
    """
    return self._debug_tag

  @debug_tag.setter
  def debug_tag(self, tag):
    """Setter for the debug tag.

    By default, the tag is the serial of the device, but sometimes it may
    be more descriptive to use a different tag of the user's choice.

    Changing debug tag changes part of the prefix of debug info emitted by
    this object, like log lines and the message of DeviceError.

    Example:
      By default, the device's serial number is used:
        'INFO [AndroidDevice|abcdefg12345] One pending call ringing.'
      The tag can be customized with `ad.debug_tag = 'Caller'`:
        'INFO [AndroidDevice|Caller] One pending call ringing.'
    """
    self.log.info('Logging debug tag set to "%s"', tag)
    self._debug_tag = tag
    self.log.extra['tag'] = tag

  @property
  def has_active_service(self):
    """True if any service is running on the device.

    A service can be a snippet or logcat collection.
    """
    return self.services.is_any_alive

  @property
  def log_path(self):
    """A string that is the path for all logs collected from this device."""
    if not os.path.exists(self._log_path):
      utils.create_dir(self._log_path)
    return self._log_path

  @log_path.setter
  def log_path(self, new_path):
    """Setter for `log_path`, use with caution."""
    if self.has_active_service:
      raise DeviceError(
          self, 'Cannot change `log_path` when there is service running.'
      )
    old_path = self._log_path
    if new_path == old_path:
      return
    if os.listdir(new_path):
      raise DeviceError(
          self, 'Logs already exist at %s, cannot override.' % new_path
      )
    if os.path.exists(old_path):
      # Remove new path so copytree doesn't complain.
      shutil.rmtree(new_path, ignore_errors=True)
      shutil.copytree(old_path, new_path)
      shutil.rmtree(old_path, ignore_errors=True)
    self._log_path = new_path

  @property
  def serial(self):
    """The serial number used to identify a device.

    This is essentially the value used for adb's `-s` arg, which means it
    can be a network address or USB bus number.
    """
    return self._serial

  def update_serial(self, new_serial):
    """Updates the serial number of a device.

    The "serial number" used with adb's `-s` arg is not necessarily the
    actual serial number. For remote devices, it could be a combination of
    host names and port numbers.

    This is used for when such identifier of remote devices changes during
    a test. For example, when a remote device reboots, it may come back
    with a different serial number.

    This is NOT meant for switching the object to represent another device.

    We intentionally did not make it a regular setter of the serial
    property so people don't accidentally call this without understanding
    the consequences.

    Args:
      new_serial: string, the new serial number for the same device.

    Raises:
      DeviceError: tries to update serial when any service is running.
    """
    new_serial = str(new_serial)
    if self.has_active_service:
      raise DeviceError(
          self,
          'Cannot change device serial number when there is service running.',
      )
    if self._debug_tag == self.serial:
      self._debug_tag = new_serial
    self._serial = new_serial
    self.adb.serial = new_serial
    self.fastboot.serial = new_serial

  @contextlib.contextmanager
  def handle_reboot(self):
    """Properly manage the service life cycle when the device needs to
    temporarily disconnect.

    The device can temporarily lose adb connection due to user-triggered
    reboot. Use this function to make sure the services
    started by Mobly are properly stopped and restored afterwards.

    For sample usage, see self.reboot().
    """
    live_service_names = self.services.list_live_services()
    self.services.stop_all()
    # On rooted devices, system properties may change on reboot, so disable
    # the `build_info` cache by setting `_is_rebooting` to True and
    # repopulate it after reboot.
    # Note, this logic assumes that instance variable assignment in Python
    # is atomic; otherwise, `threading` data structures would be necessary.
    # Additionally, nesting calls to `handle_reboot` while changing the
    # read-only property values during reboot will result in stale values.
    self._is_rebooting = True
    try:
      yield
    finally:
      self.wait_for_boot_completion()
      # On boot completion, invalidate the `build_info` cache since any
      # value it had from before boot completion is potentially invalid.
      # If the value gets set after the final invalidation and before
      # setting`_is_rebooting` to True, then that's okay because the
      # device has finished rebooting at that point, and values at that
      # point should be valid.
      # If the reboot fails for some reason, then `_is_rebooting` is never
      # set to False, which means the `build_info` cache remains disabled
      # until the next reboot. This is relatively okay because the
      # `build_info` cache is only minimizes adb commands.
      self._build_info = None
      self._is_rebooting = False
      if self.is_rootable:
        self.root_adb()
    self.services.start_services(live_service_names)

  @contextlib.contextmanager
  def handle_usb_disconnect(self):
    """Properly manage the service life cycle when USB is disconnected.

    The device can temporarily lose adb connection due to user-triggered
    USB disconnection, e.g. the following cases can be handled by this
    method:

    * Power measurement: Using Monsoon device to measure battery consumption
      would potentially disconnect USB.
    * Unplug USB so device loses connection.
    * ADB connection over WiFi and WiFi got disconnected.
    * Any other type of USB disconnection, as long as snippet session can
      be kept alive while USB disconnected (reboot caused USB
      disconnection is not one of these cases because snippet session
      cannot survive reboot.
      Use handle_reboot() instead).

    Use this function to make sure the services started by Mobly are
    properly reconnected afterwards.

    Just like the usage of self.handle_reboot(), this method does not
    automatically detect if the disconnection is because of a reboot or USB
    disconnect. Users of this function should make sure the right handle_*
    function is used to handle the correct type of disconnection.

    This method also reconnects snippet event client. Therefore, the
    callback objects created (by calling Async RPC methods) before
    disconnection would still be valid and can be used to retrieve RPC
    execution result after device got reconnected.

    Example Usage:

    .. code-block:: python

      with ad.handle_usb_disconnect():
        try:
          # User action that triggers USB disconnect, could throw
          # exceptions.
          do_something()
        finally:
          # User action that triggers USB reconnect
          action_that_reconnects_usb()
          # Make sure device is reconnected before returning from this
          # context
          ad.adb.wait_for_device(timeout=SOME_TIMEOUT)
    """
    live_service_names = self.services.list_live_services()
    self.services.pause_all()
    try:
      yield
    finally:
      self.services.resume_services(live_service_names)

  @property
  def build_info(self):
    """Gets the build info of this Android device, including build id and type.

    This is not available if the device is in bootloader mode.

    Returns:
      A dict with the build info of this Android device, or None if the
      device is in bootloader mode.
    """
    if self.is_bootloader:
      self.log.error('Device is in fastboot mode, could not get build info.')
      return
    if self._build_info is None or self._is_rebooting:
      info = {}
      build_info = self.adb.getprops(CACHED_SYSTEM_PROPS)
      for build_info_constant in BuildInfoConstants:
        info[build_info_constant.build_info_key] = build_info.get(
            build_info_constant.system_prop_key, ''
        )
      self._build_info = info
      return info
    return self._build_info

  @property
  def is_bootloader(self):
    """True if the device is in bootloader mode."""
    return self.serial in list_fastboot_devices()

  @property
  def is_adb_root(self):
    """True if adb is running as root for this device."""
    try:
      return '0' == self.adb.shell('id -u').decode('utf-8').strip()
    except adb.AdbError:
      # Wait a bit and retry to work around adb flakiness for this cmd.
      time.sleep(0.2)
      return '0' == self.adb.shell('id -u').decode('utf-8').strip()

  @property
  def is_rootable(self):
    return self.is_adb_detectable() and self.build_info['debuggable'] == '1'

  @functools.cached_property
  def model(self):
    """The Android code name for the device."""
    # If device is in bootloader mode, get mode name from fastboot.
    if self.is_bootloader:
      out = self.fastboot.getvar('product').strip()
      # 'out' is never empty because of the 'total time' message fastboot
      # writes to stderr.
      lines = out.decode('utf-8').split('\n', 1)
      if lines:
        tokens = lines[0].split(' ')
        if len(tokens) > 1:
          return tokens[1].lower()
      return None
    model = self.build_info['build_product'].lower()
    if model == 'sprout':
      return model
    return self.build_info['product_name'].lower()

  @property
  def is_emulator(self):
    """Whether this device is probably an emulator.

    Returns:
      True if this is probably an emulator.
    """
    if EMULATOR_SERIAL_REGEX.match(self.serial):
      # If the device's serial follows 'emulator-dddd', then it's almost
      # certainly an emulator.
      return True
    elif self.build_info['build_characteristics'] == 'emulator':
      # If the device says that it's an emulator, then it's probably an
      # emulator although some real devices apparently report themselves
      # as emulators in addition to other things, so only return True on
      # an exact match.
      return True
    elif self.build_info['hardware'] in ['ranchu', 'goldfish', 'cutf_cvm']:
      # Ranchu and Goldfish are the hardware properties that the AOSP
      # emulators report, so if the device says it's an AOSP emulator, it
      # probably is one. Cuttlefish emulators report 'cutf_cvm` as the
      # hardware property.
      return True
    else:
      return False

  def load_config(self, config):
    """Add attributes to the AndroidDevice object based on config.

    Args:
      config: A dictionary representing the configs.

    Raises:
      Error: The config is trying to overwrite an existing attribute.
    """
    for k, v in config.items():
      if hasattr(self, k) and k not in _ANDROID_DEVICE_SETTABLE_PROPS:
        raise DeviceError(
            self,
            'Attribute %s already exists with value %s, cannot set again.'
            % (k, getattr(self, k)),
        )
      setattr(self, k, v)
      self.add_device_info(k, v)

  def root_adb(self):
    """Change adb to root mode for this device if allowed.

    If executed on a production build, adb will not be switched to root
    mode per security restrictions.
    """
    self.adb.root()
    # `root` causes the device to temporarily disappear from adb.
    # So we need to wait for the device to come back before proceeding.
    self.adb.wait_for_device(timeout=DEFAULT_TIMEOUT_BOOT_COMPLETION_SECOND)

  def load_snippet(self, name, package, config=None):
    """Starts the snippet apk with the given package name and connects.

    Examples:

    .. code-block:: python

      ad.load_snippet(
          name='maps', package='com.google.maps.snippets')
      ad.maps.activateZoom('3')

    Args:
      name: string, the attribute name to which to attach the snippet
        client. E.g. `name='maps'` attaches the snippet client to
        `ad.maps`.
      package: string, the package name of the snippet apk to connect to.
      config: snippet_client_v2.Config, the configuration object for
        controlling the snippet behaviors. See the docstring of the `Config`
        class for supported configurations.

    Raises:
      SnippetError: Illegal load operations are attempted.
    """
    # Should not load snippet with an existing attribute.
    if hasattr(self, name):
      raise SnippetError(
          self,
          'Attribute "%s" already exists, please use a different name.' % name,
      )
    self.services.snippets.add_snippet_client(name, package, config=config)

  def unload_snippet(self, name):
    """Stops a snippet apk.

    Args:
      name: The attribute name the snippet server is attached with.

    Raises:
      SnippetError: The given snippet name is not registered.
    """
    self.services.snippets.remove_snippet_client(name)

  def generate_filename(
      self, file_type, time_identifier=None, extension_name=None
  ):
    """Generates a name for an output file related to this device.

    The name follows the pattern:

      {file type},{debug_tag},{serial},{model},{time identifier}.{ext}

    "debug_tag" is only added if it's different from the serial. "ext" is
    added if specified by user.

    Args:
      file_type: string, type of this file, like "logcat" etc.
      time_identifier: string or RuntimeTestInfo. If a `RuntimeTestInfo`
        is passed in, the `signature` of the test case will be used. If
        a string is passed in, the string itself will be used.
        Otherwise the current timestamp will be used.
      extension_name: string, the extension name of the file.

    Returns:
      String, the filename generated.
    """
    time_str = time_identifier
    if time_identifier is None:
      time_str = mobly_logger.get_log_file_timestamp()
    elif isinstance(time_identifier, runtime_test_info.RuntimeTestInfo):
      time_str = time_identifier.signature
    filename_tokens = [file_type]
    if self.debug_tag != self.serial:
      filename_tokens.append(self.debug_tag)
    filename_tokens.extend([self.serial, self.model, time_str])
    filename_str = ','.join(filename_tokens)
    if extension_name is not None:
      filename_str = '%s.%s' % (filename_str, extension_name)
    filename_str = mobly_logger.sanitize_filename(filename_str)
    self.log.debug('Generated filename: %s', filename_str)
    return filename_str

  def take_bug_report(
      self, test_name=None, begin_time=None, timeout=300, destination=None
  ):
    """Takes a bug report on the device and stores it in a file.

    Args:
      test_name: Name of the test method that triggered this bug report.
      begin_time: Timestamp of when the test started. If not set, then
        this will default to the current time.
      timeout: float, the number of seconds to wait for bugreport to
        complete, default is 5min.
      destination: string, path to the directory where the bugreport
        should be saved.

    Returns:
      A string that is the absolute path to the bug report on the host.
    """
    prefix = DEFAULT_BUG_REPORT_NAME
    if test_name:
      prefix = '%s,%s' % (DEFAULT_BUG_REPORT_NAME, test_name)
    if begin_time is None:
      begin_time = mobly_logger.get_log_file_timestamp()

    new_br = True
    try:
      stdout = self.adb.shell('bugreportz -v').decode('utf-8')
      # This check is necessary for builds before N, where adb shell's ret
      # code and stderr are not propagated properly.
      if 'not found' in stdout:
        new_br = False
    except adb.AdbError:
      new_br = False

    if destination is None:
      destination = os.path.join(self.log_path, 'BugReports')
    br_path = utils.abs_path(destination)
    utils.create_dir(br_path)
    filename = self.generate_filename(prefix, str(begin_time), 'txt')
    if new_br:
      filename = filename.replace('.txt', '.zip')
    full_out_path = os.path.join(br_path, filename)
    # in case device restarted, wait for adb interface to return
    self.wait_for_boot_completion()
    self.log.debug('Start taking bugreport.')
    if new_br:
      out = self.adb.shell('bugreportz', timeout=timeout).decode('utf-8')
      if not out.startswith('OK'):
        raise DeviceError(self, 'Failed to take bugreport: %s' % out)
      br_out_path = out.split(':')[1].strip()
      self.adb.pull([br_out_path, full_out_path])
      self.adb.shell(['rm', br_out_path])
    else:
      # shell=True as this command redirects the stdout to a local file
      # using shell redirection.
      self.adb.bugreport(' > "%s"' % full_out_path, shell=True, timeout=timeout)
    self.log.debug('Bugreport taken at %s.', full_out_path)
    return full_out_path

  def take_screenshot(self, destination, prefix='screenshot'):
    """Takes a screenshot of the device.

    Args:
      destination: string, full path to the directory to save in.
      prefix: string, prefix file name of the screenshot.

    Returns:
      string, full path to the screenshot file on the host.
    """
    filename = self.generate_filename(prefix, extension_name='png')
    device_path = os.path.join('/storage/emulated/0/', filename)
    self.adb.shell(
        ['screencap', '-p', device_path], timeout=TAKE_SCREENSHOT_TIMEOUT_SECOND
    )
    utils.create_dir(destination)
    self.adb.pull([device_path, destination])
    pic_path = os.path.join(destination, filename)
    self.log.debug('Screenshot taken, saved on the host: %s', pic_path)
    self.adb.shell(['rm', device_path])
    return pic_path

  def run_iperf_client(self, server_host, extra_args=''):
    """Start iperf client on the device.

    Return status as true if iperf client start successfully.
    And data flow information as results.

    Args:
      server_host: Address of the iperf server.
      extra_args: A string representing extra arguments for iperf client,
        e.g. '-i 1 -t 30'.

    Returns:
      status: true if iperf client start successfully.
      results: results have data flow information
    """
    out = self.adb.shell('iperf3 -c %s %s' % (server_host, extra_args))
    clean_out = str(out, 'utf-8').strip().split('\n')
    if 'error' in clean_out[0].lower():
      return False, clean_out
    return True, clean_out

  def wait_for_boot_completion(
      self, timeout=DEFAULT_TIMEOUT_BOOT_COMPLETION_SECOND
  ):
    """Waits for Android framework to broadcast ACTION_BOOT_COMPLETED.

    This function times out after 15 minutes.

    Args:
      timeout: float, the number of seconds to wait before timing out.
        If not specified, no timeout takes effect.
    """
    deadline = time.perf_counter() + timeout

    self.adb.wait_for_device(timeout=timeout)
    while time.perf_counter() < deadline:
      try:
        if self.is_boot_completed():
          return
      except (adb.AdbError, adb.AdbTimeoutError):
        # adb shell calls may fail during certain period of booting
        # process, which is normal. Ignoring these errors.
        pass
      time.sleep(5)
    raise DeviceError(self, 'Booting process timed out')

  def is_boot_completed(self):
    """Checks if device boot is completed by verifying system property."""
    completed = self.adb.getprop('sys.boot_completed')
    if completed == '1':
      self.log.debug('Device boot completed.')
      return True
    return False

  def is_adb_detectable(self):
    """Checks if USB is on and device is ready by verifying adb devices."""
    serials = list_adb_devices()
    if self.serial in serials:
      self.log.debug('Is now adb detectable.')
      return True
    return False

  def reboot(self):
    """Reboots the device.

    Generally one should use this method to reboot the device instead of
    directly calling `adb.reboot`. Because this method gracefully handles
    the teardown and restoration of running services.

    This method is blocking and only returns when the reboot has completed
    and the services restored.

    Raises:
      Error: Waiting for completion timed out.
    """
    if self.is_bootloader:
      self.fastboot.reboot()
      return
    with self.handle_reboot():
      self.adb.reboot()

  def __getattr__(self, name):
    """Tries to return a snippet client registered with `name`.

    This is for backward compatibility of direct accessing snippet clients.
    """
    client = self.services.snippets.get_snippet_client(name)
    if client:
      return client
    return self.__getattribute__(name)


# Properties in AndroidDevice that have setters.
# This line has to live below the AndroidDevice code.
_ANDROID_DEVICE_SETTABLE_PROPS = utils.get_settable_properties(AndroidDevice)


class AndroidDeviceLoggerAdapter(logging.LoggerAdapter):
  """A wrapper class that adds a prefix to each log line.

  Usage:

  .. code-block:: python

    my_log = AndroidDeviceLoggerAdapter(logging.getLogger(), {
      'tag': <custom tag>
    })

  Then each log line added by my_log will have a prefix
  '[AndroidDevice|<tag>]'
  """

  def process(self, msg, kwargs):
    msg = _DEBUG_PREFIX_TEMPLATE % (self.extra['tag'], msg)
    return (msg, kwargs)
