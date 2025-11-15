# Copyright 2024 Google Inc.
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
from typing import Iterable, Optional

from mobly import utils
from mobly.controllers.android_device import AndroidDevice
from mobly.controllers.android_device_lib import adb
from mobly.controllers.android_device_lib import errors


DEFAULT_TIMEOUT_INSTALL_APK_SEC = 300
# Error messages from adb.
ADB_UNINSTALL_INTERNAL_ERROR_MSG = 'DELETE_FAILED_INTERNAL_ERROR'


def _execute_adb_install(
    device: AndroidDevice, install_args: Iterable[str], timeout: int
) -> None:
  """Executes the adb install command.

  Args:
    device: AndroidDevice, Mobly's Android controller object.
    install_args: list of strings, the args to be added to `adb install` cmd.
    timeout: int, the number of seconds to wait before timing out.

  Raises:
    AdbError: installation failed.
  """
  stderr_buffer = io.BytesIO()
  stdout = device.adb.install(
      install_args, stderr=stderr_buffer, timeout=timeout
  )
  stderr = stderr_buffer.getvalue().decode('utf-8').strip()
  if not _is_apk_install_success(stdout, stderr):
    adb_cmd = 'adb -s %s install %s' % (device.serial, ' '.join(install_args))
    raise adb.AdbError(cmd=adb_cmd, stdout=stdout, stderr=stderr, ret_code=0)


def _is_apk_install_success(stdout: bytes, stderr: str) -> bool:
  """Checks output of the adb install command and decides if install succeeded.

  Args:
    stdout: string, the standard out output of an adb install command.
    stderr: string, the standard error output of an adb install command.

  Returns:
    True if the installation succeeded; False otherwise.
  """
  if utils.grep('Failure', stdout):
    return False
  return any([not stderr, stderr == 'Success', 'waiting for device' in stderr])


def _should_retry_apk_install(error_msg: str) -> bool:
  """Decides whether we should retry adb install.

  Args:
    error_msg: string, the error message of an adb install failure.

  Returns:
    True if a retry is warranted; False otherwise.
  """
  return 'INSTALL_FAILED_INSUFFICIENT_STORAGE' in error_msg


def install(
    device: AndroidDevice,
    apk_path: str,
    timeout: int = DEFAULT_TIMEOUT_INSTALL_APK_SEC,
    user_id: Optional[int] = None,
    params: Optional[Iterable[str]] = None,
) -> None:
  """Install an apk on an Android device.

  Installing apk is more complicated than most people realize on Android.
  This is just a util for the most common use cases. If you need special logic
  beyond this, we recomend you write your own instead of modifying this.

  Args:
    device: AndroidDevice, Mobly's Android controller object.
    apk_path: string, file path of an apk file.
    timeout: int, the number of seconds to wait before timing out.
    user_id: int, the ID of the user to install the apk for. For SDK>=24,
        install for the current user by default. Android's multi-user support
        did not realistically work until SDK 24.
    params: string list, additional parameters included in the adb install cmd.

  Raises:
    AdbError: Installation failed.
    ValueError: Attempts to set user_id on SDK<24.
  """
  android_api_version = int(device.build_info['build_version_sdk'])
  if user_id is not None and android_api_version < 24:
    raise ValueError('Cannot specify `user_id` for device below SDK 24.')
  # Figure out the args to use.
  args = ['-r', '-t']
  if android_api_version >= 24:
    if user_id is None:
      user_id = device.adb.current_user_id
    args = ['--user', str(user_id)] + args
  if android_api_version >= 23:
    args.append('-g')
  if android_api_version >= 17:
    args.append('-d')
  args += params or []
  args.append(apk_path)
  try:
    _execute_adb_install(device, args, timeout)
    return
  except adb.AdbError as e:
    if not _should_retry_apk_install(str(e)):
      raise
    device.log.debug('Retrying installation of %s', apk_path)
    device.reboot()
    _execute_adb_install(device, args, timeout)


def is_apk_installed(device: AndroidDevice, package_name: str) -> bool:
  """Check if the given apk is already installed.

  Args:
    device: AndroidDevice, Mobly's Android controller object.
    package_name: str, name of the package.

  Returns:
    True if package is installed. False otherwise.
  """
  try:
    out = device.adb.shell(['pm', 'list', 'package'])
    return bool(utils.grep('^package:%s$' % package_name, out))
  except adb.AdbError as error:
    raise errors.DeviceError(device, error)


def uninstall(device: AndroidDevice, package_name: str) -> None:
  """Uninstall an apk on an Android device if it is installed.

  Works for regular app and OEM pre-installed non-system app.

  Args:
    device: AndroidDevice, Mobly's Android controller object.
    package_name: string, package name of the app.
  """
  if is_apk_installed(device, package_name):
    try:
      device.adb.uninstall([package_name])
    except adb.AdbError as e1:
      # This error can happen if the package to uninstall is non-system and
      # pre-loaded by OEM. Try removing it via PackageManager (pm) under UID 0.
      if ADB_UNINSTALL_INTERNAL_ERROR_MSG in str(e1):
        device.log.debug(
            'Encountered uninstall internal error, try pm remove with UID 0.'
        )
        try:
          device.adb.shell(
              ['pm', 'uninstall', '-k', '--user', '0', package_name]
          )
          return
        except adb.AdbError as e2:
          device.log.exception('Second attempt to uninstall failed: %s', e2)
      raise e1
