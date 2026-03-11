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

import asyncio
import heapq
import queue
import random
import threading
import time
import warnings
from collections.abc import Iterable
from functools import partial
from types import TracebackType  # used in type hints
from typing import (
    TYPE_CHECKING,
    Any,
    Callable,
    cast,
)

from .._dns import DNSPointer, DNSQuestion, DNSQuestionType
from .._logger import log
from .._protocol.outgoing import DNSOutgoing
from .._record_update import RecordUpdate
from .._services import (
    ServiceListener,
    ServiceStateChange,
    Signal,
    SignalRegistrationInterface,
)
from .._updates import RecordUpdateListener
from .._utils.name import cached_possible_types, service_type_name
from .._utils.time import current_time_millis, millis_to_seconds
from ..const import (
    _ADDRESS_RECORD_TYPES,
    _BROWSER_TIME,
    _CLASS_IN,
    _DNS_PACKET_HEADER_LEN,
    _EXPIRE_REFRESH_TIME_PERCENT,
    _FLAGS_QR_QUERY,
    _MAX_MSG_TYPICAL,
    _MDNS_ADDR,
    _MDNS_ADDR6,
    _MDNS_PORT,
    _TYPE_PTR,
)

# https://datatracker.ietf.org/doc/html/rfc6762#section-5.2
_FIRST_QUERY_DELAY_RANDOM_INTERVAL = (20, 120)  # ms

_ON_CHANGE_DISPATCH = {
    ServiceStateChange.Added: "add_service",
    ServiceStateChange.Removed: "remove_service",
    ServiceStateChange.Updated: "update_service",
}

SERVICE_STATE_CHANGE_ADDED = ServiceStateChange.Added
SERVICE_STATE_CHANGE_REMOVED = ServiceStateChange.Removed
SERVICE_STATE_CHANGE_UPDATED = ServiceStateChange.Updated

QU_QUESTION = DNSQuestionType.QU

STARTUP_QUERIES = 4

RESCUE_RECORD_RETRY_TTL_PERCENTAGE = 0.1

if TYPE_CHECKING:
    from .._core import Zeroconf

float_ = float
int_ = int
bool_ = bool
str_ = str

_QuestionWithKnownAnswers = dict[DNSQuestion, set[DNSPointer]]

heappop = heapq.heappop
heappush = heapq.heappush


class _ScheduledPTRQuery:  # noqa: PLW1641
    __slots__ = (
        "alias",
        "cancelled",
        "expire_time_millis",
        "name",
        "ttl",
        "when_millis",
    )

    def __init__(
        self,
        alias: str,
        name: str,
        ttl: int,
        expire_time_millis: float,
        when_millis: float,
    ) -> None:
        """Create a scheduled query."""
        self.alias = alias
        self.name = name
        self.ttl = ttl
        # Since queries are stored in a heap we need to track if they are cancelled
        # so we can remove them from the heap when they are cancelled as it would
        # be too expensive to search the heap for the record to remove and instead
        # we just mark it as cancelled and ignore it when we pop it off the heap
        # when the query is due.
        self.cancelled = False
        # Expire time millis is the actual millisecond time the record will expire
        self.expire_time_millis = expire_time_millis
        # When millis is the millisecond time the query should be sent
        # For the first query this is the refresh time which is 75% of the TTL
        #
        # For subsequent queries we increase the time by 10% of the TTL
        # until we reach the expire time and then we stop because it means
        # we failed to rescue the record.
        self.when_millis = when_millis

    def __repr__(self) -> str:
        """Return a string representation of the scheduled query."""
        return (
            f"<{self.__class__.__name__} "
            f"alias={self.alias} "
            f"name={self.name} "
            f"ttl={self.ttl} "
            f"cancelled={self.cancelled} "
            f"expire_time_millis={self.expire_time_millis} "
            f"when_millis={self.when_millis}"
            ">"
        )

    def __lt__(self, other: _ScheduledPTRQuery) -> bool:
        """Compare two scheduled queries."""
        if type(other) is _ScheduledPTRQuery:
            return self.when_millis < other.when_millis
        return NotImplemented

    def __le__(self, other: _ScheduledPTRQuery) -> bool:
        """Compare two scheduled queries."""
        if type(other) is _ScheduledPTRQuery:
            return self.when_millis < other.when_millis or self.__eq__(other)
        return NotImplemented

    def __eq__(self, other: Any) -> bool:
        """Compare two scheduled queries."""
        if type(other) is _ScheduledPTRQuery:
            return self.when_millis == other.when_millis
        return NotImplemented

    def __ge__(self, other: _ScheduledPTRQuery) -> bool:
        """Compare two scheduled queries."""
        if type(other) is _ScheduledPTRQuery:
            return self.when_millis > other.when_millis or self.__eq__(other)
        return NotImplemented

    def __gt__(self, other: _ScheduledPTRQuery) -> bool:
        """Compare two scheduled queries."""
        if type(other) is _ScheduledPTRQuery:
            return self.when_millis > other.when_millis
        return NotImplemented


