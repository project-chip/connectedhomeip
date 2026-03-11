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

import base64
import concurrent.futures
import datetime
import errno
import inspect
import io
import logging
import os
import platform
import random
import re
import shlex
import signal
import string
import subprocess
import threading
import time
import traceback
from typing import Literal, Tuple, overload

import portpicker

# File name length is limited to 255 chars on some OS, so we need to make sure
# the file names we output fits within the limit.
MAX_FILENAME_LEN = 255
# Number of times to retry to get available port
MAX_PORT_ALLOCATION_RETRY = 50

ascii_letters_and_digits = string.ascii_letters + string.digits
valid_filename_chars = f'-_.{ascii_letters_and_digits}'

GMT_to_olson = {
    'GMT-9': 'America/Anchorage',
    'GMT-8': 'US/Pacific',
    'GMT-7': 'US/Mountain',
    'GMT-6': 'US/Central',
    'GMT-5': 'US/Eastern',
    'GMT-4': 'America/Barbados',
    'GMT-3': 'America/Buenos_Aires',
    'GMT-2': 'Atlantic/South_Georgia',
    'GMT-1': 'Atlantic/Azores',
    'GMT+0': 'Africa/Casablanca',
    'GMT+1': 'Europe/Amsterdam',
    'GMT+2': 'Europe/Athens',
    'GMT+3': 'Europe/Moscow',
    'GMT+4': 'Asia/Baku',
    'GMT+5': 'Asia/Oral',
    'GMT+6': 'Asia/Almaty',
    'GMT+7': 'Asia/Bangkok',
    'GMT+8': 'Asia/Hong_Kong',
    'GMT+9': 'Asia/Tokyo',
    'GMT+10': 'Pacific/Guam',
    'GMT+11': 'Pacific/Noumea',
    'GMT+12': 'Pacific/Fiji',
    'GMT+13': 'Pacific/Tongatapu',
    'GMT-11': 'Pacific/Midway',
    'GMT-10': 'Pacific/Honolulu',
}


class Error(Exception):
  """Raised when an error occurs in a util"""


def abs_path(path):
  """Resolve the '.' and '~' in a path to get the absolute path.

  Args:
    path: The path to expand.

  Returns:
    The absolute path of the input path.
  """
  return os.path.abspath(os.path.expanduser(path))


def create_dir(path):
  """Creates a directory if it does not exist already.

  Args:
    path: The path of the directory to create.
  """
  full_path = abs_path(path)
  if not os.path.exists(full_path):
    try:
      os.makedirs(full_path)
    except OSError as e:
      # ignore the error for dir already exist.
      if e.errno != errno.EEXIST:
        raise


def create_alias(target_path, alias_path):
  """Creates an alias at 'alias_path' pointing to the file 'target_path'.

  On Unix, this is implemented via symlink. On Windows, this is done by
  creating a Windows shortcut file.

  Args:
    target_path: Destination path that the alias should point to.
    alias_path: Path at which to create the new alias.
  """
  if platform.system() == 'Windows' and not alias_path.endswith('.lnk'):
    alias_path += '.lnk'
  if os.path.lexists(alias_path):
    os.remove(alias_path)
  if platform.system() == 'Windows':
    from win32com import client

    shell = client.Dispatch('WScript.Shell')
    shortcut = shell.CreateShortCut(alias_path)
    shortcut.Targetpath = target_path
    shortcut.save()
  else:
    os.symlink(target_path, alias_path)


def get_current_epoch_time():
  """Current epoch time in milliseconds.

  Returns:
    An integer representing the current epoch time in milliseconds.
  """
  return int(round(time.time() * 1000))


def get_current_human_time():
  """Returns the current time in human readable format.

  Returns:
    The current time stamp in Month-Day-Year Hour:Min:Sec format.
  """
  return time.strftime('%m-%d-%Y %H:%M:%S ')


