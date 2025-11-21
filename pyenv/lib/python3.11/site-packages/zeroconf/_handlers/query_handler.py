"""Multicast DNS Service Discovery for Python, v0.14-wmcbrine
Copyright 2003 Paul Scott-Murphy, 2014 William McBrine

This module provides a framework for the use of DNS Service Discovery
using IP multicast.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
USA
"""

from __future__ import annotations

from typing import TYPE_CHECKING, cast

from .._cache import DNSCache, _UniqueRecordsType
from .._dns import DNSAddress, DNSPointer, DNSQuestion, DNSRecord, DNSRRSet
from .._protocol.incoming import DNSIncoming
from .._services.info import ServiceInfo
from .._transport import _WrappedTransport
from .._utils.net import IPVersion
from ..const import (
    _ADDRESS_RECORD_TYPES,
    _CLASS_IN,
    _DNS_OTHER_TTL,
    _MDNS_PORT,
    _ONE_SECOND,
    _SERVICE_TYPE_ENUMERATION_NAME,
    _TYPE_A,
    _TYPE_AAAA,
    _TYPE_ANY,
    _TYPE_NSEC,
    _TYPE_PTR,
    _TYPE_SRV,
    _TYPE_TXT,
)
from .answers import (
    QuestionAnswers,
    _AnswerWithAdditionalsType,
    construct_outgoing_multicast_answers,
    construct_outgoing_unicast_answers,
)

_RESPOND_IMMEDIATE_TYPES = {_TYPE_NSEC, _TYPE_SRV, *_ADDRESS_RECORD_TYPES}

_EMPTY_SERVICES_LIST: list[ServiceInfo] = []
_EMPTY_TYPES_LIST: list[str] = []

_IPVersion_ALL = IPVersion.All

_int = int
_str = str

_ANSWER_STRATEGY_SERVICE_TYPE_ENUMERATION = 0
_ANSWER_STRATEGY_POINTER = 1
_ANSWER_STRATEGY_ADDRESS = 2
_ANSWER_STRATEGY_SERVICE = 3
_ANSWER_STRATEGY_TEXT = 4

if TYPE_CHECKING:
    from .._core import Zeroconf


class _AnswerStrategy:
    __slots__ = ("question", "services", "strategy_type", "types")

    def __init__(
        self,
        question: DNSQuestion,
        strategy_type: _int,
        types: list[str],
        services: list[ServiceInfo],
    ) -> None:
        """Create an answer strategy."""
        self.question = question
        self.strategy_type = strategy_type
        self.types = types
        self.services = services


