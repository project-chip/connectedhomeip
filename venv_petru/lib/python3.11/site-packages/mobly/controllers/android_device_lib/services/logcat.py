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
import io
import logging
import os
import time

from mobly import logger as mobly_logger
from mobly import utils
from mobly.controllers.android_device_lib import adb
from mobly.controllers.android_device_lib import errors
from mobly.controllers.android_device_lib.services import base_service

CREATE_LOGCAT_FILE_TIMEOUT_SEC = 5


class Error(errors.ServiceError):
  """Root error type for logcat service."""

  SERVICE_TYPE = 'Logcat'


class Config:
  """Config object for logcat service.

  Attributes:
    clear_log: bool, clears the logcat before collection if True.
    logcat_params: string, extra params to be added to logcat command.
    output_file_path: string, the path on the host to write the log file
      to, including the actual filename. The service will automatically
      generate one if not specified.
  """

  def __init__(self, logcat_params=None, clear_log=True, output_file_path=None):
    self.clear_log = clear_log
    self.logcat_params = logcat_params if logcat_params else ''
    self.output_file_path = output_file_path


class Logcat(base_service.BaseService):
  """Android logcat service for Mobly's AndroidDevice controller.

  Attributes:
    adb_logcat_file_path: string, path to the file that the service writes
      adb logcat to by default.
  """

  OUTPUT_FILE_TYPE = 'logcat'

  def __init__(self, android_device, configs=None):
    super().__init__(android_device, configs)
    self._ad = android_device
    self._adb_logcat_process = None
    self._adb_logcat_file_obj = None
    self.adb_logcat_file_path = None
    # Logcat service uses a single config obj, using singular internal
    # name: `_config`.
    self._config = configs if configs else Config()

  def _enable_logpersist(self):
    """Attempts to enable logpersist daemon to persist logs."""
    # Logpersist is only allowed on rootable devices because of excessive
    # reads/writes for persisting logs.
    if not self._ad.is_rootable:
      return

    logpersist_warning = (
        '%s encountered an error enabling persistent'
        ' logs, logs may not get saved.'
    )
    # Android L and older versions do not have logpersist installed,
    # so check that the logpersist scripts exists before trying to use
    # them.
    if not self._ad.adb.has_shell_command('logpersist.start'):
      logging.warning(logpersist_warning, self)
      return

    try:
      # Disable adb log spam filter for rootable devices. Have to stop
      # and clear settings first because 'start' doesn't support --clear
      # option before Android N.
      self._ad.adb.shell('logpersist.stop --clear')
      self._ad.adb.shell('logpersist.start')
    except adb.AdbError:
      logging.warning(logpersist_warning, self)

  def _is_timestamp_in_range(self, target, begin_time, end_time):
    low = mobly_logger.logline_timestamp_comparator(begin_time, target) <= 0
    high = mobly_logger.logline_timestamp_comparator(end_time, target) >= 0
    return low and high

  def create_output_excerpts(self, test_info):
    """Convenient method for creating excerpts of adb logcat.

    This copies logcat lines from self.adb_logcat_file_path to an excerpt
    file, starting from the location where the previous excerpt ended.

    Call this method at the end of: `setup_class`, `teardown_test`, and
    `teardown_class`.

    Args:
      test_info: `self.current_test_info` in a Mobly test.

    Returns:
      List of strings, the absolute paths to excerpt files.
    """
    dest_path = test_info.output_path
    utils.create_dir(dest_path)
    filename = self._ad.generate_filename(
        self.OUTPUT_FILE_TYPE, test_info, 'txt'
    )
    excerpt_file_path = os.path.join(dest_path, filename)
    with io.open(
        excerpt_file_path, 'w', encoding='utf-8', errors='replace'
    ) as out:
      # Devices may accidentally go offline during test,
      # check not None before readline().
      while self._adb_logcat_file_obj:
        line = self._adb_logcat_file_obj.readline()
        if not line:
          break
        out.write(line)
    self._ad.log.debug('logcat excerpt created at: %s', excerpt_file_path)
    return [excerpt_file_path]

  @property
  def is_alive(self):
    return True if self._adb_logcat_process else False

  def clear_adb_log(self):
    """Clears cached adb content."""
    try:
      self._ad.adb.logcat('-c')
    except adb.AdbError as e:
      # On Android O, the clear command fails due to a known bug.
      # Catching this so we don't crash from this Android issue.
      if b'failed to clear' in e.stderr:
        self._ad.log.warning('Encountered known Android error to clear logcat.')
      else:
        raise

  def _assert_not_running(self):
    """Asserts the logcat service is not running.

    Raises:
      Error, if the logcat service is running.
    """
    if self.is_alive:
      raise Error(
          self._ad,
          'Logcat thread is already running, cannot start another one.',
      )

  def update_config(self, new_config):
    """Updates the configuration for the service.

    The service needs to be stopped before updating, and explicitly started
    after the update.

    This will reset the service. Previous output files may be orphaned if
    output path is changed.

    Args:
      new_config: Config, the new config to use.
    """
    self._assert_not_running()
    self._ad.log.info(
        '[LogcatService] Changing config from %s to %s',
        self._config,
        new_config,
    )
    self._config = new_config

  def _open_logcat_file(self):
    """Create a file object that points to the beginning of the logcat file.
    Wait for the logcat file to be created by the subprocess if it doesn't
    exist.
    """
    if not self._adb_logcat_file_obj:
      deadline = time.perf_counter() + CREATE_LOGCAT_FILE_TIMEOUT_SEC
      while not os.path.exists(self.adb_logcat_file_path):
        if time.perf_counter() > deadline:
          raise Error(
              self._ad, 'Timeout while waiting for logcat file to be created.'
          )
        time.sleep(1)
      self._adb_logcat_file_obj = io.open(
          self.adb_logcat_file_path, 'r', encoding='utf-8', errors='replace'
      )
      self._adb_logcat_file_obj.seek(0, os.SEEK_END)

  def _close_logcat_file(self):
    """Closes and resets the logcat file object, if it exists."""
    if self._adb_logcat_file_obj:
      self._adb_logcat_file_obj.close()
      self._adb_logcat_file_obj = None

  def start(self):
    """Starts a standing adb logcat collection.

    The collection runs in a separate subprocess and saves logs in a file.
    """
    if self._ad.is_bootloader:
      self._ad.log.warning(
          'Skip starting logcat because the device is in fastboot mode.'
      )
      return
    self._assert_not_running()
    if self._config.clear_log:
      self.clear_adb_log()
    self._start()
    self._open_logcat_file()

  def _start(self):
    """The actual logic of starting logcat."""
    self._enable_logpersist()
    if self._config.output_file_path:
      self._close_logcat_file()
      self.adb_logcat_file_path = self._config.output_file_path
    if not self.adb_logcat_file_path:
      f_name = self._ad.generate_filename(
          self.OUTPUT_FILE_TYPE, extension_name='txt'
      )
      logcat_file_path = os.path.join(self._ad.log_path, f_name)
      self.adb_logcat_file_path = logcat_file_path
    utils.create_dir(os.path.dirname(self.adb_logcat_file_path))
    # In debugging mode of IntelijIDEA, "patch_args" remove
    # double quotes in args if starting and ending with it.
    # Add spaces at beginning and at last to fix this issue.
    cmd = ' "%s" -s %s logcat -v threadtime -T 1 %s >> "%s" ' % (
        adb.ADB,
        self._ad.serial,
        self._config.logcat_params,
        self.adb_logcat_file_path,
    )
    process = utils.start_standing_subprocess(cmd, shell=True)
    self._adb_logcat_process = process

  def stop(self):
    """Stops the adb logcat service."""
    self._close_logcat_file()
    self._stop()

  def _stop(self):
    """Stops the background process for logcat."""
    if not self._adb_logcat_process:
      return
    try:
      utils.stop_standing_subprocess(self._adb_logcat_process)
    except Exception:
      self._ad.log.exception('Failed to stop adb logcat.')
    self._adb_logcat_process = None

  def pause(self):
    """Pauses logcat.

    Note: the service is unable to collect the logs when paused, if more
    logs are generated on the device than the device's log buffer can hold,
    some logs would be lost.
    """
    self._stop()

  def resume(self):
    """Resumes a paused logcat service."""
    self._assert_not_running()
    # Not clearing the log regardless of the config when resuming.
    # Otherwise the logs during the paused time will be lost.
    self._start()
