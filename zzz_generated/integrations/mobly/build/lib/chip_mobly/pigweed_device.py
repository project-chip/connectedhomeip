# Copyright (c) 2020 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import importlib
import os
from pathlib import Path

import serial  # type: ignore
from pw_hdlc.rpc import HdlcRpcClient

# Point the script to the .proto file with our RPC services.
PROTO = Path(os.environ["PW_ROOT"], "pw_rpc/pw_rpc_protos/echo.proto")

MOBLY_CONTROLLER_CONFIG_NAME = "PigweedDevice"


class Error(Exception):
    """This is the Exception class defined for all errors."""


class PigweedDevice:
    def __init__(self, device_tty, baud, platform_module=None, platform_args=None):
        self.pw_rpc_client = HdlcRpcClient(
            serial.Serial(device_tty, baud), [PROTO])
        self._platform = None
        print("Platform args: %s" % platform_args)
        print("Platform module: %s" % platform_module)
        if platform_module:
            m = importlib.import_module(platform_module)
            create_platform_method = getattr(m, "create_platform")
            self._platform = create_platform_method(platform_args)

    def rpcs(self):
        return self.pw_rpc_client.rpcs().pw.rpc

    @property
    def platform(self):
        return self._platform


def create(configs):
    """Initializes the CHIP devices based on the testbed configuration.

    Args:
      configs: a list of testbed configs.

    Returns:
      a list of device objects
    """
    objs = []
    for config in configs:
        _validate_config(config)
        device = PigweedDevice(**config)
        objs.append(device)
    return objs


def destroy(unused_objs):
    """Destroys the wearable objects.

    Args:
      unused_objs: a list of device objects.
    """
    pass


def _validate_config(config):
    """Verifies that a config dict for a CHIP device is valid.

    Args:
      config: A dict that is the configuration for a CHIP device.

    Raises:
      chip_device.Error: Config file is not valid.
    """
    required_keys = ["device_tty", "baud"]  # A placeholder.
    for key in required_keys:
        if key not in config:
            raise Error("Required key %s missing from config %s" %
                        (key, config))