class _QueryResponse:
    """A pair for unicast and multicast DNSOutgoing responses."""

    __slots__ = (
        "_additionals",
        "_cache",
        "_is_probe",
        "_mcast_aggregate",
        "_mcast_aggregate_last_second",
        "_mcast_now",
        "_now",
        "_questions",
        "_ucast",
    )

    def __init__(self, cache: DNSCache, questions: list[DNSQuestion], is_probe: bool, now: float) -> None:
        """Build a query response."""
        self._is_probe = is_probe
        self._questions = questions
        self._now = now
        self._cache = cache
        self._additionals: _AnswerWithAdditionalsType = {}
        self._ucast: set[DNSRecord] = set()
        self._mcast_now: set[DNSRecord] = set()
        self._mcast_aggregate: set[DNSRecord] = set()
        self._mcast_aggregate_last_second: set[DNSRecord] = set()

    def add_qu_question_response(self, answers: _AnswerWithAdditionalsType) -> None:
        """Generate a response to a multicast QU query."""
        for record, additionals in answers.items():
            self._additionals[record] = additionals
            if self._is_probe:
                self._ucast.add(record)
            if not self._has_mcast_within_one_quarter_ttl(record):
                self._mcast_now.add(record)
            elif not self._is_probe:
                self._ucast.add(record)

    def add_ucast_question_response(self, answers: _AnswerWithAdditionalsType) -> None:
        """Generate a response to a unicast query."""
        self._additionals.update(answers)
        self._ucast.update(answers)

    def add_mcast_question_response(self, answers: _AnswerWithAdditionalsType) -> None:
        """Generate a response to a multicast query."""
        self._additionals.update(answers)
        for answer in answers:
            if self._is_probe:
                self._mcast_now.add(answer)
                continue

            if self._has_mcast_record_in_last_second(answer):
                self._mcast_aggregate_last_second.add(answer)
                continue

            if len(self._questions) == 1:
                question = self._questions[0]
                if question.type in _RESPOND_IMMEDIATE_TYPES:
                    self._mcast_now.add(answer)
                    continue

            self._mcast_aggregate.add(answer)

    def answers(
        self,
    ) -> QuestionAnswers:
        """Return answer sets that will be queued."""
        ucast = {r: self._additionals[r] for r in self._ucast}
        mcast_now = {r: self._additionals[r] for r in self._mcast_now}
        mcast_aggregate = {r: self._additionals[r] for r in self._mcast_aggregate}
        mcast_aggregate_last_second = {r: self._additionals[r] for r in self._mcast_aggregate_last_second}
        return QuestionAnswers(ucast, mcast_now, mcast_aggregate, mcast_aggregate_last_second)

    def _has_mcast_within_one_quarter_ttl(self, record: DNSRecord) -> bool:
        """Check to see if a record has been mcasted recently.

        https://datatracker.ietf.org/doc/html/rfc6762#section-5.4
        When receiving a question with the unicast-response bit set, a
        responder SHOULD usually respond with a unicast packet directed back
        to the querier.  However, if the responder has not multicast that
        record recently (within one quarter of its TTL), then the responder
        SHOULD instead multicast the response so as to keep all the peer
        caches up to date
        """
        if TYPE_CHECKING:
            record = cast(_UniqueRecordsType, record)
        maybe_entry = self._cache.async_get_unique(record)
        return bool(maybe_entry is not None and maybe_entry.is_recent(self._now))

    def _has_mcast_record_in_last_second(self, record: DNSRecord) -> bool:
        """Check if an answer was seen in the last second.
        Protect the network against excessive packet flooding
        https://datatracker.ietf.org/doc/html/rfc6762#section-14
        """
        if TYPE_CHECKING:
            record = cast(_UniqueRecordsType, record)
        maybe_entry = self._cache.async_get_unique(record)
        return bool(maybe_entry is not None and self._now - maybe_entry.created < _ONE_SECOND)


