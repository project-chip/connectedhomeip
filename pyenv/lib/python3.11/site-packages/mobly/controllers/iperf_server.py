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

import io
import json
import logging
import os

from mobly import utils

MOBLY_CONTROLLER_CONFIG_NAME = 'IPerfServer'


def create(configs):
  results = []
  for c in configs:
    try:
      results.append(IPerfServer(c, logging.log_path))
    except:
      pass
  return results


def destroy(objs):
  for ipf in objs:
    try:
      ipf.stop()
    except:
      pass


class IPerfResult:

  def __init__(self, result_path):
    try:
      with io.open(result_path, 'r', encoding='utf-8') as f:
        self.result = json.load(f)
    except ValueError:
      with io.open(result_path, 'r', encoding='utf-8') as f:
        # Possibly a result from interrupted iperf run, skip first line
        # and try again.
        lines = f.readlines()[1:]
        self.result = json.loads(''.join(lines))

  def _has_data(self):
    """Checks if the iperf result has valid throughput data.

    Returns:
        True if the result contains throughput data. False otherwise.
    """
    return ('end' in self.result) and ('sum' in self.result['end'])

  def get_json(self):
    """
    Returns:
        The raw json output from iPerf.
    """
    return self.result

  @property
  def error(self):
    if 'error' not in self.result:
      return None
    return self.result['error']

  @property
  def avg_rate(self):
    """Average receiving rate in MB/s over the entire run.

    If the result is not from a success run, this property is None.
    """
    if not self._has_data or 'sum' not in self.result['end']:
      return None
    bps = self.result['end']['sum']['bits_per_second']
    return bps / 8 / 1024 / 1024

  @property
  def avg_receive_rate(self):
    """Average receiving rate in MB/s over the entire run. This data may
    not exist if iperf was interrupted.

    If the result is not from a success run, this property is None.
    """
    if not self._has_data or 'sum_received' not in self.result['end']:
      return None
    bps = self.result['end']['sum_received']['bits_per_second']
    return bps / 8 / 1024 / 1024

  @property
  def avg_send_rate(self):
    """Average sending rate in MB/s over the entire run. This data may
    not exist if iperf was interrupted.

    If the result is not from a success run, this property is None.
    """
    if not self._has_data or 'sum_sent' not in self.result['end']:
      return None
    bps = self.result['end']['sum_sent']['bits_per_second']
    return bps / 8 / 1024 / 1024


class IPerfServer:
  """Class that handles iperf3 operations."""

  def __init__(self, port, log_path):
    self.port = port
    self.log_path = os.path.join(log_path, 'iPerf{}'.format(self.port))
    self.iperf_str = 'iperf3 -s -J -p {}'.format(port)
    self.iperf_process = None
    self.log_files = []
    self.started = False

  def start(self, extra_args='', tag=''):
    """Starts iperf server on specified port.

    Args:
        extra_args: A string representing extra arguments to start iperf
            server with.
        tag: Appended to log file name to identify logs from different
            iperf runs.
    """
    if self.started:
      return
    utils.create_dir(self.log_path)
    if tag:
      tag = tag + ','
    out_file_name = 'IPerfServer,{},{}{}.log'.format(
        self.port, tag, len(self.log_files)
    )
    full_out_path = os.path.join(self.log_path, out_file_name)
    cmd = '%s %s > %s' % (self.iperf_str, extra_args, full_out_path)
    self.iperf_process = utils.start_standing_subprocess(cmd, shell=True)
    self.log_files.append(full_out_path)
    self.started = True

  def stop(self):
    if self.started:
      utils.stop_standing_subprocess(self.iperf_process)
      self.started = False