def epoch_to_human_time(epoch_time):
  """Converts an epoch timestamp to human readable time.

  This essentially converts an output of get_current_epoch_time to an output
  of get_current_human_time

  Args:
    epoch_time: An integer representing an epoch timestamp in milliseconds.

  Returns:
    A time string representing the input time.
    None if input param is invalid.
  """
  if isinstance(epoch_time, int):
    try:
      d = datetime.datetime.fromtimestamp(epoch_time / 1000)
      return d.strftime('%m-%d-%Y %H:%M:%S ')
    except ValueError:
      return None


def get_timezone_olson_id():
  """Return the Olson ID of the local (non-DST) timezone.

  Returns:
    A string representing one of the Olson IDs of the local (non-DST)
    timezone.
  """
  tzoffset = int(time.timezone / 3600)
  if tzoffset <= 0:
    gmt = f'GMT+{-tzoffset}'
  else:
    gmt = f'GMT-{tzoffset}'
  return GMT_to_olson[gmt]


def find_files(paths, file_predicate):
  """Locate files whose names and extensions match the given predicate in
  the specified directories.

  Args:
    paths: A list of directory paths where to find the files.
    file_predicate: A function that returns True if the file name and
      extension are desired.

  Returns:
    A list of files that match the predicate.
  """
  file_list = []
  for path in paths:
    p = abs_path(path)
    for dirPath, _, fileList in os.walk(p):
      for fname in fileList:
        name, ext = os.path.splitext(fname)
        if file_predicate(name, ext):
          file_list.append((dirPath, name, ext))
  return file_list


def load_file_to_base64_str(f_path):
  """Loads the content of a file into a base64 string.

  Args:
    f_path: full path to the file including the file name.

  Returns:
    A base64 string representing the content of the file in utf-8 encoding.
  """
  path = abs_path(f_path)
  with io.open(path, 'rb') as f:
    f_bytes = f.read()
    base64_str = base64.b64encode(f_bytes).decode('utf-8')
    return base64_str


def find_field(item_list, cond, comparator, target_field):
  """Finds the value of a field in a dict object that satisfies certain
  conditions.

  Args:
    item_list: A list of dict objects.
    cond: A param that defines the condition.
    comparator: A function that checks if an dict satisfies the condition.
    target_field: Name of the field whose value to be returned if an item
      satisfies the condition.

  Returns:
    Target value or None if no item satisfies the condition.
  """
  for item in item_list:
    if comparator(item, cond) and target_field in item:
      return item[target_field]
  return None


def rand_ascii_str(length):
  """Generates a random string of specified length, composed of ascii letters
  and digits.

  Args:
    length: The number of characters in the string.

  Returns:
    The random string generated.
  """
  letters = [random.choice(ascii_letters_and_digits) for _ in range(length)]
  return ''.join(letters)


# Thead/Process related functions.
def _collect_process_tree(starting_pid):
  """Collects PID list of the descendant processes from the given PID.

  This function only available on Unix like system.

  Args:
    starting_pid: The PID to start recursively traverse.

  Returns:
    A list of pid of the descendant processes.
  """
  ret = []
  stack = [starting_pid]

  while stack:
    pid = stack.pop()
    if platform.system() == 'Darwin':
      command = ['pgrep', '-P', str(pid)]
    else:
      command = [
          'ps',
          '-o',
          'pid',
          '--ppid',
          str(pid),
          '--noheaders',
      ]
    try:
      ps_results = subprocess.check_output(command).decode().strip()
    except subprocess.CalledProcessError:
      # Ignore if there is not child process.
      continue

    children_pid_list = [int(p.strip()) for p in ps_results.split('\n')]
    stack.extend(children_pid_list)
    ret.extend(children_pid_list)

  return ret


def _kill_process_tree(proc):
  """Kills the subprocess and its descendants."""
  if os.name == 'nt':
    # The taskkill command with "/T" option ends the specified process and any
    # child processes started by it:
    # https://docs.microsoft.com/en-us/windows-server/administration/windows-commands/taskkill
    subprocess.check_output(
        [
            'taskkill',
            '/F',
            '/T',
            '/PID',
            str(proc.pid),
        ]
    )
    return

  failed = []
  for child_pid in _collect_process_tree(proc.pid):
    try:
      os.kill(child_pid, signal.SIGTERM)
    except Exception:  # pylint: disable=broad-except
      failed.append(child_pid)
      logging.exception('Failed to kill standing subprocess %d', child_pid)

  try:
    proc.kill()
  except Exception:  # pylint: disable=broad-except
    failed.append(proc.pid)
    logging.exception('Failed to kill standing subprocess %d', proc.pid)

  if failed:
    raise Error('Failed to kill standing subprocesses: %s' % failed)


