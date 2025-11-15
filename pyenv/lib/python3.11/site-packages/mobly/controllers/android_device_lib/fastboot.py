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

import logging
from subprocess import PIPE

from mobly import utils

# The default fastboot command timeout settings.
DEFAULT_TIMEOUT_SEC = 180

# Command to use for running fastboot commands.
FASTBOOT = 'fastboot'


def exe_cmd(*cmds, timeout=DEFAULT_TIMEOUT_SEC):
  """Executes commands in a new shell. Directing stderr to PIPE, with timeout.

  This is fastboot's own exe_cmd because of its peculiar way of writing
  non-error info to stderr.

  Args:
    cmds: A sequence of commands and arguments.
    timeout: The number of seconds to wait before timing out.

  Returns:
    The output of the command run, in bytes.

  Raises:
    Exception: An error occurred during the command execution or
      the command timed out.
  """
  cmd = ' '.join(cmds)
  (ret, out, err) = utils.run_command(
      cmd=cmd,
      stdout=PIPE,
      stderr=PIPE,
      shell=True,
      timeout=timeout,
  )
  logging.debug(
      'cmd: %s, stdout: %s, stderr: %s, ret: %s',
      utils.cli_cmd_to_string(cmds),
      out,
      err,
      ret,
  )
  if not err:
    return out
  return err


class FastbootProxy:
  """Proxy class for fastboot.

  For syntactic reasons, the '-' in fastboot commands need to be replaced
  with '_'. Can directly execute fastboot commands on an object:
  >> fb = FastbootProxy(<serial>)
  >> fb.devices() # will return the console output of "fastboot devices".
  """

  def __init__(self, serial=''):
    self.serial = serial

  def fastboot_str(self):
    if self.serial:
      return '{} -s {}'.format(FASTBOOT, self.serial)
    return FASTBOOT

  def _exec_fastboot_cmd(self, name, arg_str, timeout=DEFAULT_TIMEOUT_SEC):
    return exe_cmd(
        ' '.join((self.fastboot_str(), name, arg_str)), timeout=timeout
    )

  def args(self, *args, timeout=DEFAULT_TIMEOUT_SEC):
    return exe_cmd(' '.join((self.fastboot_str(),) + args), timeout=timeout)

  def __getattr__(self, name):
    def fastboot_call(*args, timeout=DEFAULT_TIMEOUT_SEC):
      clean_name = name.replace('_', '-')
      arg_str = ' '.join(str(elem) for elem in args)
      return self._exec_fastboot_cmd(clean_name, arg_str, timeout=timeout)

    return fastboot_call
