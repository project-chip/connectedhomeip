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

from random import randint
from typing import TYPE_CHECKING

from zeroconf import (BadTypeInNameException, DNSOutgoing, DNSQuestion, DNSQuestionType, ServiceInfo, Zeroconf, current_time_millis,
                      service_type_name)
from zeroconf._services.info import float_
from zeroconf.const import _CLASS_IN, _DUPLICATE_QUESTION_INTERVAL, _FLAGS_QR_QUERY, _LISTENER_TIME, _MDNS_PORT, _TYPE_AAAA

_AVOID_SYNC_DELAY_RANDOM_INTERVAL = (20, 120)


class MdnsAsyncServiceInfo(ServiceInfo):
    def __init__(self, name: str | None = None, type_: str | None = None, server: str | None = None) -> None:
        if server and not (name and type_):
            # Use server-only mode (for address resolution)
            super().__init__(type_="", name="", server=server)
            self._name = None
            self.type = None
            self.server = server
            return

        if not type_.endswith(service_type_name(name, strict=False)):
            raise BadTypeInNameException

        super().__init__(type_=type_, name=name, server=server)
        self._name = name
        self.type = type_
        self.server = server

    async def async_request(
        self,
        zc: Zeroconf,
        timeout: float,
        question_type: DNSQuestionType | None = None,
        addr: str | None = None,
        port: int = _MDNS_PORT,
    ) -> bool:
        """Returns true if the service could be discovered on the
        network, and updates this object with details discovered.

        This method will be run in the event loop.

        Passing addr and port is optional, and will default to the
        mDNS multicast address and port. This is useful for directing
        requests to a specific host that may be able to respond across
        subnets.

        This override of **zeroconf.ServiceInfo.async_request** disables
        known-answer caching and clears the cache to ensure a fresh
        response each time. Unlike the original implementation, which
        sends all questions at once, this version sends one question
        type at a time (e.g., SRV, TXT, AAAA).
        """
        if not zc.started:
            await zc.async_wait_for_start()

        now = current_time_millis()

        if TYPE_CHECKING:
            assert zc.loop is not None

        first_request = True
        delay = self._get_initial_delay()
        next_ = now
        last = now + timeout
        record_type = next(iter(self._query_record_types))
        try:
            self.async_clear_cache()
            zc.async_add_listener(self, None)
            while not self._is_complete:
                if last <= now:
                    return False
                if next_ <= now:
                    this_question_type = question_type or DNSQuestionType.QU if first_request else DNSQuestionType.QM
                    first_request = False

                    # Prepare outgoing mDNS query message with
                    # a single question and record type.
                    out = DNSOutgoing(_FLAGS_QR_QUERY)
                    question = DNSQuestion(self._name, record_type, _CLASS_IN)
                    question.unicast = question_type is DNSQuestionType.QU
                    out.add_question(question)

                    # Send the query to the specified address and port.
                    zc.async_send(out, addr, port)

                    next_ = now + delay
                    next_ += randint(*_AVOID_SYNC_DELAY_RANDOM_INTERVAL)

                    if this_question_type is DNSQuestionType.QM and delay < _DUPLICATE_QUESTION_INTERVAL:
                        delay = _DUPLICATE_QUESTION_INTERVAL

                await self.async_wait(min(next_, last) - now, zc.loop)
                now = current_time_millis()
        finally:
            zc.async_remove_listener(self)

        return True

    def _get_initial_delay(self) -> float_:
        return _LISTENER_TIME


class AddressResolverIPv6(MdnsAsyncServiceInfo):
    """Resolve a host name to an IPv6 address."""

    def __init__(self, server: str) -> None:
        """Initialize the AddressResolver."""
        super().__init__(server, server, server=server)
        self._query_record_types = {_TYPE_AAAA}

    @property
    def _is_complete(self) -> bool:
        """The ServiceInfo has all expected properties."""
        return bool(self._ipv6_addresses)
