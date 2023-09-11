# SPDX-FileCopyrightText: 2009-2021 Arm Limited
# SPDX-License-Identifier: Apache-2.0

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