class _DNSPointerOutgoingBucket:
    """A DNSOutgoing bucket."""

    __slots__ = ("bytes", "now_millis", "out")

    def __init__(self, now_millis: float, multicast: bool) -> None:
        """Create a bucket to wrap a DNSOutgoing."""
        self.now_millis = now_millis
        self.out = DNSOutgoing(_FLAGS_QR_QUERY, multicast)
        self.bytes = 0

    def add(self, max_compressed_size: int_, question: DNSQuestion, answers: set[DNSPointer]) -> None:
        """Add a new set of questions and known answers to the outgoing."""
        self.out.add_question(question)
        for answer in answers:
            self.out.add_answer_at_time(answer, self.now_millis)
        self.bytes += max_compressed_size


def group_ptr_queries_with_known_answers(
    now: float_,
    multicast: bool_,
    question_with_known_answers: _QuestionWithKnownAnswers,
) -> list[DNSOutgoing]:
    """Aggregate queries so that as many known answers as possible fit in the same packet
    without having known answers spill over into the next packet unless the
    question and known answers are always going to exceed the packet size.

    Some responders do not implement multi-packet known answer suppression
    so we try to keep all the known answers in the same packet as the
    questions.
    """
    return _group_ptr_queries_with_known_answers(now, multicast, question_with_known_answers)


def _group_ptr_queries_with_known_answers(
    now_millis: float_,
    multicast: bool_,
    question_with_known_answers: _QuestionWithKnownAnswers,
) -> list[DNSOutgoing]:
    """Inner wrapper for group_ptr_queries_with_known_answers."""
    # This is the maximum size the query + known answers can be with name compression.
    # The actual size of the query + known answers may be a bit smaller since other
    # parts may be shared when the final DNSOutgoing packets are constructed. The
    # goal of this algorithm is to quickly bucket the query + known answers without
    # the overhead of actually constructing the packets.
    query_by_size: dict[DNSQuestion, int] = {
        question: (question.max_size + sum(answer.max_size_compressed for answer in known_answers))
        for question, known_answers in question_with_known_answers.items()
    }
    max_bucket_size = _MAX_MSG_TYPICAL - _DNS_PACKET_HEADER_LEN
    query_buckets: list[_DNSPointerOutgoingBucket] = []
    for question in sorted(
        query_by_size,
        key=query_by_size.get,  # type: ignore
        reverse=True,
    ):
        max_compressed_size = query_by_size[question]
        answers = question_with_known_answers[question]
        for query_bucket in query_buckets:
            if query_bucket.bytes + max_compressed_size <= max_bucket_size:
                query_bucket.add(max_compressed_size, question, answers)
                break
        else:
            # If a single question and known answers won't fit in a packet
            # we will end up generating multiple packets, but there will never
            # be multiple questions
            query_bucket = _DNSPointerOutgoingBucket(now_millis, multicast)
            query_bucket.add(max_compressed_size, question, answers)
            query_buckets.append(query_bucket)

    return [query_bucket.out for query_bucket in query_buckets]


