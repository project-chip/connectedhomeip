#
#    Copyright (c) 2024-2025 Project CHIP Authors
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

from random import randint
from typing import TYPE_CHECKING, Optional

from zeroconf import (BadTypeInNameException, DNSOutgoing, DNSQuestion, DNSQuestionType, ServiceInfo, Zeroconf, current_time_millis,
                      service_type_name)
from zeroconf.const import _CLASS_IN, _DUPLICATE_QUESTION_INTERVAL, _FLAGS_QR_QUERY, _LISTENER_TIME, _MDNS_PORT, _TYPE_AAAA

_LISTENER_TIME_MS = _LISTENER_TIME


class MdnsAsyncServiceInfo(ServiceInfo):
    """
    This subclass of **zeroconf.ServiceInfo** enables AAAA address
    only resolution or full service info queries. It also overrides
    the **async_request** method to disable caching.
    """

    def __init__(self,
                 name: str | None = None,  # Fully qualified service name
                 type_: str | None = None,  # Fully qualified service type name
                 server: str | None = None  # Fully qualified name for service host (defaults to name)
                 ) -> None:

        # For AAAA address resolution, only server (hostname)
        # is to be provided, this configuration is set by the
        # AddressResolverIPv6 class which inherits from this class
        if server and not (name and type_):
            super().__init__(type_=server, name=server, server=server)
            return

        # Validate a fully qualified service name, instance or subtype
        if not type_.endswith(service_type_name(name, strict=False)):
            raise BadTypeInNameException

        super().__init__(type_=type_, name=name, server=server)

    async def async_request(
        self,
        zc: Zeroconf,
        timeout_ms: float,
        question_type: Optional[DNSQuestionType] = None,
        addr: str | None = None,
        port: int = _MDNS_PORT,
    ) -> bool:
        """Returns true if the service could be discovered on the network, and updates this
        object with details discovered.

        Custom override of `zeroconf.ServiceInfo.async_request` that prioritizes fresh data.

        Key differences from the base implementation:
        - Bypasses known-answer caching to force a network query on every call.
        - Clears the internal cache before querying to prevent stale results.
        - Allows filtering by specific DNS record types (SRV, TXT, A, AAAA, etc.).
        """
        if not zc.started:
            await zc.async_wait_for_start()
        if TYPE_CHECKING:
            assert zc.loop is not None

        # Force fresh queries by clearing cache and question history
        zc.cache.cache.clear()
        zc.question_history.clear()
        self.async_clear_cache()

        now_ms = current_time_millis()

        # Absolute cutoff time after which the request stops if incomplete
        deadline_ms = now_ms + timeout_ms

        # Delay before sending the first retry query if the initial query did not complete
        initial_delay_ms = _LISTENER_TIME_MS + randint(0, 50)

        # Minimum delay between subsequent QM retries after the first retry,
        # to prevent duplicate-question suppression by responding devices
        duplicate_interval_ms = _DUPLICATE_QUESTION_INTERVAL

        # Linger after completion to catch late SRV/TXT/AAAA/A responses
        # Most devices send these within <200 ms, but we allow extra headroom
        linger_after_complete_ms = randint(300, 500)

        first_send = True
        next_send = now_ms

        # Build an outgoing DNS query for the requested record types
        def build_outgoing(as_qu: bool) -> DNSOutgoing:
            out = DNSOutgoing(_FLAGS_QR_QUERY)
            for rtype in self._query_record_types:
                q = DNSQuestion(self.name, rtype, _CLASS_IN)
                q.unicast = as_qu
                out.add_question(q)
            return out

        try:
            zc.async_add_listener(self, None)
            use_qu_first = (question_type in (None, DNSQuestionType.QU))

            while not self._is_complete:
                now_ms = current_time_millis()

                # Deadline reached before record was found
                if now_ms >= deadline_ms:
                    return False

                if now_ms >= next_send:
                    # If this is the first cycle and QU is allowed, send a unicast-preferred query first.
                    if use_qu_first:
                        zc.async_send(build_outgoing(as_qu=True), addr, port)
                        use_qu_first = False
                    # Always send the multicast (QM) query to reach all possible responders.
                    zc.async_send(build_outgoing(as_qu=False), addr, port)

                if first_send:
                    # First retry: wait the shorter initial delay before sending again.
                    next_send = now_ms + initial_delay_ms
                    first_send = False
                else:
                    # Subsequent retries: wait at least the duplicate-question interval
                    # to avoid suppression by responders, plus a small random offset.
                    next_send = now_ms + max(duplicate_interval_ms, initial_delay_ms + randint(0, 100))

                # Sleep until the next scheduled send time or the overall deadline,
                # whichever occurs first. Clamp to 0 to avoid negative waits.
                await self.async_wait(max(0, min(next_send, deadline_ms) - now_ms), zc.loop)

            # After exiting the query loop, wait briefly to passively
            # catch any late TXT/SRV/AAAA/A responses.
            remaining = max(0, min(linger_after_complete_ms, (deadline_ms - current_time_millis())))
            if remaining:
                await self.async_wait(remaining, zc.loop)

            return True

        finally:
            zc.async_remove_listener(self)


class AddressResolverIPv6(MdnsAsyncServiceInfo):
    """Resolve a host name to an IPv6 address."""

    def __init__(self, hostname: str) -> None:
        """Initialize the AddressResolver."""
        super().__init__(server=hostname)
        self._query_record_types = {_TYPE_AAAA}

    @property
    def _is_complete(self) -> bool:
        """The ServiceInfo has all expected properties."""
        return bool(self._ipv6_addresses)
