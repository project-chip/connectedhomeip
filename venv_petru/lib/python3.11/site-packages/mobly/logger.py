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

import datetime
import logging
import os
import re
import sys
from typing import Any, MutableMapping, Tuple

from mobly import records
from mobly import utils

LINUX_MAX_FILENAME_LENGTH = 255
# Filename sanitization mappings for Windows.
# See https://docs.microsoft.com/en-us/windows/win32/fileio/naming-a-file#naming-conventions
# Although the documentation says that 260 (including terminating nul, so 259)
# is the max length. From manually testing on a Windows 10 machine, the actual
# length seems to be lower.
WINDOWS_MAX_FILENAME_LENGTH = 237
WINDOWS_RESERVED_CHARACTERS_REPLACEMENTS = {
    '<': '-',
    '>': '-',
    ':': '-',
    '"': '_',
    '/': '_',
    '\\': '_',
    '|': ',',
    '?': ',',
    '*': ',',
    # Integer zero (i.e. NUL) is not a valid character.
    # While integers 1-31 are also usually valid, they aren't sanitized because
    # they are situationally valid.
    chr(0): '0',
}
# Note, although the documentation does not specify as such, COM0 and LPT0 are
# also invalid/reserved filenames.
WINDOWS_RESERVED_FILENAME_REGEX = re.compile(
    r'^(CON|PRN|AUX|NUL|(COM|LPT)[0-9])(\.[^.]*)?$', re.IGNORECASE
)
WINDOWS_RESERVED_FILENAME_PREFIX = 'mobly_'

log_line_format = '%(asctime)s.%(msecs).03d %(levelname)s %(message)s'
# The micro seconds are added by the format string above,
# so the time format does not include ms.
log_line_time_format = '%m-%d %H:%M:%S'
log_line_timestamp_len = 18

logline_timestamp_re = re.compile(r'\d\d-\d\d \d\d:\d\d:\d\d.\d\d\d')


def _parse_logline_timestamp(t):
  """Parses a logline timestamp into a tuple.

  Args:
    t: Timestamp in logline format.

  Returns:
    An iterable of date and time elements in the order of month, day, hour,
    minute, second, microsecond.
  """
  date, time = t.split(' ')
  month, day = date.split('-')
  h, m, s = time.split(':')
  s, ms = s.split('.')
  return (month, day, h, m, s, ms)


def is_valid_logline_timestamp(timestamp):
  if len(timestamp) == log_line_timestamp_len:
    if logline_timestamp_re.match(timestamp):
      return True
  return False


def logline_timestamp_comparator(t1, t2):
  """Comparator for timestamps in logline format.

  Args:
    t1: Timestamp in logline format.
    t2: Timestamp in logline format.

  Returns:
    -1 if t1 < t2; 1 if t1 > t2; 0 if t1 == t2.
  """
  dt1 = _parse_logline_timestamp(t1)
  dt2 = _parse_logline_timestamp(t2)
  for u1, u2 in zip(dt1, dt2):
    if u1 < u2:
      return -1
    elif u1 > u2:
      return 1
  return 0


def _get_timestamp(time_format, delta=None):
  t = datetime.datetime.now()
  if delta:
    t = t + datetime.timedelta(seconds=delta)
  return t.strftime(time_format)[:-3]


def epoch_to_log_line_timestamp(epoch_time, time_zone=None):
  """Converts an epoch timestamp in ms to log line timestamp format, which
  is readible for humans.

  Args:
    epoch_time: integer, an epoch timestamp in ms.
    time_zone: instance of tzinfo, time zone information.
      Using pytz rather than python 3.2 time_zone implementation for
      python 2 compatibility reasons.

  Returns:
    A string that is the corresponding timestamp in log line timestamp
    format.
  """
  s, ms = divmod(epoch_time, 1000)
  d = datetime.datetime.fromtimestamp(s, tz=time_zone)
  return d.strftime('%m-%d %H:%M:%S.') + str(ms)


def get_log_line_timestamp(delta=None):
  """Returns a timestamp in the format used by log lines.

  Default is current time. If a delta is set, the return value will be
  the current time offset by delta seconds.

  Args:
    delta: Number of seconds to offset from current time; can be negative.

  Returns:
    A timestamp in log line format with an offset.
  """
  return _get_timestamp('%m-%d %H:%M:%S.%f', delta)


def get_log_file_timestamp(delta=None):
  """Returns a timestamp in the format used for log file names.

  Default is current time. If a delta is set, the return value will be
  the current time offset by delta seconds.

  Args:
    delta: Number of seconds to offset from current time; can be negative.

  Returns:
    A timestamp in log filen name format with an offset.
  """
  return _get_timestamp('%m-%d-%Y_%H-%M-%S-%f', delta)


