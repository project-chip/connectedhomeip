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
"""
Helper module for common telnet capability to communicate with
AttenuatorDevice(s).

User code shouldn't need to directly access this class.
"""

import telnetlib
from mobly.controllers import attenuator


def _ascii_string(uc_string):
  return str(uc_string).encode("ASCII")


class TelnetScpiClient:
  """This is an internal helper class for Telnet+SCPI command-based
  instruments. It should only be used by those implemention control libraries
  and not by any user code directly.
  """

  def __init__(self, tx_cmd_separator="\n", rx_cmd_separator="\n", prompt=""):
    self._tn = None
    self.tx_cmd_separator = tx_cmd_separator
    self.rx_cmd_separator = rx_cmd_separator
    self.prompt = prompt
    self.host = None
    self.port = None

  def open(self, host, port=23):
    if self._tn:
      self._tn.close()
    self.host = host
    self.port = port
    self._tn = telnetlib.Telnet()
    self._tn.open(host, port, 10)

  @property
  def is_open(self):
    return bool(self._tn)

  def close(self):
    if self._tn:
      self._tn.close()
      self._tn = None

  def cmd(self, cmd_str, wait_ret=True):
    if not isinstance(cmd_str, str):
      raise TypeError("Invalid command string", cmd_str)
    if not self.is_open:
      raise attenuator.Error("Telnet connection not open for commands")

    cmd_str.strip(self.tx_cmd_separator)
    self._tn.read_until(_ascii_string(self.prompt), 2)
    self._tn.write(_ascii_string(cmd_str + self.tx_cmd_separator))
    if wait_ret is False:
      return None

    match_idx, match_val, ret_text = self._tn.expect(
        [_ascii_string("\S+" + self.rx_cmd_separator)], 1
    )

    if match_idx == -1:
      raise attenuator.Error("Telnet command failed to return valid data")

    ret_text = ret_text.decode()
    ret_text = ret_text.strip(
        self.tx_cmd_separator + self.rx_cmd_separator + self.prompt
    )

    return ret_text
