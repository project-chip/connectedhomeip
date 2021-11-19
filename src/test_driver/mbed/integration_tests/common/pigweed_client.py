# Copyright (c) 2009-2021 Arm Limited
# SPDX-License-Identifier: Apache-2.0
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

import os
from pathlib import Path
import importlib

from pw_hdlc.rpc import HdlcRpcClient, default_channels


class PigweedClient:
    def __init__(self, device, protos):
        """
        Pigweed Client class containing RPC client initialization and service functions
        Create HdlcRpcCLient object and redirect serial communication to it
        :param device: test device instance
        :param protos: array of RPC protocols
        """
        self.device = device
        self.device.stop()
        self.last_timeout = self.device.serial.get_timeout()
        self.device.serial.set_timeout(0.01)
        self._pw_rpc_client = HdlcRpcClient(lambda: self.device.serial.read(4096),
                                            protos, default_channels(self.device.serial.write))
        self._rpcs = self._pw_rpc_client.rpcs()

    def __del__(self):
        self.device.serial.set_timeout(self.last_timeout)
        self.device.start()

    @property
    def rpcs(self):
        return self._rpcs
