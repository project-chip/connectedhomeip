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

from ._dns import DNSQuestion, DNSRecord
from .const import _DUPLICATE_QUESTION_INTERVAL

# The QuestionHistory is used to implement Duplicate Question Suppression
# https://datatracker.ietf.org/doc/html/rfc6762#section-7.3

_float = float


class QuestionHistory:
    """Remember questions and known answers."""

    def __init__(self) -> None:
        """Init a new QuestionHistory."""
        self._history: dict[DNSQuestion, tuple[float, set[DNSRecord]]] = {}

    def add_question_at_time(self, question: DNSQuestion, now: _float, known_answers: set[DNSRecord]) -> None:
        """Remember a question with known answers."""
        self._history[question] = (now, known_answers)

    def suppresses(self, question: DNSQuestion, now: _float, known_answers: set[DNSRecord]) -> bool:
        """Check to see if a question should be suppressed.

        https://datatracker.ietf.org/doc/html/rfc6762#section-7.3
        When multiple queriers on the network are querying
        for the same resource records, there is no need for them to all be
        repeatedly asking the same question.
        """
        previous_question = self._history.get(question)
        # There was not previous question in the history
        if not previous_question:
            return False
        than, previous_known_answers = previous_question
        # The last question was older than 999ms
        if now - than > _DUPLICATE_QUESTION_INTERVAL:
            return False
        # The last question has more known answers than
        # we knew so we have to ask
        return not previous_known_answers - known_answers

    def async_expire(self, now: _float) -> None:
        """Expire the history of old questions."""
        removes: list[DNSQuestion] = []
        for question, now_known_answers in self._history.items():
            than, _ = now_known_answers
            if now - than > _DUPLICATE_QUESTION_INTERVAL:
                removes.append(question)
        for question in removes:
            del self._history[question]

    def clear(self) -> None:
        """Clear the history."""
        self._history.clear()
