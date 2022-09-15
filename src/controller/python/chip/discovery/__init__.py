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

import logging
import time
import threading
import enum

from dataclasses import dataclass
from typing import List, Dict, Set, Callable

from chip.discovery.library_handle import _GetDiscoveryLibraryHandle
from chip.discovery.types import DiscoverSuccessCallback_t, DiscoverFailureCallback_t
from chip.native import PyChipError


class FilterType(enum.IntEnum):
    # These must match chip::Dnssd::DiscoveryFilterType values (barring the naming convention)
    NONE = 0
    SHORT_DISCRIMINATOR = 1
    LONG_DISCRIMINATOR = 2
    VENDOR_ID = 3
    DEVICE_TYPE = 4
    COMMISSIONING_MODE = 5
    INSTANCE_NAME = 6
    COMMISSIONER = 7
    COMPRESSED_FABRIC_ID = 8


@dataclass(unsafe_hash=True)
class PeerId:
    """Represents a remote peer id."""
    fabricId: int
    nodeId: int


@dataclass(unsafe_hash=True)
class NodeAddress:
    """Represents a distinct address where a node can be reached."""
    interface: int
    ip: str
    port: int


@dataclass
class AggregatedDiscoveryResults:
    """Discovery results for a node."""
    peerId: PeerId
    addresses: Set[NodeAddress]


@dataclass
class PendingDiscovery:
    """Accumulator for ongoing discovery."""
    result: AggregatedDiscoveryResults
    callback: Callable[[AggregatedDiscoveryResults], None]
    expireTime: int
    firstResultTime: int


@dataclass
class CommissionableNode():
    instanceName: str = None
    hostName: str = None
    port: int = None
    longDiscriminator: int = None
    vendorId: int = None
    productId: int = None
    commissioningMode: int = None
    deviceType: int = None
    deviceName: str = None
    pairingInstruction: str = None
    pairingHint: int = None
    mrpRetryIntervalIdle: int = None
    mrpRetryIntervalActive: int = None
    supportsTcp: bool = None
    addresses: List[str] = None


# Milliseconds to wait for additional results onece a single result has
# been received
_RESULT_WAIT_TIME_SEC = 0.05


class _PendingDiscoveries:
    """Manages a list of pending discoveries and associated callbacks."""

    activeDiscoveries: List[PendingDiscovery] = []

    def __init__(self):
        self.operationCondition = threading.Condition()
        self.resolution = threading.Thread(
            target=self.ResolutionThread, daemon=True)
        self.resolution.start()

    def Start(self, peerId: PeerId, callback: Callable[[AggregatedDiscoveryResults], None], timeoutMs: int):
        """Add a new pending result item to the internal list."""
        with self.operationCondition:
            self.activeDiscoveries.append(
                PendingDiscovery(
                    AggregatedDiscoveryResults(peerId, addresses=set()),
                    callback=callback,
                    expireTime=time.time() + timeoutMs/1000.0,
                    firstResultTime=0,
                ))
            self.operationCondition.notify()

    def OnSuccess(self, peerId: PeerId, address: NodeAddress):
        """Notify of a succesful address resolution."""
        with self.operationCondition:
            for item in self.activeDiscoveries:
                if item.result.peerId != peerId:
                    continue

                item.result.addresses.add(address)
                if item.firstResultTime == 0:
                    item.firstResultTime = time.time()
            self.operationCondition.notify()

    def ResolutionThread(self):
        while True:
            with self.operationCondition:
                self.operationCondition.wait(
                    self.ComputeNextEventTimeoutSeconds())

                updatedDiscoveries = []

                for item in self.activeDiscoveries:
                    if self.NeedsCallback(item):
                        try:
                            item.callback(item.result)
                        except:
                            logging.exception("Node discovery callback failed")
                    else:
                        updatedDiscoveries.append(item)

                self.activeDiscoveries = updatedDiscoveries

    def NeedsCallback(self, item: PendingDiscovery):
        """Find out if the callback needs to be called for the given item."""
        now = time.time()
        if item.expireTime <= now:
            return True

        if (item.firstResultTime > 0) and (item.firstResultTime + _RESULT_WAIT_TIME_SEC <= now):
            return True

        return False

    def ComputeNextEventTimeoutSeconds(self):
        """Compute how much a thread needs to sleep based on the active discoveries list."""
        sleepTimeSec = 10.0  # just in case

        now = time.time()
        for item in self.activeDiscoveries:
            # figure out expiry time for an item
            expireSleep = item.expireTime - now
            if expireSleep < sleepTimeSec:
                sleepTimeSec = expireSleep

            # Allow only a short time window for 'additional results' once we
            # have one
            resultSleep = (item.firstResultTime + _RESULT_WAIT_TIME_SEC) - now
            if resultSleep < sleepTimeSec:
                sleepTimeSec = resultSleep

        # in case our processing missed some expire times, set a very short
        # sleep
        if sleepTimeSec <= 0:
            sleepTimeSec = 0.001

        return sleepTimeSec


# define firstResultTime


# All pending discovery operations awayting callback results
_gPendingDiscoveries = _PendingDiscoveries()


@DiscoverSuccessCallback_t
def _DiscoverSuccess(fabric: int, node: int, interface: int, ip: str,  port: int):
    peerId = PeerId(fabric, node)
    address = NodeAddress(interface, ip, port)

    global _gPendingDiscoveries
    _gPendingDiscoveries.OnSuccess(peerId, address)


@DiscoverFailureCallback_t
def _DiscoverFailure(fabric: int, node: int, errorCode: PyChipError):
    # Many discovery errors currently do not include a useful node/fabric id
    # hence we just log and rely on discovery timeouts to return 'no data'
    logging.error("Discovery failure, error %d", errorCode.code)


def FindAddressAsync(fabricid: int, nodeid: int, callback, timeout_ms=1000):
    """Discovers the IP address(es) of a node.

    Args:
      fabricid: the fabric to which the node is attached
      nodeid:   the node id to find
      callback: Will be called once node resolution completes.
    """

    _GetDiscoveryLibraryHandle().pychip_discovery_set_callbacks(
        _DiscoverSuccess, _DiscoverFailure)

    global _gPendingDiscoveries
    _gPendingDiscoveries.Start(
        PeerId(fabricid, nodeid),
        callback,
        timeout_ms
    )

    res = _GetDiscoveryLibraryHandle().pychip_discovery_resolve(fabricid, nodeid)
    if res != 0:
        raise Exception("Failed to start node resolution")


class _SyncAddressFinder:
    """Performs a blocking wait for an address resolution operation."""

    def __init__(self):
        self.semaphore = threading.Semaphore(value=0)
        self.result = None

    def Callback(self, data):
        self.result = data
        self.semaphore.release()

    def WaitForResult(self):
        self.semaphore.acquire()
        return self.result


def FindAddress(fabricid, nodeid, timeout_ms=1000):
    """Performs an address discovery for a node and returns the result."""
    finder = _SyncAddressFinder()
    FindAddressAsync(fabricid, nodeid, finder.Callback, timeout_ms)
    return finder.WaitForResult()
