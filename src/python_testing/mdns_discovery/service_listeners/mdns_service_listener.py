#
#    Copyright (c) 2025 Project CHIP Authors
#    All rights reserved.
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
from asyncio import Event, TimeoutError, wait_for

from zeroconf import ServiceListener, Zeroconf

logger = logging.getLogger(__name__)


class MdnsServiceListener(ServiceListener):
    """
    A service listener used during mDNS service discovery.

    The listener registers with an `AsyncZeroconf` instance to receive
    service add/update events. When such an event occurs, the internal
    asyncio `Event` (`updated_event`) is set.

    The event is entirely internal — `wait_for_service_update()` is
    expected to await `updated_event.wait()` to detect when a service
    record update has been received.
    """

    def __init__(self):
        self.updated_event = Event()

    def add_service(self, zeroconf: Zeroconf, service_type: str, name: str) -> None:
        self.updated_event.set()

    def update_service(self, zeroconf: Zeroconf, service_type: str, name: str) -> None:
        self.updated_event.set()

    def remove_service(self, zeroconf: Zeroconf, service_type: str, name: str) -> None:
        pass

    async def wait_for_service_update(self, service_name: str, rec_types: str, timeout: float) -> None:
        """
        Wait until a service add/update event occurs or timeout is reached.

        Args:
            service_name (str): Name of the service.
            rec_types (str): Record types being queried.
            timeout (float): Maximum time in seconds to wait.

        Raises:
            TimeoutError: If no update occurs within the given timeout.

        Returns:
            None
        """
        try:
            logger.info(f"Service record information lookup {rec_types} for '{service_name}' in progress...")
            await wait_for(self.updated_event.wait(), timeout)
        except TimeoutError:
            logger.info(
                f"Service record information lookup {rec_types} for '{service_name}' timeout ({timeout} seconds) reached without an update."
            )
            raise