def concurrent_exec(func, param_list, max_workers=30, raise_on_exception=False):
  """Executes a function with different parameters pseudo-concurrently.

  This is basically a map function. Each element (should be an iterable) in
  the param_list is unpacked and passed into the function. Due to Python's
  GIL, there's no true concurrency. This is suited for IO-bound tasks.

  Args:
    func: The function that performs a task.
    param_list: A list of iterables, each being a set of params to be
      passed into the function.
    max_workers: int, the number of workers to use for parallelizing the
      tasks. By default, this is 30 workers.
    raise_on_exception: bool, raises all of the task failures if any of the
      tasks failed if `True`. By default, this is `False`.

  Returns:
    A list of return values from each function execution. If an execution
    caused an exception, the exception object will be the corresponding
    result.

  Raises:
    RuntimeError: If executing any of the tasks failed and
      `raise_on_exception` is True.
  """
  with concurrent.futures.ThreadPoolExecutor(
      max_workers=max_workers
  ) as executor:
    # Start the load operations and mark each future with its params
    future_to_params = {executor.submit(func, *p): p for p in param_list}
    return_vals = []
    exceptions = []
    for future in concurrent.futures.as_completed(future_to_params):
      params = future_to_params[future]
      try:
        return_vals.append(future.result())
      except Exception as exc:  # pylint: disable=broad-except
        logging.exception(
            '%s generated an exception: %s', params, traceback.format_exc()
        )
        return_vals.append(exc)
        exceptions.append(exc)
    if raise_on_exception and exceptions:
      error_messages = []
      for exception in exceptions:
        error_messages.append(
            ''.join(
                traceback.format_exception(
                    exception.__class__, exception, exception.__traceback__
                )
            )
        )
      raise RuntimeError('\n\n'.join(error_messages))
    return return_vals


# Provide hint for pytype checker to avoid the Union[bytes, str] case.
@overload
def run_command(
    cmd,
    stdout=...,
    stderr=...,
    shell=...,
    timeout=...,
    cwd=...,
    env=...,
    universal_newlines: Literal[False] = ...,
) -> Tuple[int, bytes, bytes]:
  ...


@overload
def run_command(
    cmd,
    stdout=...,
    stderr=...,
    shell=...,
    timeout=...,
    cwd=...,
    env=...,
    universal_newlines: Literal[True] = ...,
) -> Tuple[int, str, str]:
  ...


