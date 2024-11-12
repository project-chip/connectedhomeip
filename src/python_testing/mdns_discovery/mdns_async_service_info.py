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
from typing import TYPE_CHECKING, Optional

from zeroconf import (DNSOutgoing, DNSQuestion, DNSQuestionType, Zeroconf, current_time_millis)
from zeroconf.const import (_CLASS_IN, _DUPLICATE_QUESTION_INTERVAL, _FLAGS_QR_QUERY,
                            _MDNS_PORT, _TYPE_A, _TYPE_AAAA, _TYPE_SRV, _TYPE_TXT)


# Import from zeroconf blows up, import from clone class works :\
# ▼ ▼ ▼ ▼ ▼

# from zeroconf import ServiceInfo
from mdns_discovery.service_info_base import ServiceInfo


int_ = int
float_ = float
str_ = str


QU_QUESTION = DNSQuestionType.QU
QM_QUESTION = DNSQuestionType.QM


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
    def __init__(self, zc: 'Zeroconf', *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._zc = zc

    async def async_request(
        self,
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
        if not self._zc.started:
            await self._zc.async_wait_for_start()

        now = current_time_millis()

        if TYPE_CHECKING:
            assert self._zc.loop is not None

        first_request = True
        delay = self._get_initial_delay()
        next_ = now
        last = now + timeout
        try:
            self._zc.async_add_listener(self, None)
            while not self._is_complete:
                if last <= now:
                    return False
                if next_ <= now:
                    this_question_type = question_type or QU_QUESTION if first_request else QM_QUESTION
                    out: DNSOutgoing = self._generate_request_query(this_question_type, record_type)
                    first_request = False
                    if out.questions:
                        # All questions may have been suppressed
                        # by the question history, so nothing to send,
                        # but keep waiting for answers in case another
                        # client on the network is asking the same
                        # question or they have not arrived yet.
                        self._zc.async_send(out, addr, port)
                    next_ = now + delay
                    next_ += self._get_random_delay()
                    if this_question_type is QM_QUESTION and delay < _DUPLICATE_QUESTION_INTERVAL:
                        # If we just asked a QM question, we need to
                        # wait at least the duplicate question interval
                        # before asking another QM question otherwise
                        # its likely to be suppressed by the question
                        # history of the remote responder.
                        delay = _DUPLICATE_QUESTION_INTERVAL

                await self.async_wait(min(next_, last) - now, self._zc.loop)
                now = current_time_millis()
        finally:
            self._zc.async_remove_listener(self)

        return True

    def _generate_request_query(
        self, question_type: DNSQuestionType, record_type: DNSRecordType
    ) -> DNSOutgoing:
        """Generate the request query."""
        out = DNSOutgoing(_FLAGS_QR_QUERY)
        name = self._name
        server = self.server or name
        qu_question = question_type is QU_QUESTION
        if record_type is None or record_type is DNSRecordType.SRV:
            print("Requesting MDNS SRV record...")
            self._add_question(
                out, qu_question, server, _TYPE_SRV, _CLASS_IN
            )
        if record_type is None or record_type is DNSRecordType.TXT:
            print("Requesting MDNS TXT record...")
            self._add_question(
                out, qu_question, server, _TYPE_TXT, _CLASS_IN
            )
        if record_type is None or record_type is DNSRecordType.A:
            print("Requesting MDNS A record...")
            self._add_question(
                out, qu_question, server, _TYPE_A, _CLASS_IN
            )
        if record_type is None or record_type is DNSRecordType.AAAA:
            print("Requesting MDNS AAAA record...")
            self._add_question(
                out, qu_question, server, _TYPE_AAAA, _CLASS_IN
            )
        return out

    def _add_question(
        self,
        out: DNSOutgoing,
        qu_question: bool,
        name: str_,
        type_: int_,
        class_: int_
    ) -> None:
        """Add a question."""
        question = DNSQuestion(name, type_, class_)
        question.unicast = qu_question
        out.add_question(question)
