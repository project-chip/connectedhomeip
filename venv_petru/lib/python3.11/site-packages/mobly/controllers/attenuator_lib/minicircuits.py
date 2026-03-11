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
This module has the class for controlling Mini-Circuits RCDAT series
attenuators over Telnet.

See http://www.minicircuits.com/softwaredownload/Prog_Manual-6-Programmable_Attenuator.pdf
"""

from mobly.controllers import attenuator
from mobly.controllers.attenuator_lib import telnet_scpi_client


class AttenuatorDevice:
  """This provides a specific telnet-controlled implementation of
  AttenuatorDevice for Mini-Circuits RC-DAT attenuators.

  Attributes:
      path_count: The number of signal attenuation path this device has.
  """

  def __init__(self, path_count=1):
    self.path_count = path_count
    # The telnet client used to communicate with the attenuator device.
    self._telnet_client = telnet_scpi_client.TelnetScpiClient(
        tx_cmd_separator="\r\n", rx_cmd_separator="\r\n", prompt=""
    )

  @property
  def is_open(self):
    """This function returns the state of the telnet connection to the
    underlying AttenuatorDevice.

    Returns:
        True if there is a successfully open connection to the
        AttenuatorDevice.
    """
    return bool(self._telnet_client.is_open)

  def open(self, host, port=23):
    """Opens a telnet connection to the desired AttenuatorDevice and
    queries basic information.

    Args:
        host: A valid hostname (IP address or DNS-resolvable name) to an
            MC-DAT attenuator instrument.
        port: An optional port number (defaults to telnet default 23)
    """
    self._telnet_client.open(host, port)
    config_str = self._telnet_client.cmd("MN?")
    if config_str.startswith("MN="):
      config_str = config_str[len("MN=") :]
    self.properties = dict(
        zip(["model", "max_freq", "max_atten"], config_str.split("-", 2))
    )
    self.max_atten = float(self.properties["max_atten"])

  def close(self):
    """Closes a telnet connection to the desired attenuator device.

    This should be called as part of any teardown procedure prior to the
    attenuator instrument leaving scope.
    """
    if self.is_open:
      self._telnet_client.close()

  def set_atten(self, idx, value):
    """Sets the attenuation value for a particular signal path.

    Args:
        idx: Zero-based index int which is the identifier for a particular
            signal path in an instrument. For instruments that only has one
            channel, this is ignored by the device.
        value: A float that is the attenuation value to set.

    Raises:
        Error: The underlying telnet connection to the instrument is not
            open.
        IndexError: The index of the attenuator is greater than the maximum
            index of the underlying instrument.
        ValueError: The requested set value is greater than the maximum
            attenuation value.
    """
    if not self.is_open:
      raise attenuator.Error(
          "Connection to attenuator at %s is not open!"
          % self._telnet_client.host
      )
    if idx + 1 > self.path_count:
      raise IndexError("Attenuator index out of range!", self.path_count, idx)
    if value > self.max_atten:
      raise ValueError("Attenuator value out of range!", self.max_atten, value)
    # The actual device uses one-based index for channel numbers.
    self._telnet_client.cmd("CHAN:%s:SETATT:%s" % (idx + 1, value))

  def get_atten(self, idx=0):
    """This function returns the current attenuation from an attenuator at a
    given index in the instrument.

    Args:
        idx: This zero-based index is the identifier for a particular
            attenuator in an instrument.

    Raises:
        Error: The underlying telnet connection to the instrument is not
            open.

    Returns:
        A float that is the current attenuation value.
    """
    if not self.is_open:
      raise attenuator.Error(
          "Connection to attenuator at %s is not open!"
          % self._telnet_client.host
      )
    if idx + 1 > self.path_count or idx < 0:
      raise IndexError("Attenuator index out of range!", self.path_count, idx)
    telnet_cmd = ":ATT?" if self.path_count == 1 else "CHAN:%s:ATT?" % (idx + 1)
    atten_val_str = self._telnet_client.cmd(telnet_cmd)
    atten_val = float(atten_val_str)
    return atten_val