def run_command(
    cmd,
    stdout=None,
    stderr=None,
    shell=False,
    timeout=None,
    cwd=None,
    env=None,
    universal_newlines=False,
):
  """Runs a command in a subprocess.

  This function is very similar to subprocess.check_output. The main
  difference is that it returns the return code and std error output as well
  as supporting a timeout parameter.

  Args:
    cmd: string or list of strings, the command to run.
      See subprocess.Popen() documentation.
    stdout: file handle, the file handle to write std out to. If None is
      given, then subprocess.PIPE is used. See subprocess.Popen()
      documentation.
    stderr: file handle, the file handle to write std err to. If None is
      given, then subprocess.PIPE is used. See subprocess.Popen()
      documentation.
    shell: bool, True to run this command through the system shell,
      False to invoke it directly. See subprocess.Popen() docs.
    timeout: float, the number of seconds to wait before timing out.
      If not specified, no timeout takes effect.
    cwd: string, the path to change the child's current directory to before
      it is executed. Note that this directory is not considered when
      searching the executable, so you can't specify the program's path
      relative to cwd.
    env: dict, a mapping that defines the environment variables for the
      new process. Default behavior is inheriting the current process'
      environment.
    universal_newlines: bool, True to open file objects in text mode, False in
      binary mode.

  Returns:
    A 3-tuple of the consisting of the return code, the std output, and the
      std error.

  Raises:
    subprocess.TimeoutExpired: The command timed out.
  """
  if stdout is None:
    stdout = subprocess.PIPE
  if stderr is None:
    stderr = subprocess.PIPE
  process = subprocess.Popen(
      cmd,
      stdout=stdout,
      stderr=stderr,
      shell=shell,
      cwd=cwd,
      env=env,
      universal_newlines=universal_newlines,
  )
  timer = None
  timer_triggered = threading.Event()
  if timeout and timeout > 0:
    # The wait method on process will hang when used with PIPEs with large
    # outputs, so use a timer thread instead.

    def timeout_expired():
      timer_triggered.set()
      process.terminate()

    timer = threading.Timer(timeout, timeout_expired)
    timer.start()
  # If the command takes longer than the timeout, then the timer thread
  # will kill the subprocess, which will make it terminate.
  out, err = process.communicate()
  if timer is not None:
    timer.cancel()
  if timer_triggered.is_set():
    raise subprocess.TimeoutExpired(
        cmd=cmd, timeout=timeout, output=out, stderr=err
    )
  logging.debug(
      'cmd: %s, stdout: %s, stderr: %s, ret: %s',
      cli_cmd_to_string(cmd),
      out,
      err,
      process.returncode,
  )
  return process.returncode, out, err


def start_standing_subprocess(
    cmd,
    shell=False,
    env=None,
    stdout=subprocess.PIPE,
    stderr=subprocess.PIPE,
):
  """Starts a long-running subprocess.

  This is not a blocking call and the subprocess started by it should be
  explicitly terminated with stop_standing_subprocess.

  For short-running commands, you should use subprocess.check_call, which
  blocks.

  Args:
    cmd: string, the command to start the subprocess with.
    shell: bool, True to run this command through the system shell,
      False to invoke it directly. See subprocess.Popen() docs.
    env: dict, a custom environment to run the standing subprocess. If not
      specified, inherits the current environment. See subprocess.Popen()
      docs.
    stdout: None, subprocess.PIPE, subprocess.DEVNULL, an existing file
      descriptor, or an existing file object. See subprocess.Popen() docs.
    stderr: None, subprocess.PIPE, subprocess.DEVNULL, an existing file
      descriptor, or an existing file object. See subprocess.Popen() docs.

  Returns:
    The subprocess that was started.
  """
  logging.debug('Starting standing subprocess with: %s', cmd)
  proc = subprocess.Popen(
      cmd,
      stdin=subprocess.PIPE,
      stdout=stdout,
      stderr=stderr,
      shell=shell,
      env=env,
  )
  # Leaving stdin open causes problems for input, e.g. breaking the
  # code.inspect() shell (http://stackoverflow.com/a/25512460/1612937), so
  # explicitly close it assuming it is not needed for standing subprocesses.
  proc.stdin.close()
  proc.stdin = None
  logging.debug('Started standing subprocess %d', proc.pid)
  return proc


def stop_standing_subprocess(proc):
  """Stops a subprocess started by start_standing_subprocess.

  Before killing the process, we check if the process is running, if it has
  terminated, Error is raised.

  Catches and ignores the PermissionError which only happens on Macs.

  Args:
    proc: Subprocess to terminate.

  Raises:
    Error: if the subprocess could not be stopped.
  """
  logging.debug('Stopping standing subprocess %d', proc.pid)

  _kill_process_tree(proc)

  # Call wait and close pipes on the original Python object so we don't get
  # runtime warnings.
  if proc.stdout:
    proc.stdout.close()
  if proc.stderr:
    proc.stderr.close()
  proc.wait()
  logging.debug('Stopped standing subprocess %d', proc.pid)


