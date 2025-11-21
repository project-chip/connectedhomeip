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

from operator import attrgetter

from .._dns import DNSQuestion, DNSRecord
from .._protocol.outgoing import DNSOutgoing
from ..const import _FLAGS_AA, _FLAGS_QR_RESPONSE

_AnswerWithAdditionalsType = dict[DNSRecord, set[DNSRecord]]

int_ = int


MULTICAST_DELAY_RANDOM_INTERVAL = (20, 120)

NAME_GETTER = attrgetter("name")

_FLAGS_QR_RESPONSE_AA = _FLAGS_QR_RESPONSE | _FLAGS_AA

float_ = float


class QuestionAnswers:
    """A group of answers to a question."""

    __slots__ = ("mcast_aggregate", "mcast_aggregate_last_second", "mcast_now", "ucast")

    def __init__(
        self,
        ucast: _AnswerWithAdditionalsType,
        mcast_now: _AnswerWithAdditionalsType,
        mcast_aggregate: _AnswerWithAdditionalsType,
        mcast_aggregate_last_second: _AnswerWithAdditionalsType,
    ) -> None:
        """Initialize a QuestionAnswers."""
        self.ucast = ucast
        self.mcast_now = mcast_now
        self.mcast_aggregate = mcast_aggregate
        self.mcast_aggregate_last_second = mcast_aggregate_last_second

    def __repr__(self) -> str:
        """Return a string representation of this QuestionAnswers."""
        return (
            f"QuestionAnswers(ucast={self.ucast}, mcast_now={self.mcast_now}, "
            f"mcast_aggregate={self.mcast_aggregate}, "
            f"mcast_aggregate_last_second={self.mcast_aggregate_last_second})"
        )


class AnswerGroup:
    """A group of answers scheduled to be sent at the same time."""

    __slots__ = ("answers", "send_after", "send_before")

    def __init__(
        self,
        send_after: float_,
        send_before: float_,
        answers: _AnswerWithAdditionalsType,
    ) -> None:
        self.send_after = send_after  # Must be sent after this time
        self.send_before = send_before  # Must be sent before this time
        self.answers = answers


def construct_outgoing_multicast_answers(
    answers: _AnswerWithAdditionalsType,
) -> DNSOutgoing:
    """Add answers and additionals to a DNSOutgoing."""
    out = DNSOutgoing(_FLAGS_QR_RESPONSE_AA, True)
    _add_answers_additionals(out, answers)
    return out


def construct_outgoing_unicast_answers(
    answers: _AnswerWithAdditionalsType,
    ucast_source: bool,
    questions: list[DNSQuestion],
    id_: int_,
) -> DNSOutgoing:
    """Add answers and additionals to a DNSOutgoing."""
    out = DNSOutgoing(_FLAGS_QR_RESPONSE_AA, False, id_)
    # Adding the questions back when the source is legacy unicast behavior
    if ucast_source:
        for question in questions:
            out.add_question(question)
    _add_answers_additionals(out, answers)
    return out


def _add_answers_additionals(out: DNSOutgoing, answers: _AnswerWithAdditionalsType) -> None:
    # Find additionals and suppress any additionals that are already in answers
    sending: set[DNSRecord] = set(answers)
    # Answers are sorted to group names together to increase the chance
    # that similar names will end up in the same packet and can reduce the
    # overall size of the outgoing response via name compression
    for answer in sorted(answers, key=NAME_GETTER):
        out.add_answer_at_time(answer, 0)
        additionals = answers[answer]
        for additional in additionals:
            if additional not in sending:
                out.add_additional_answer(additional)
                sending.add(additional)