def generate_service_query(
    zc: Zeroconf,
    now_millis: float_,
    types_: set[str],
    multicast: bool,
    question_type: DNSQuestionType | None,
) -> list[DNSOutgoing]:
    """Generate a service query for sending with zeroconf.send."""
    questions_with_known_answers: _QuestionWithKnownAnswers = {}
    qu_question = not multicast if question_type is None else question_type is QU_QUESTION
    question_history = zc.question_history
    cache = zc.cache
    for type_ in types_:
        question = DNSQuestion(type_, _TYPE_PTR, _CLASS_IN)
        question.unicast = qu_question
        known_answers = {
            record
            for record in cache.get_all_by_details(type_, _TYPE_PTR, _CLASS_IN)
            if not record.is_stale(now_millis)
        }
        if not qu_question and question_history.suppresses(question, now_millis, known_answers):
            log.debug("Asking %s was suppressed by the question history", question)
            continue
        if TYPE_CHECKING:
            pointer_known_answers = cast(set[DNSPointer], known_answers)
        else:
            pointer_known_answers = known_answers
        questions_with_known_answers[question] = pointer_known_answers
        if not qu_question:
            question_history.add_question_at_time(question, now_millis, known_answers)

    return _group_ptr_queries_with_known_answers(now_millis, multicast, questions_with_known_answers)


def _on_change_dispatcher(
    listener: ServiceListener,
    zeroconf: Zeroconf,
    service_type: str,
    name: str,
    state_change: ServiceStateChange,
) -> None:
    """Dispatch a service state change to a listener."""
    getattr(listener, _ON_CHANGE_DISPATCH[state_change])(zeroconf, service_type, name)


def _service_state_changed_from_listener(
    listener: ServiceListener,
) -> Callable[..., None]:
    """Generate a service_state_changed handlers from a listener."""
    assert listener is not None
    if not hasattr(listener, "update_service"):
        warnings.warn(
            f"{listener!r} has no update_service method. Provide one (it can be empty if you "
            "don't care about the updates), it'll become mandatory.",
            FutureWarning,
            stacklevel=1,
        )
    return partial(_on_change_dispatcher, listener)