def wait_for_standing_subprocess(proc, timeout=None):
  """Waits for a subprocess started by start_standing_subprocess to finish
  or times out.

  Propagates the exception raised by the subprocess.wait(.) function.
  The subprocess.TimeoutExpired exception is raised if the process timed-out
  rather than terminating.

  If no exception is raised: the subprocess terminated on its own. No need
  to call stop_standing_subprocess() to kill it.

  If an exception is raised: the subprocess is still alive - it did not
  terminate. Either call stop_standing_subprocess() to kill it, or call
  wait_for_standing_subprocess() to keep waiting for it to terminate on its
  own.

  If the corresponding subprocess command generates a large amount of output
  and this method is called with a timeout value, then the command can hang
  indefinitely. See http://go/pylib/subprocess.html#subprocess.Popen.wait

  This function does not support Python 2.

  Args:
    p: Subprocess to wait for.
    timeout: An integer number of seconds to wait before timing out.
  """
  proc.wait(timeout)


def get_available_host_port():
  """Gets a host port number available for adb forward.

  DEPRECATED: This method is unreliable. Pass `tcp:0` to adb forward instead.

  Returns:
    An integer representing a port number on the host available for adb
    forward.

  Raises:
    Error: when no port is found after MAX_PORT_ALLOCATION_RETRY times.
  """
  logging.warning(
      'The method mobly.utils.get_available_host_port is deprecated because it '
      'is unreliable. Pass "tcp:0" to adb forward instead.'
  )

  # Only import adb module if needed.
  from mobly.controllers.android_device_lib import adb

  port = portpicker.pick_unused_port()
  if not adb.is_adb_available():
    return port
  for _ in range(MAX_PORT_ALLOCATION_RETRY):
    # Make sure adb is not using this port so we don't accidentally
    # interrupt ongoing runs by trying to bind to the port.
    if port not in adb.list_occupied_adb_ports():
      return port
    port = portpicker.pick_unused_port()
  raise Error(
      'Failed to find available port after {} retries'.format(
          MAX_PORT_ALLOCATION_RETRY
      )
  )


def grep(regex, output):
  """Similar to linux's `grep`, this returns the line in an output stream
  that matches a given regex pattern.

  It does not rely on the `grep` binary and is not sensitive to line endings,
  so it can be used cross-platform.

  Args:
    regex: string, a regex that matches the expected pattern.
    output: byte string, the raw output of the adb cmd.

  Returns:
    A list of strings, all of which are output lines that matches the
    regex pattern.
  """
  lines = output.decode('utf-8').strip().splitlines()
  results = []
  for line in lines:
    if re.search(regex, line):
      results.append(line.strip())
  return results


def cli_cmd_to_string(args):
  """Converts a cmd arg list to string.

  Args:
    args: list of strings, the arguments of a command.

  Returns:
    String representation of the command.
  """
  if isinstance(args, str):
    # Return directly if it's already a string.
    return args
  return ' '.join([shlex.quote(arg) for arg in args])


def get_settable_properties(cls):
  """Gets the settable properties of a class.

  Only returns the explicitly defined properties with setters.

  Args:
    cls: A class in Python.
  """
  results = []
  for attr, value in vars(cls).items():
    if isinstance(value, property) and value.fset is not None:
      results.append(attr)
  return results


def find_subclasses_in_module(base_classes, module):
  """Finds the subclasses of the given classes in the given module.

  Args:
    base_classes: list of classes, the base classes to look for the
      subclasses of in the module.
    module: module, the module to look for the subclasses in.

  Returns:
    A list of all of the subclasses found in the module.
  """
  subclasses = []
  for _, module_member in module.__dict__.items():
    if inspect.isclass(module_member):
      for base_class in base_classes:
        if issubclass(module_member, base_class):
          subclasses.append(module_member)
  return subclasses


def find_subclass_in_module(base_class, module):
  """Finds the single subclass of the given base class in the given module.

  Args:
    base_class: class, the base class to look for a subclass of in the module.
    module: module, the module to look for the single subclass in.

  Returns:
    The single subclass of the given base class.

  Raises:
    ValueError: If the number of subclasses found was not exactly one.
  """
  subclasses = find_subclasses_in_module([base_class], module)
  if len(subclasses) != 1:
    raise ValueError(
        'Expected 1 subclass of %s per module, found %s.'
        % (base_class.__name__, [subclass.__name__ for subclass in subclasses])
    )
  return subclasses[0]