def _setup_test_logger(log_path, console_level, prefix=None):
  """Customizes the root logger for a test run.

  The logger object has a stream handler and a file handler. The stream
  handler logs INFO level to the terminal, the file handler logs DEBUG
  level to files.

  Args:
    log_path: Location of the log file.
    console_level: Log level threshold used for log messages printed
      to the console. Logs with a level less severe than
      console_level will not be printed to the console.
    prefix: A prefix for each log line in terminal.
    filename: Name of the log file. The default is the time the logger
      is requested.
  """
  log = logging.getLogger()
  kill_test_logger(log)
  log.propagate = False
  log.setLevel(logging.DEBUG)
  # Log info to stream
  terminal_format = log_line_format
  if prefix:
    terminal_format = '[%s] %s' % (prefix, log_line_format)
  c_formatter = logging.Formatter(terminal_format, log_line_time_format)
  ch = logging.StreamHandler(sys.stdout)
  ch.setFormatter(c_formatter)
  ch.setLevel(console_level)
  # Log everything to file
  f_formatter = logging.Formatter(log_line_format, log_line_time_format)
  # Write logger output to files
  fh_info = logging.FileHandler(
      os.path.join(log_path, records.OUTPUT_FILE_INFO_LOG)
  )
  fh_info.setFormatter(f_formatter)
  fh_info.setLevel(logging.INFO)
  fh_debug = logging.FileHandler(
      os.path.join(log_path, records.OUTPUT_FILE_DEBUG_LOG)
  )
  fh_debug.setFormatter(f_formatter)
  fh_debug.setLevel(logging.DEBUG)
  log.addHandler(ch)
  log.addHandler(fh_info)
  log.addHandler(fh_debug)
  log.log_path = log_path
  logging.log_path = log_path
  logging.root_output_path = log_path


def kill_test_logger(logger):
  """Cleans up a test logger object by removing all of its handlers.

  Args:
    logger: The logging object to clean up.
  """
  for h in list(logger.handlers):
    logger.removeHandler(h)
    if isinstance(h, logging.FileHandler):
      h.close()


def create_latest_log_alias(actual_path, alias):
  """Creates a symlink to the latest test run logs.

  Args:
    actual_path: string, the source directory where the latest test run's
      logs are.
    alias: string, the name of the directory to contain the latest log
      files.
  """
  alias_path = os.path.join(os.path.dirname(actual_path), alias)
  utils.create_alias(actual_path, alias_path)


def setup_test_logger(
    log_path, prefix=None, alias='latest', console_level=logging.INFO
):
  """Customizes the root logger for a test run.

  In addition to configuring the Mobly logging handlers, this also sets two
  attributes on the `logging` module for the output directories:

  root_output_path: path to the directory for the entire test run.
  log_path: same as `root_output_path` outside of a test class run. In the
    context of a test class run, this is the output directory for files
    specific to a test class.

  Args:
    log_path: string, the location of the report file.
    prefix: optional string, a prefix for each log line in terminal.
    alias: optional string, The name of the alias to use for the latest log
      directory. If a falsy value is provided, then the alias directory
      will not be created, which is useful to save storage space when the
      storage system (e.g. ZIP files) does not properly support
      shortcut/symlinks.
    console_level: optional logging level, log level threshold used for log
      messages printed to the console. Logs with a level less severe than
      console_level will not be printed to the console.
  """
  utils.create_dir(log_path)
  _setup_test_logger(log_path, console_level, prefix)
  logging.debug('Test output folder: "%s"', log_path)
  if alias:
    create_latest_log_alias(log_path, alias=alias)


def _truncate_filename(filename, max_length):
  """Truncates a filename while trying to preserve the extension.

  Args:
    filename: string, the filename to potentially truncate.

  Returns:
  The truncated filename that is less than or equal to the given maximum
  length.
  """
  if len(filename) <= max_length:
    return filename

  if '.' in filename:
    filename, extension = filename.rsplit('.', 1)
    # Subtract one for the extension's period.
    if len(extension) > max_length - 1:
      # This is kind of a degrenerate case where the extension is
      # extremely long, in which case, just return the truncated filename.
      return filename[:max_length]
    return '.'.join([filename[: max_length - len(extension) - 1], extension])
  else:
    return filename[:max_length]