class QueryScheduler:
    """Schedule outgoing PTR queries for Continuous Multicast DNS Querying

    https://datatracker.ietf.org/doc/html/rfc6762#section-5.2

    """

    __slots__ = (
        "_addr",
        "_clock_resolution_millis",
        "_first_random_delay_interval",
        "_loop",
        "_min_time_between_queries_millis",
        "_multicast",
        "_next_run",
        "_next_scheduled_for_alias",
        "_port",
        "_query_heap",
        "_question_type",
        "_startup_queries_sent",
        "_types",
        "_zc",
    )

    def __init__(
        self,
        zc: Zeroconf,
        types: set[str],
        addr: str | None,
        port: int,
        multicast: bool,
        delay: int,
        first_random_delay_interval: tuple[int, int],
        question_type: DNSQuestionType | None,
    ) -> None:
        self._zc = zc
        self._types = types
        self._addr = addr
        self._port = port
        self._multicast = multicast
        self._first_random_delay_interval = first_random_delay_interval
        self._min_time_between_queries_millis = delay
        self._loop: asyncio.AbstractEventLoop | None = None
        self._startup_queries_sent = 0
        self._next_scheduled_for_alias: dict[str, _ScheduledPTRQuery] = {}
        self._query_heap: list[_ScheduledPTRQuery] = []
        self._next_run: asyncio.TimerHandle | None = None
        self._clock_resolution_millis = time.get_clock_info("monotonic").resolution * 1000
        self._question_type = question_type

    def start(self, loop: asyncio.AbstractEventLoop) -> None:
        """Start the scheduler.

        https://datatracker.ietf.org/doc/html/rfc6762#section-5.2
        To avoid accidental synchronization when, for some reason, multiple
        clients begin querying at exactly the same moment (e.g., because of
        some common external trigger event), a Multicast DNS querier SHOULD
        also delay the first query of the series by a randomly chosen amount
        in the range 20-120 ms.
        """
        start_delay = millis_to_seconds(random.randint(*self._first_random_delay_interval))  # noqa: S311
        self._loop = loop
        self._next_run = loop.call_later(start_delay, self._process_startup_queries)

    def stop(self) -> None:
        """Stop the scheduler."""
        if self._next_run is not None:
            self._next_run.cancel()
            self._next_run = None
        self._next_scheduled_for_alias.clear()
        self._query_heap.clear()

    def _schedule_ptr_refresh(
        self,
        pointer: DNSPointer,
        expire_time_millis: float_,
        refresh_time_millis: float_,
    ) -> None:
        """Schedule a query for a pointer."""
        scheduled_ptr_query = _ScheduledPTRQuery(
            pointer.alias, pointer.name, pointer.ttl, expire_time_millis, refresh_time_millis
        )
        self._schedule_ptr_query(scheduled_ptr_query)

    def _schedule_ptr_query(self, scheduled_query: _ScheduledPTRQuery) -> None:
        """Schedule a query for a pointer."""
        self._next_scheduled_for_alias[scheduled_query.alias] = scheduled_query
        heappush(self._query_heap, scheduled_query)

    def cancel_ptr_refresh(self, pointer: DNSPointer) -> None:
        """Cancel a query for a pointer."""
        scheduled = self._next_scheduled_for_alias.pop(pointer.alias, None)
        if scheduled:
            scheduled.cancelled = True

    def reschedule_ptr_first_refresh(self, pointer: DNSPointer) -> None:
        """Reschedule a query for a pointer."""
        current = self._next_scheduled_for_alias.get(pointer.alias)
        refresh_time_millis = pointer.get_expiration_time(_EXPIRE_REFRESH_TIME_PERCENT)
        if current is not None:
            # If the expire time is within self._min_time_between_queries_millis
            # of the current scheduled time avoid churn by not rescheduling
            if (
                -self._min_time_between_queries_millis
                <= refresh_time_millis - current.when_millis
                <= self._min_time_between_queries_millis
            ):
                return
            current.cancelled = True
            del self._next_scheduled_for_alias[pointer.alias]
        expire_time_millis = pointer.get_expiration_time(100)
        self._schedule_ptr_refresh(pointer, expire_time_millis, refresh_time_millis)

    def schedule_rescue_query(
        self,
        query: _ScheduledPTRQuery,
        now_millis: float_,
        additional_percentage: float_,
    ) -> None:
        """Reschedule a query for a pointer at an additional percentage of expiration."""
        ttl_millis = query.ttl * 1000
        additional_wait = ttl_millis * additional_percentage
        next_query_time = now_millis + additional_wait
        if next_query_time >= query.expire_time_millis:
            # If we would schedule past the expire time
            # there is no point in scheduling as we already
            # tried to rescue the record and failed
            return
        scheduled_ptr_query = _ScheduledPTRQuery(
            query.alias,
            query.name,
            query.ttl,
            query.expire_time_millis,
            next_query_time,
        )
        self._schedule_ptr_query(scheduled_ptr_query)

    def _process_startup_queries(self) -> None:
        if TYPE_CHECKING:
            assert self._loop is not None
        # This is a safety to ensure we stop sending queries if Zeroconf instance
        # is stopped without the browser being cancelled
        if self._zc.done:
            return

        now_millis = current_time_millis()

        # At first we will send STARTUP_QUERIES queries to get the cache populated
        self.async_send_ready_queries(self._startup_queries_sent == 0, now_millis, self._types)
        self._startup_queries_sent += 1

        # Once we finish sending the initial queries we will
        # switch to a strategy of sending queries only when we
        # need to refresh records that are about to expire
        if self._startup_queries_sent >= STARTUP_QUERIES:
            self._next_run = self._loop.call_at(
                millis_to_seconds(now_millis + self._min_time_between_queries_millis),
                self._process_ready_types,
            )
            return

        self._next_run = self._loop.call_later(self._startup_queries_sent**2, self._process_startup_queries)

    def _process_ready_types(self) -> None:
        """Generate a list of ready types that is due and schedule the next time."""
        if TYPE_CHECKING:
            assert self._loop is not None
        # This is a safety to ensure we stop sending queries if Zeroconf instance
        # is stopped without the browser being cancelled
        if self._zc.done:
            return

        now_millis = current_time_millis()
        # Refresh records that are about to expire (aka
        # _EXPIRE_REFRESH_TIME_PERCENT which is currently 75% of the TTL) and
        # additional rescue queries if the 75% query failed to refresh the record
        # with a minimum time between queries of _min_time_between_queries
        # which defaults to 10s

        ready_types: set[str] = set()
        next_scheduled: _ScheduledPTRQuery | None = None
        end_time_millis = now_millis + self._clock_resolution_millis
        schedule_rescue: list[_ScheduledPTRQuery] = []

        while self._query_heap:
            query = self._query_heap[0]
            if query.cancelled:
                heappop(self._query_heap)
                continue
            if query.when_millis > end_time_millis:
                next_scheduled = query
                break
            query = heappop(self._query_heap)
            ready_types.add(query.name)
            del self._next_scheduled_for_alias[query.alias]
            # If there is still more than 10% of the TTL remaining
            # schedule a query again to try to rescue the record
            # from expiring. If the record is refreshed before
            # the query, the query will get cancelled.
            schedule_rescue.append(query)

        for query in schedule_rescue:
            self.schedule_rescue_query(query, now_millis, RESCUE_RECORD_RETRY_TTL_PERCENTAGE)

        if ready_types:
            self.async_send_ready_queries(False, now_millis, ready_types)

        next_time_millis = now_millis + self._min_time_between_queries_millis

        if next_scheduled is not None and next_scheduled.when_millis > next_time_millis:
            next_when_millis = next_scheduled.when_millis
        else:
            next_when_millis = next_time_millis

        self._next_run = self._loop.call_at(millis_to_seconds(next_when_millis), self._process_ready_types)

    def async_send_ready_queries(
        self, first_request: bool, now_millis: float_, ready_types: set[str]
    ) -> None:
        """Send any ready queries."""
        # If they did not specify and this is the first request, ask QU questions
        # https://datatracker.ietf.org/doc/html/rfc6762#section-5.4 since we are
        # just starting up and we know our cache is likely empty. This ensures
        # the next outgoing will be sent with the known answers list.
        question_type = QU_QUESTION if self._question_type is None and first_request else self._question_type
        outs = generate_service_query(self._zc, now_millis, ready_types, self._multicast, question_type)
        if outs:
            for out in outs:
                self._zc.async_send(out, self._addr, self._port)


