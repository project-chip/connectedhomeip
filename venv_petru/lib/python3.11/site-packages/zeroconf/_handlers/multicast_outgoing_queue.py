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

import random
from collections import deque
from typing import TYPE_CHECKING

from .._utils.time import current_time_millis, millis_to_seconds
from .answers import (
    MULTICAST_DELAY_RANDOM_INTERVAL,
    AnswerGroup,
    _AnswerWithAdditionalsType,
    construct_outgoing_multicast_answers,
)

RAND_INT = random.randint

if TYPE_CHECKING:
    from .._core import Zeroconf

_float = float
_int = int


class MulticastOutgoingQueue:
    """An outgoing queue used to aggregate multicast responses."""

    __slots__ = (
        "_additional_delay",
        "_aggregation_delay",
        "_multicast_delay_random_max",
        "_multicast_delay_random_min",
        "queue",
        "zc",
    )

    def __init__(self, zeroconf: Zeroconf, additional_delay: _int, max_aggregation_delay: _int) -> None:
        self.zc = zeroconf
        self.queue: deque[AnswerGroup] = deque()
        # Additional delay is used to implement
        # Protect the network against excessive packet flooding
        # https://datatracker.ietf.org/doc/html/rfc6762#section-14
        self._multicast_delay_random_min = MULTICAST_DELAY_RANDOM_INTERVAL[0]
        self._multicast_delay_random_max = MULTICAST_DELAY_RANDOM_INTERVAL[1]
        self._additional_delay = additional_delay
        self._aggregation_delay = max_aggregation_delay

    def async_add(self, now: _float, answers: _AnswerWithAdditionalsType) -> None:
        """Add a group of answers with additionals to the outgoing queue."""
        loop = self.zc.loop
        if TYPE_CHECKING:
            assert loop is not None
        random_int = RAND_INT(self._multicast_delay_random_min, self._multicast_delay_random_max)
        random_delay = random_int + self._additional_delay
        send_after = now + random_delay
        send_before = now + self._aggregation_delay + self._additional_delay
        if len(self.queue):
            # If we calculate a random delay for the send after time
            # that is less than the last group scheduled to go out,
            # we instead add the answers to the last group as this
            # allows aggregating additional responses
            last_group = self.queue[-1]
            if send_after <= last_group.send_after:
                last_group.answers.update(answers)
                return
        else:
            loop.call_at(loop.time() + millis_to_seconds(random_delay), self.async_ready)
        self.queue.append(AnswerGroup(send_after, send_before, answers))

    def _remove_answers_from_queue(self, answers: _AnswerWithAdditionalsType) -> None:
        """Remove a set of answers from the outgoing queue."""
        for pending in self.queue:
            for record in answers:
                pending.answers.pop(record, None)

    def async_ready(self) -> None:
        """Process anything in the queue that is ready."""
        zc = self.zc
        loop = zc.loop
        if TYPE_CHECKING:
            assert loop is not None
        now = current_time_millis()

        if len(self.queue) > 1 and self.queue[0].send_before > now:
            # There is more than one answer in the queue,
            # delay until we have to send it (first answer group reaches send_before)
            loop.call_at(
                loop.time() + millis_to_seconds(self.queue[0].send_before - now),
                self.async_ready,
            )
            return

        answers: _AnswerWithAdditionalsType = {}
        # Add all groups that can be sent now
        while len(self.queue) and self.queue[0].send_after <= now:
            answers.update(self.queue.popleft().answers)

        if len(self.queue):
            # If there are still groups in the queue that are not ready to send
            # be sure we schedule them to go out later
            loop.call_at(
                loop.time() + millis_to_seconds(self.queue[0].send_after - now),
                self.async_ready,
            )

        if answers:  # pragma: no branch
            # If we have the same answer scheduled to go out, remove them
            self._remove_answers_from_queue(answers)
            zc.async_send(construct_outgoing_multicast_answers(answers))
