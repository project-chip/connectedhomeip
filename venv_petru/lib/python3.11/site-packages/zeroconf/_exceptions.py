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


class Error(Exception):
    """Base class for all zeroconf exceptions."""


class IncomingDecodeError(Error):
    """Exception when there is invalid data in an incoming packet."""


class NonUniqueNameException(Error):
    """Exception when the name is already registered."""


class NamePartTooLongException(Error):
    """Exception when the name is too long."""


class AbstractMethodException(Error):
    """Exception when a required method is not implemented."""


class BadTypeInNameException(Error):
    """Exception when the type in a name is invalid."""


class ServiceNameAlreadyRegistered(Error):
    """Exception when a service name is already registered."""


class EventLoopBlocked(Error):
    """Exception when the event loop is blocked.

    This exception is never expected to be thrown
    during normal operation. It should only happen
    when the cpu is maxed out or there is something blocking
    the event loop.
    """


class NotRunningException(Error):
    """Exception when an action is called with a zeroconf instance that is not running.

    The instance may not be running because it was already shutdown
    or startup has failed in some unexpected way.
    """