class _ServiceBrowserBase(RecordUpdateListener):
    """Base class for ServiceBrowser."""

    __slots__ = (
        "_cache",
        "_loop",
        "_pending_handlers",
        "_query_sender_task",
        "_service_state_changed",
        "done",
        "query_scheduler",
        "types",
        "zc",
    )

    def __init__(
        self,
        zc: Zeroconf,
        type_: str | list,
        handlers: ServiceListener | list[Callable[..., None]] | None = None,
        listener: ServiceListener | None = None,
        addr: str | None = None,
        port: int = _MDNS_PORT,
        delay: int = _BROWSER_TIME,
        question_type: DNSQuestionType | None = None,
    ) -> None:
        """Used to browse for a service for specific type(s).

        Constructor parameters are as follows:

        * `zc`: A Zeroconf instance
        * `type_`: fully qualified service type name
        * `handler`: ServiceListener or Callable that knows how to process ServiceStateChange events
        * `listener`: ServiceListener
        * `addr`: address to send queries (will default to multicast)
        * `port`: port to send queries (will default to mdns 5353)
        * `delay`: The initial delay between answering questions
        * `question_type`: The type of questions to ask (DNSQuestionType.QM or DNSQuestionType.QU)

        The listener object will have its add_service() and
        remove_service() methods called when this browser
        discovers changes in the services availability.
        """
        assert handlers or listener, "You need to specify at least one handler"
        self.types: set[str] = set(type_ if isinstance(type_, list) else [type_])
        for check_type_ in self.types:
            # Will generate BadTypeInNameException on a bad name
            service_type_name(check_type_, strict=False)
        self.zc = zc
        self._cache = zc.cache
        assert zc.loop is not None
        self._loop = zc.loop
        self._pending_handlers: dict[tuple[str, str], ServiceStateChange] = {}
        self._service_state_changed = Signal()
        self.query_scheduler = QueryScheduler(
            zc,
            self.types,
            addr,
            port,
            addr in (None, _MDNS_ADDR, _MDNS_ADDR6),
            delay,
            _FIRST_QUERY_DELAY_RANDOM_INTERVAL,
            question_type,
        )
        self.done = False
        self._query_sender_task: asyncio.Task | None = None

        if hasattr(handlers, "add_service"):
            listener = cast(ServiceListener, handlers)
            handlers = None

        handlers = cast(list[Callable[..., None]], handlers or [])

        if listener:
            handlers.append(_service_state_changed_from_listener(listener))

        for h in handlers:
            self.service_state_changed.register_handler(h)

    def _async_start(self) -> None:
        """Generate the next time and setup listeners.

        Must be called by uses of this base class after they
        have finished setting their properties.
        """
        self.zc.async_add_listener(self, [DNSQuestion(type_, _TYPE_PTR, _CLASS_IN) for type_ in self.types])
        # Only start queries after the listener is installed
        self._query_sender_task = asyncio.ensure_future(self._async_start_query_sender())

    @property
    def service_state_changed(self) -> SignalRegistrationInterface:
        return self._service_state_changed.registration_interface

    def _names_matching_types(self, names: Iterable[str]) -> list[tuple[str, str]]:
        """Return the type and name for records matching the types we are browsing."""
        return [
            (type_, name) for name in names for type_ in self.types.intersection(cached_possible_types(name))
        ]

    def _enqueue_callback(
        self,
        state_change: ServiceStateChange,
        type_: str_,
        name: str_,
    ) -> None:
        # Code to ensure we only do a single update message
        # Precedence is; Added, Remove, Update
        key = (name, type_)
        if (
            state_change is SERVICE_STATE_CHANGE_ADDED
            or (
                state_change is SERVICE_STATE_CHANGE_REMOVED
                and self._pending_handlers.get(key) is not SERVICE_STATE_CHANGE_ADDED
            )
            or (state_change is SERVICE_STATE_CHANGE_UPDATED and key not in self._pending_handlers)
        ):
            self._pending_handlers[key] = state_change

    def async_update_records(self, zc: Zeroconf, now: float_, records: list[RecordUpdate]) -> None:
        """Callback invoked by Zeroconf when new information arrives.

        Updates information required by browser in the Zeroconf cache.

        Ensures that there is are no unnecessary duplicates in the list.

        This method will be run in the event loop.
        """
        for record_update in records:
            record = record_update.new
            old_record = record_update.old
            record_type = record.type

            if record_type is _TYPE_PTR:
                if TYPE_CHECKING:
                    record = cast(DNSPointer, record)
                pointer = record
                for type_ in self.types.intersection(cached_possible_types(pointer.name)):
                    if old_record is None:
                        self._enqueue_callback(SERVICE_STATE_CHANGE_ADDED, type_, pointer.alias)
                        self.query_scheduler.reschedule_ptr_first_refresh(pointer)
                    elif pointer.is_expired(now):
                        self._enqueue_callback(SERVICE_STATE_CHANGE_REMOVED, type_, pointer.alias)
                        self.query_scheduler.cancel_ptr_refresh(pointer)
                    else:
                        self.query_scheduler.reschedule_ptr_first_refresh(pointer)
                continue

            # If its expired or already exists in the cache it cannot be updated.
            if old_record is not None or record.is_expired(now):
                continue

            if record_type in _ADDRESS_RECORD_TYPES:
                cache = self._cache
                names = {service.name for service in cache.async_entries_with_server(record.name)}
                # Iterate through the DNSCache and callback any services that use this address
                for type_, name in self._names_matching_types(names):
                    self._enqueue_callback(SERVICE_STATE_CHANGE_UPDATED, type_, name)
                continue

            for type_, name in self._names_matching_types((record.name,)):
                self._enqueue_callback(SERVICE_STATE_CHANGE_UPDATED, type_, name)

    def async_update_records_complete(self) -> None:
        """Called when a record update has completed for all handlers.

        At this point the cache will have the new records.

        This method will be run in the event loop.

        This method is expected to be overridden by subclasses.
        """
        for pending in self._pending_handlers.items():
            self._fire_service_state_changed_event(pending)
        self._pending_handlers.clear()

    def _fire_service_state_changed_event(self, event: tuple[tuple[str, str], ServiceStateChange]) -> None:
        """Fire a service state changed event.

        When running with ServiceBrowser, this will happen in the dedicated
        thread.

        When running with AsyncServiceBrowser, this will happen in the event loop.
        """
        name_type = event[0]
        state_change = event[1]
        self._service_state_changed.fire(
            zeroconf=self.zc,
            service_type=name_type[1],
            name=name_type[0],
            state_change=state_change,
        )

    def _async_cancel(self) -> None:
        """Cancel the browser."""
        self.done = True
        self.query_scheduler.stop()
        self.zc.async_remove_listener(self)
        assert self._query_sender_task is not None, "Attempted to cancel a browser that was not started"
        self._query_sender_task.cancel()
        self._query_sender_task = None

    async def _async_start_query_sender(self) -> None:
        """Start scheduling queries."""
        if not self.zc.started:
            await self.zc.async_wait_for_start()
        self.query_scheduler.start(self._loop)