def _sanitize_windows_filename(filename):
  """Sanitizes a filename for Windows.

  Refer to the following Windows documentation page for the rules:
  https://docs.microsoft.com/en-us/windows/win32/fileio/naming-a-file#naming-conventions

  If the filename matches one of Window's reserved file namespaces, then the
  `WINDOWS_RESERVED_FILENAME_PREFIX` (i.e. "mobly_") prefix will be appended
  to the filename to convert it into a valid Windows filename.

  Args:
    filename: string, the filename to sanitize for the Windows file system.

  Returns:
    A filename that should be safe to use on Windows.
  """
  if re.match(WINDOWS_RESERVED_FILENAME_REGEX, filename):
    return WINDOWS_RESERVED_FILENAME_PREFIX + filename

  filename = _truncate_filename(filename, WINDOWS_MAX_FILENAME_LENGTH)

  # In order to meet max length, none of these replacements should increase
  # the length of the filename.
  new_filename_chars = []
  for char in filename:
    if char in WINDOWS_RESERVED_CHARACTERS_REPLACEMENTS:
      new_filename_chars.append(WINDOWS_RESERVED_CHARACTERS_REPLACEMENTS[char])
    else:
      new_filename_chars.append(char)
  filename = ''.join(new_filename_chars)
  if filename.endswith('.') or filename.endswith(' '):
    # Filenames cannot end with a period or space on Windows.
    filename = filename[:-1] + '_'

  return filename


def sanitize_filename(filename):
  """Sanitizes a filename for various operating systems.

  Args:
    filename: string, the filename to sanitize.

  Returns:
    A string that is safe to use as a filename on various operating systems.
  """
  # Split `filename` into the directory and filename in case the user
  # accidentally passed in the full path instead of the name.
  dirname = os.path.dirname(filename)
  basename = os.path.basename(filename)
  basename = _sanitize_windows_filename(basename)
  basename = _truncate_filename(basename, LINUX_MAX_FILENAME_LENGTH)
  # Replace spaces with underscores for convenience reasons.
  basename = basename.replace(' ', '_')
  return os.path.join(dirname, basename)


def normalize_log_line_timestamp(log_line_timestamp):
  """Replace special characters in log line timestamp with normal characters.

  .. deprecated:: 1.10

    This method is obsolete with the more general `sanitize_filename` method
    and is only kept for backwards compatibility. In a future update, this
    method may be removed.

  Args:
    log_line_timestamp: A string in the log line timestamp format. Obtained
      with get_log_line_timestamp.

  Returns:
    A string representing the same time as input timestamp, but without
    special characters.
  """
  return sanitize_filename(log_line_timestamp)


class PrefixLoggerAdapter(logging.LoggerAdapter):
  """A wrapper that adds a prefix to each log line.

  This logger adapter class is like a decorator to Logger. It takes one
  Logger-like object and returns a new Logger-like object. The new Logger-like
  object will print logs with a custom prefix added. Creating new Logger-like
  objects doesn't modify the behavior of the old Logger-like object.

  Chaining multiple logger adapters is also supported. The multiple adapters
  will take effect in the order in which they are chained, i.e. the log will be
  '<prefix1> <prefix2> <prefix3> <message>' if we chain 3 PrefixLoggerAdapters.

  Example Usage:

  .. code-block:: python

    logger = PrefixLoggerAdapter(logging.getLogger(), {
      'log_prefix': <custom prefix>
    })

  Then each log line added by the logger will have a prefix:
  '<custom prefix> <message>'.
  """

  _KWARGS_TYPE = MutableMapping[str, Any]
  _PROCESS_RETURN_TYPE = Tuple[str, _KWARGS_TYPE]

  # The key of log_preifx item in the dict self.extra
  EXTRA_KEY_LOG_PREFIX: str = 'log_prefix'

  extra: _KWARGS_TYPE

  def process(self, msg: str, kwargs: _KWARGS_TYPE) -> _PROCESS_RETURN_TYPE:
    """Processes the logging call to insert contextual information.

    Args:
      msg: The logging message.
      kwargs: Keyword arguments passed in to a logging call.

    Returns:
      The message and kwargs modified.
    """
    new_msg = f'{self.extra[PrefixLoggerAdapter.EXTRA_KEY_LOG_PREFIX]} {msg}'
    return (new_msg, kwargs)

  def set_log_prefix(self, prefix: str) -> None:
    """Sets the log prefix to the given string.

    Args:
      prefix: The new log prefix.
    """
    self.debug('Setting the log prefix to "%s".', prefix)
    self.extra[PrefixLoggerAdapter.EXTRA_KEY_LOG_PREFIX] = prefix
