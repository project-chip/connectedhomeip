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


import asyncio
import enum
from ipaddress import IPv4Address, IPv6Address
from random import randint
from typing import TYPE_CHECKING, Dict, List, Optional, Set, Union, cast

from zeroconf import (BadTypeInNameException, DNSAddress, DNSOutgoing, DNSPointer, DNSQuestion, DNSQuestionType,
                      DNSRecord, DNSService, DNSText, ServiceInfo, Zeroconf, current_time_millis, service_type_name)
from zeroconf._utils.net import _encode_address
from zeroconf.const import (_CLASS_IN, _DNS_HOST_TTL, _DNS_OTHER_TTL, _DUPLICATE_QUESTION_INTERVAL, _FLAGS_QR_QUERY, _LISTENER_TIME,
                            _MDNS_PORT, _TYPE_A, _TYPE_AAAA, _TYPE_SRV, _TYPE_TXT)

int_ = int
float_ = float
str_ = str


QU_QUESTION = DNSQuestionType.QU
QM_QUESTION = DNSQuestionType.QM
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
    def __init__(
        self,
        type_: str,
        name: str,
        port: Optional[int] = None,
        weight: int = 0,
        priority: int = 0,
        properties: Union[bytes, Dict] = b'',
        server: Optional[str] = None,
        host_ttl: int = _DNS_HOST_TTL,
        other_ttl: int = _DNS_OTHER_TTL,
        *,
        addresses: Optional[List[bytes]] = None,
        parsed_addresses: Optional[List[str]] = None,
        interface_index: Optional[int] = None,
    ) -> None:
        # Accept both none, or one, but not both.
        if addresses is not None and parsed_addresses is not None:
            raise TypeError("addresses and parsed_addresses cannot be provided together")
        if not type_.endswith(service_type_name(name, strict=False)):
            raise BadTypeInNameException
        self.interface_index = interface_index
        self.text = b''
        self.type = type_
        self._name = name
        self.key = name.lower()
        self._ipv4_addresses: List[IPv4Address] = []
        self._ipv6_addresses: List[IPv6Address] = []
        if addresses is not None:
            self.addresses = addresses
        elif parsed_addresses is not None:
            self.addresses = [_encode_address(a) for a in parsed_addresses]
        self.port = port
        self.weight = weight
        self.priority = priority
        self.server = server if server else None
        self.server_key = server.lower() if server else None
        self._properties: Optional[Dict[bytes, Optional[bytes]]] = None
        self._decoded_properties: Optional[Dict[str, Optional[str]]] = None
        if isinstance(properties, bytes):
            self._set_text(properties)
        else:
            self._set_properties(properties)
        self.host_ttl = host_ttl
        self.other_ttl = other_ttl
        self._new_records_futures: Optional[Set[asyncio.Future]] = None
        self._dns_address_cache: Optional[List[DNSAddress]] = None
        self._dns_pointer_cache: Optional[DNSPointer] = None
        self._dns_service_cache: Optional[DNSService] = None
        self._dns_text_cache: Optional[DNSText] = None
        self._get_address_and_nsec_records_cache: Optional[Set[DNSRecord]] = None

    async def async_request(
        self,
        zc: 'Zeroconf',
        timeout: float,
        question_type: Optional[DNSQuestionType] = None,
        addr: Optional[str] = None,
        port: int = _MDNS_PORT,
        record_type: DNSRecordType = None,
        load_from_cache: bool = True
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

        if load_from_cache:
            if self._load_from_cache(zc, now):
                return True

        if TYPE_CHECKING:
            assert zc.loop is not None

        first_request = True
        delay = self._get_initial_delay()
        next_ = now
        last = now + timeout
        try:
            zc.async_add_listener(self, None)
            while not self._is_complete:
                if last <= now:
                    return False
                if next_ <= now:
                    this_question_type = question_type or QU_QUESTION if first_request else QM_QUESTION
                    out = self._generate_request_query(zc, now, this_question_type, record_type)
                    first_request = False
                    if out.questions:
                        # All questions may have been suppressed
                        # by the question history, so nothing to send,
                        # but keep waiting for answers in case another
                        # client on the network is asking the same
                        # question or they have not arrived yet.
                        zc.async_send(out, addr, port)
                    next_ = now + delay
                    next_ += self._get_random_delay()
                    if this_question_type is QM_QUESTION and delay < _DUPLICATE_QUESTION_INTERVAL:
                        # If we just asked a QM question, we need to
                        # wait at least the duplicate question interval
                        # before asking another QM question otherwise
                        # its likely to be suppressed by the question
                        # history of the remote responder.
                        delay = _DUPLICATE_QUESTION_INTERVAL

                await self.async_wait(min(next_, last) - now, zc.loop)
                now = current_time_millis()
        finally:
            zc.async_remove_listener(self)

        return True

    def _generate_request_query(
        self, zc: 'Zeroconf', now: float_, question_type: DNSQuestionType, record_type: DNSRecordType
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

    def _get_initial_delay(self) -> float_:
        return _LISTENER_TIME

    def _get_random_delay(self) -> int_:
        return randint(*_AVOID_SYNC_DELAY_RANDOM_INTERVAL)

    def _set_properties(self, properties: Dict[Union[str, bytes], Optional[Union[str, bytes]]]) -> None:
        """Sets properties and text of this info from a dictionary"""
        list_: List[bytes] = []
        properties_contain_str = False
        result = b''
        for key, value in properties.items():
            if isinstance(key, str):
                key = key.encode('utf-8')
                properties_contain_str = True

            record = key
            if value is not None:
                if not isinstance(value, bytes):
                    value = str(value).encode('utf-8')
                    properties_contain_str = True
                record += b'=' + value
            list_.append(record)
        for item in list_:
            result = b''.join((result, bytes((len(item),)), item))
        if not properties_contain_str:
            # If there are no str keys or values, we can use the properties
            # as-is, without decoding them, otherwise calling
            # self.properties will lazy decode them, which is expensive.
            if TYPE_CHECKING:
                self._properties = cast("Dict[bytes, Optional[bytes]]", properties)
            else:
                self._properties = properties
        self.text = result

    def _set_text(self, text: bytes) -> None:
        """Sets properties and text given a text field"""
        if text == self.text:
            return
        self.text = text
        # Clear the properties cache
        self._properties = None
        self._decoded_properties = None