class ServiceBrowser(_ServiceBrowserBase, threading.Thread):
    """Used to browse for a service of a specific type.

    The listener object will have its add_service() and
    remove_service() methods called when this browser
    discovers changes in the services availability."""

    def __init__(
        self,
        zc: Zeroconf,
        type_: str | list,
        handlers: ServiceListener | list[Callable[..., None]] | None = None,
        listener: ServiceListener | None = None,
        addr: str | None = None,
        port: int = _MDNS_PORT,
        delay: int = _BROWSER_TIME,
        question_type: DNSQuestionType | None = None,
    ) -> None:
        assert zc.loop is not None
        if not zc.loop.is_running():
            raise RuntimeError("The event loop is not running")
        threading.Thread.__init__(self)
        super().__init__(zc, type_, handlers, listener, addr, port, delay, question_type)
        # Add the queue before the listener is installed in _setup
        # to ensure that events run in the dedicated thread and do
        # not block the event loop
        self.queue: queue.SimpleQueue = queue.SimpleQueue()
        self.daemon = True
        self.start()
        zc.loop.call_soon_threadsafe(self._async_start)
        self.name = "zeroconf-ServiceBrowser-{}-{}".format(
            "-".join([type_[:-7] for type_ in self.types]),
            getattr(self, "native_id", self.ident),
        )

    def cancel(self) -> None:
        """Cancel the browser."""
        assert self.zc.loop is not None
        self.queue.put(None)
        self.zc.loop.call_soon_threadsafe(self._async_cancel)
        self.join()

    def run(self) -> None:
        """Run the browser thread."""
        while True:
            event = self.queue.get()
            if event is None:
                return
            self._fire_service_state_changed_event(event)

    def async_update_records_complete(self) -> None:
        """Called when a record update has completed for all handlers.

        At this point the cache will have the new records.

        This method will be run in the event loop.
        """
        for pending in self._pending_handlers.items():
            self.queue.put(pending)
        self._pending_handlers.clear()

    def __enter__(self) -> ServiceBrowser:
        return self

    def __exit__(  # pylint: disable=useless-return
        self,
        exc_type: type[BaseException] | None,
        exc_val: BaseException | None,
        exc_tb: TracebackType | None,
    ) -> bool | None:
        self.cancel()
        return None
