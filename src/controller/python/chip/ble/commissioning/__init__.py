#
#    Copyright (c) 2021 Project CHIP Authors
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#
from typing import Optional
from chip.internal import GetCommissioner
from chip.internal.commissioner import PairingState
from queue import Queue
from dataclasses import dataclass
from enum import Enum

TEST_NODE_ID = 11223344


class PairNotificationType(Enum):
    NETWORK_CREDENTIALS = 0
    OPERATIONAL_CREDENTIALS = 1
    COMPLETE = 2


@dataclass
class _PairNotification:
    type: PairNotificationType
    csr: Optional[bytes]
    error_code: Optional[int]


class Connection:
    def __init__(self, controller):
        self.needsNetworkCredentials = False
        self.needsOperationalCredentials = False
        self.paired = False

        self._controller = controller
        self._pair_queue = Queue()

    def ConnectToWiFi(self, ssid: str, password: str):
        if not self.needsNetworkCredentials:
            raise Exception("Not requiring network credentials yet.")

        self._controller.PairSendWiFiCredentials(ssid, password)
        self._WaitForPairProgress()

    def ConnectToThread(self, blob: bytes):
        if not self.needsNetworkCredentials:
            raise Exception("Not requiring network credentials yet.")

        self._controller.PairSendThreadCredentials(blob)
        self._WaitForPairProgress()

    def _Pair(self, discriminator: int, pin: int, deprecated_nodeid: int):
        """Sets up controller callbakcs and initiates BLE pairing."""

        self._controller.on_network_credentials_requested = self._OnNetworkCredentialsRequested
        self._controller.on_operational_credentials_requested = self._OnOperationalCredentialsRequested
        self._controller.on_pairing_complete = self._OnPairingComplete

        self._controller.BlePair(deprecated_nodeid, pin, discriminator)

    def _OnNetworkCredentialsRequested(self):
        self._pair_queue.put(_PairNotification(
            PairNotificationType.NETWORK_CREDENTIALS, None, None))

    def _OnOperationalCredentialsRequested(self, csr):
        self._pair_queue.put(_PairNotification(
            PairNotificationType.OPERATIONAL_CREDENTIALS, csr, None))

    def _OnPairingComplete(self, err):
        self._pair_queue.put(_PairNotification(
            PairNotificationType.COMPLETE, None, err))

    def _WaitForPairProgress(self):
        """Waits for some pairing callback progress.

        Throws on pairing failure. Handles the progress on success.
        """

        if self.paired:
            # Already paired, no moore progress possible
            return

        step = self._pair_queue.get()

        self.needsNetworkCredentials = step.type == PairNotificationType.NETWORK_CREDENTIALS
        self.needsOperationalCredentials = step.type == PairNotificationType.OPERATIONAL_CREDENTIALS
        self.paired = step.type == PairNotificationType.COMPLETE

        if step.type == PairNotificationType.COMPLETE:
            if step.error_code != 0:
                raise Exception('Pairing ended with error code %d' %
                                step.error_code)


def _StartAsyncConnection(discriminator: int, pin: int, deprecated_nodeid: Optional[int] = None) -> Connection:
    """Initiates the connection to a BLE device for the purpose of commissioning

    Args:
      discriminator:      the advertisement discriminator to use to connect to
                          the device
      pin:                the pin number used to pair with the device (must match the
                          device secret pin, typically available in a QR code)
      deprecated_nodeid:  TEMPORARY, TO BE REMOVED nodeid that will be set during
                          the commissioning. This will be replaced by setting the node id
                          at the time of setting operational credentials once such
                          credentials are supported.
    """
    if not deprecated_nodeid:
        deprecated_nodeid = TEST_NODE_ID

    controller = GetCommissioner()

    if controller.pairing_state != PairingState.INITIALIZED:
        raise Exception("Controller is not ready to start a new pairing")

    connection = Connection(controller)
    connection._Pair(discriminator, pin, deprecated_nodeid)

    return connection


def Connect(discriminator: int, pin: int, deprecated_nodeid: Optional[int] = None) -> Connection:
    """Connects to a BLE device for the purpose of commissioning.
    Will block until the remote device can be operated on (e.g. setting network
    credentials or operational credentials.)

    Args:
      discriminator:      the advertisement discriminator to use to connect to
                          the device
      pin:                the pin number used to pair with the device (must match the
                          device secret pin, typically available in a QR code)
      deprecated_nodeid:  TEMPORARY, TO BE REMOVED nodeid that will be set during
                          the commissioning. This will be replaced by setting the node id
                          at the time of setting operational credentials once such
                          credentials are supported.
    """
    connection = _StartAsyncConnection(discriminator, pin, deprecated_nodeid)
    connection._WaitForPairProgress()

    return connection


__all__ = [
    'Connect',
]
