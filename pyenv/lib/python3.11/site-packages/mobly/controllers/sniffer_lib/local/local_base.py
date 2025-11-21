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
Class for Local sniffers - i.e. running on the local machine.

This class provides configuration for local interfaces but leaves
the actual capture (sniff) to sub-classes.
"""

import os
import shutil
import subprocess
import tempfile
from mobly import logger
from mobly import utils
from mobly.controllers import sniffer


class SnifferLocalBase(sniffer.Sniffer):
  """This class defines the common behaviors of WLAN sniffers running on
  WLAN interfaces of the local machine.

  Specific mechanisms to capture packets over the local WLAN interfaces are
  implemented by sub-classes of this class - i.e. it is not a final class.
  """

  def __init__(self, interface, logger, base_configs=None):
    """See base class documentation"""
    self._base_configs = None
    self._capture_file_path = ""
    self._interface = ""
    self._logger = logger
    self._process = None
    self._temp_capture_file_path = ""

    if interface == "":
      raise sniffer.InvalidDataError("Empty interface provided")
    self._interface = interface
    self._base_configs = base_configs

    try:
      subprocess.check_call(["ifconfig", self._interface, "down"])
      subprocess.check_call(["iwconfig", self._interface, "mode", "monitor"])
      subprocess.check_call(["ifconfig", self._interface, "up"])
    except Exception as err:
      raise sniffer.ExecutionError(err)

  def get_interface(self):
    """See base class documentation"""
    return self._interface

  def get_type(self):
    """See base class documentation"""
    return "local"

  def get_capture_file(self):
    return self._capture_file_path

  def _pre_capture_config(self, override_configs=None):
    """Utility function which configures the wireless interface per the
    specified configurations. Operation is performed before every capture
    start using baseline configurations (specified when sniffer initialized)
    and override configurations specified here.
    """
    final_configs = {}
    if self._base_configs:
      final_configs.update(self._base_configs)
    if override_configs:
      final_configs.update(override_configs)

    if sniffer.Sniffer.CONFIG_KEY_CHANNEL in final_configs:
      try:
        subprocess.check_call(
            [
                "iwconfig",
                self._interface,
                "channel",
                str(final_configs[sniffer.Sniffer.CONFIG_KEY_CHANNEL]),
            ]
        )
      except Exception as err:
        raise sniffer.ExecutionError(err)

  def _get_command_line(
      self, additional_args=None, duration=None, packet_count=None
  ):
    """Utility function to be implemented by every child class - which
    are the concrete sniffer classes. Each sniffer-specific class should
    derive the command line to execute its sniffer based on the specified
    arguments.
    """
    raise NotImplementedError("Base class should not be called directly!")

  def _post_process(self):
    """Utility function which is executed after a capture is done. It
    moves the capture file to the requested location.
    """
    self._process = None
    shutil.move(self._temp_capture_file_path, self._capture_file_path)

  def start_capture(
      self,
      override_configs=None,
      additional_args=None,
      duration=None,
      packet_count=None,
  ):
    """See base class documentation"""
    if self._process is not None:
      raise sniffer.InvalidOperationError(
          "Trying to start a sniff while another is still running!"
      )
    capture_dir = os.path.join(
        self._logger.log_path, "Sniffer-{}".format(self._interface)
    )
    os.makedirs(capture_dir, exist_ok=True)
    self._capture_file_path = os.path.join(
        capture_dir, "capture_{}.pcap".format(logger.get_log_file_timestamp())
    )

    self._pre_capture_config(override_configs)
    _, self._temp_capture_file_path = tempfile.mkstemp(suffix=".pcap")

    cmd = self._get_command_line(
        additional_args=additional_args,
        duration=duration,
        packet_count=packet_count,
    )

    self._process = utils.start_standing_subprocess(cmd)
    return sniffer.ActiveCaptureContext(self, duration)

  def stop_capture(self):
    """See base class documentation"""
    if self._process is None:
      raise sniffer.InvalidOperationError(
          "Trying to stop a non-started process"
      )
    utils.stop_standing_subprocess(self._process)
    self._post_process()

  def wait_for_capture(self, timeout=None):
    """See base class documentation"""
    if self._process is None:
      raise sniffer.InvalidOperationError(
          "Trying to wait on a non-started process"
      )
    try:
      utils.wait_for_standing_subprocess(self._process, timeout)
      self._post_process()
    except subprocess.TimeoutExpired:
      self.stop_capture()
