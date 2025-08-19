#
#    Copyright (c) 2024 Project CHIP Authors
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


import enum
from random import randint
from typing import TYPE_CHECKING, Optional

from zeroconf import (BadTypeInNameException, DNSOutgoing, DNSQuestion, DNSQuestionType, ServiceInfo, Zeroconf, current_time_millis,
                      service_type_name)
from zeroconf.const import (_CLASS_IN, _DUPLICATE_QUESTION_INTERVAL, _FLAGS_QR_QUERY, _LISTENER_TIME, _MDNS_PORT, _TYPE_A,
                            _TYPE_AAAA, _TYPE_SRV, _TYPE_TXT)

_AVOID_SYNC_DELAY_RANDOM_INTERVAL = (20, 120)


@enum.unique
class DNSRecordType(enum.Enum):
    """An MDNS record type.

    "A" - A MDNS record type
    "AAAA" - AAAA MDNS record type
    "SRV" - SRV MDNS record type
    "TXT" - TXT MDNS record type
    """

    A = 0
    AAAA = 1
    SRV = 2
    TXT = 3


class MdnsAsyncServiceInfo(ServiceInfo):
    def __init__(self, name: str, type_: str) -> None:
        super().__init__(type_=type_, name=name)

        if type_ and not type_.endswith(service_type_name(name, strict=False)):
            raise BadTypeInNameException

        self._name = name
        self.type = type_
        self.key = name.lower()

    async def async_request(
        self,
        zc: Zeroconf,
        timeout: float,
        question_type: Optional[DNSQuestionType] = None,
        addr: Optional[str] = None,
        port: int = _MDNS_PORT,
        record_type: DNSRecordType = None
    ) -> bool:
        """Returns true if the service could be discovered on the
        network, and updates this object with details discovered.

        This method will be run in the event loop.

        Passing addr and port is optional, and will default to the
        mDNS multicast address and port. This is useful for directing
        requests to a specific host that may be able to respond across
        subnets.
        """
        if not zc.started:
            await zc.async_wait_for_start()

        now = current_time_millis()

        if TYPE_CHECKING:
            assert zc.loop is not None

        first_request = True
        delay = _LISTENER_TIME
        next_ = now
        last = now + timeout
        try:
            zc.async_add_listener(self, None)
            while not self._is_complete:
                if last <= now:
                    return False
                if next_ <= now:
                    this_question_type = question_type or DNSQuestionType.QU if first_request else DNSQuestionType.QM
                    out: DNSOutgoing = self._generate_request_query(this_question_type, record_type)
                    first_request = False

                    if out.questions:
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

    def _generate_request_query(self, question_type: DNSQuestionType, record_type: DNSRecordType) -> DNSOutgoing:
        """Generate the request query."""
        out = DNSOutgoing(_FLAGS_QR_QUERY)
        name = self._name
        qu_question = question_type is DNSQuestionType.QU

        record_types = {
            DNSRecordType.SRV: (_TYPE_SRV, "Requesting MDNS SRV record..."),
            DNSRecordType.TXT: (_TYPE_TXT, "Requesting MDNS TXT record..."),
            DNSRecordType.A: (_TYPE_A, "Requesting MDNS A record..."),
            DNSRecordType.AAAA: (_TYPE_AAAA, "Requesting MDNS AAAA record..."),
        }

        # Iterate over record types, add question uppon match
        for r_type, (type_const, message) in record_types.items():
            if record_type is None or record_type == r_type:
                print(message)
                question = DNSQuestion(name, type_const, _CLASS_IN)
                question.unicast = qu_question
                out.add_question(question)

        return out