class QueryHandler:
    """Query the ServiceRegistry."""

    __slots__ = (
        "cache",
        "out_delay_queue",
        "out_queue",
        "question_history",
        "registry",
        "zc",
    )

    def __init__(self, zc: Zeroconf) -> None:
        """Init the query handler."""
        self.zc = zc
        self.registry = zc.registry
        self.cache = zc.cache
        self.question_history = zc.question_history
        self.out_queue = zc.out_queue
        self.out_delay_queue = zc.out_delay_queue

    def _add_service_type_enumeration_query_answers(
        self,
        types: list[str],
        answer_set: _AnswerWithAdditionalsType,
        known_answers: DNSRRSet,
    ) -> None:
        """Provide an answer to a service type enumeration query.

        https://datatracker.ietf.org/doc/html/rfc6763#section-9
        """
        for stype in types:
            dns_pointer = DNSPointer(
                _SERVICE_TYPE_ENUMERATION_NAME,
                _TYPE_PTR,
                _CLASS_IN,
                _DNS_OTHER_TTL,
                stype,
                0.0,
            )
            if not known_answers.suppresses(dns_pointer):
                answer_set[dns_pointer] = set()

    def _add_pointer_answers(
        self,
        services: list[ServiceInfo],
        answer_set: _AnswerWithAdditionalsType,
        known_answers: DNSRRSet,
    ) -> None:
        """Answer PTR/ANY question."""
        for service in services:
            # Add recommended additional answers according to
            # https://tools.ietf.org/html/rfc6763#section-12.1.
            dns_pointer = service._dns_pointer(None)
            if known_answers.suppresses(dns_pointer):
                continue
            answer_set[dns_pointer] = {
                service._dns_service(None),
                service._dns_text(None),
                *service._get_address_and_nsec_records(None),
            }

    def _add_address_answers(
        self,
        services: list[ServiceInfo],
        answer_set: _AnswerWithAdditionalsType,
        known_answers: DNSRRSet,
        type_: _int,
    ) -> None:
        """Answer A/AAAA/ANY question."""
        for service in services:
            answers: list[DNSAddress] = []
            additionals: set[DNSRecord] = set()
            seen_types: set[int] = set()
            for dns_address in service._dns_addresses(None, _IPVersion_ALL):
                seen_types.add(dns_address.type)
                if dns_address.type != type_:
                    additionals.add(dns_address)
                elif not known_answers.suppresses(dns_address):
                    answers.append(dns_address)
            missing_types: set[int] = _ADDRESS_RECORD_TYPES - seen_types
            if answers:
                if missing_types:
                    assert service.server is not None, "Service server must be set for NSEC record."
                    additionals.add(service._dns_nsec(list(missing_types), None))
                for answer in answers:
                    answer_set[answer] = additionals
            elif type_ in missing_types:
                assert service.server is not None, "Service server must be set for NSEC record."
                answer_set[service._dns_nsec(list(missing_types), None)] = set()

    def _answer_question(
        self,
        question: DNSQuestion,
        strategy_type: _int,
        types: list[str],
        services: list[ServiceInfo],
        known_answers: DNSRRSet,
    ) -> _AnswerWithAdditionalsType:
        """Answer a question."""
        answer_set: _AnswerWithAdditionalsType = {}

        if strategy_type == _ANSWER_STRATEGY_SERVICE_TYPE_ENUMERATION:
            self._add_service_type_enumeration_query_answers(types, answer_set, known_answers)
        elif strategy_type == _ANSWER_STRATEGY_POINTER:
            self._add_pointer_answers(services, answer_set, known_answers)
        elif strategy_type == _ANSWER_STRATEGY_ADDRESS:
            self._add_address_answers(services, answer_set, known_answers, question.type)
        elif strategy_type == _ANSWER_STRATEGY_SERVICE:
            # Add recommended additional answers according to
            # https://tools.ietf.org/html/rfc6763#section-12.2.
            service = services[0]
            dns_service = service._dns_service(None)
            if not known_answers.suppresses(dns_service):
                answer_set[dns_service] = service._get_address_and_nsec_records(None)
        elif strategy_type == _ANSWER_STRATEGY_TEXT:  # pragma: no branch
            service = services[0]
            dns_text = service._dns_text(None)
            if not known_answers.suppresses(dns_text):
                answer_set[dns_text] = set()

        return answer_set

    def async_response(  # pylint: disable=unused-argument
        self, msgs: list[DNSIncoming], ucast_source: bool
    ) -> QuestionAnswers | None:
        """Deal with incoming query packets. Provides a response if possible.

        This function must be run in the event loop as it is not
        threadsafe.
        """
        strategies: list[_AnswerStrategy] = []
        for msg in msgs:
            for question in msg._questions:
                strategies.extend(self._get_answer_strategies(question))

        if not strategies:
            # We have no way to answer the question because we have
            # nothing in the ServiceRegistry that matches or we do not
            # understand the question.
            return None

        is_probe = False
        msg = msgs[0]
        questions = msg._questions
        # Only decode known answers if we are not a probe and we have
        # at least one answer strategy
        answers: list[DNSRecord] = []
        for msg in msgs:
            if msg.is_probe():
                is_probe = True
            else:
                answers.extend(msg.answers())

        query_res = _QueryResponse(self.cache, questions, is_probe, msg.now)
        known_answers = DNSRRSet(answers)
        known_answers_set: set[DNSRecord] | None = None
        now = msg.now
        for strategy in strategies:
            question = strategy.question
            is_unicast = question.unique  # unique and unicast are the same flag
            if not is_unicast:
                if known_answers_set is None:  # pragma: no branch
                    known_answers_set = known_answers.lookup_set()
                self.question_history.add_question_at_time(question, now, known_answers_set)
            answer_set = self._answer_question(
                question,
                strategy.strategy_type,
                strategy.types,
                strategy.services,
                known_answers,
            )
            if not ucast_source and is_unicast:
                query_res.add_qu_question_response(answer_set)
                continue
            if ucast_source:
                query_res.add_ucast_question_response(answer_set)
            # We always multicast as well even if its a unicast
            # source as long as we haven't done it recently (75% of ttl)
            query_res.add_mcast_question_response(answer_set)

        return query_res.answers()

    def _get_answer_strategies(
        self,
        question: DNSQuestion,
    ) -> list[_AnswerStrategy]:
        """Collect strategies to answer a question."""
        name = question.name
        question_lower_name = name.lower()
        type_ = question.type
        strategies: list[_AnswerStrategy] = []

        if type_ == _TYPE_PTR and question_lower_name == _SERVICE_TYPE_ENUMERATION_NAME:
            types = self.registry.async_get_types()
            if types:
                strategies.append(
                    _AnswerStrategy(
                        question,
                        _ANSWER_STRATEGY_SERVICE_TYPE_ENUMERATION,
                        types,
                        _EMPTY_SERVICES_LIST,
                    )
                )
            return strategies

        if type_ in (_TYPE_PTR, _TYPE_ANY):
            services = self.registry.async_get_infos_type(question_lower_name)
            if services:
                strategies.append(
                    _AnswerStrategy(question, _ANSWER_STRATEGY_POINTER, _EMPTY_TYPES_LIST, services)
                )

        if type_ in (_TYPE_A, _TYPE_AAAA, _TYPE_ANY):
            services = self.registry.async_get_infos_server(question_lower_name)
            if services:
                strategies.append(
                    _AnswerStrategy(question, _ANSWER_STRATEGY_ADDRESS, _EMPTY_TYPES_LIST, services)
                )

        if type_ in (_TYPE_SRV, _TYPE_TXT, _TYPE_ANY):
            service = self.registry.async_get_info_name(question_lower_name)
            if service is not None:
                if type_ in (_TYPE_SRV, _TYPE_ANY):
                    strategies.append(
                        _AnswerStrategy(
                            question,
                            _ANSWER_STRATEGY_SERVICE,
                            _EMPTY_TYPES_LIST,
                            [service],
                        )
                    )
                if type_ in (_TYPE_TXT, _TYPE_ANY):
                    strategies.append(
                        _AnswerStrategy(
                            question,
                            _ANSWER_STRATEGY_TEXT,
                            _EMPTY_TYPES_LIST,
                            [service],
                        )
                    )

        return strategies

    def handle_assembled_query(
        self,
        packets: list[DNSIncoming],
        addr: _str,
        port: _int,
        transport: _WrappedTransport,
        v6_flow_scope: tuple[()] | tuple[int, int],
    ) -> None:
        """Respond to a (re)assembled query.

        If the protocol received packets with the TC bit set, it will
        wait a bit for the rest of the packets and only call
        handle_assembled_query once it has a complete set of packets
        or the timer expires. If the TC bit is not set, a single
        packet will be in packets.
        """
        first_packet = packets[0]
        ucast_source = port != _MDNS_PORT
        question_answers = self.async_response(packets, ucast_source)
        if question_answers is None:
            return
        if question_answers.ucast:
            questions = first_packet._questions
            id_ = first_packet.id
            out = construct_outgoing_unicast_answers(question_answers.ucast, ucast_source, questions, id_)
            # When sending unicast, only send back the reply
            # via the same socket that it was received from
            # as we know its reachable from that socket
            self.zc.async_send(out, addr, port, v6_flow_scope, transport)
        if question_answers.mcast_now:
            self.zc.async_send(construct_outgoing_multicast_answers(question_answers.mcast_now))
        if question_answers.mcast_aggregate:
            self.out_queue.async_add(first_packet.now, question_answers.mcast_aggregate)
        if question_answers.mcast_aggregate_last_second:
            # https://datatracker.ietf.org/doc/html/rfc6762#section-14
            # If we broadcast it in the last second, we have to delay
            # at least a second before we send it again
            self.out_delay_queue.async_add(first_packet.now, question_answers.mcast_aggregate_last_second)
