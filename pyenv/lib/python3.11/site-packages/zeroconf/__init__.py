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

from ._cache import DNSCache  # noqa # import needed for backwards compat
from ._core import Zeroconf
from ._dns import (  # noqa # import needed for backwards compat
    DNSAddress,
    DNSEntry,
    DNSHinfo,
    DNSNsec,
    DNSPointer,
    DNSQuestion,
    DNSQuestionType,
    DNSRecord,
    DNSService,
    DNSText,
)
from ._exceptions import (
    AbstractMethodException,
    BadTypeInNameException,
    Error,
    EventLoopBlocked,
    IncomingDecodeError,
    NamePartTooLongException,
    NonUniqueNameException,
    NotRunningException,
    ServiceNameAlreadyRegistered,
)
from ._logger import QuietLogger, log  # noqa # import needed for backwards compat
from ._protocol.incoming import DNSIncoming  # noqa # import needed for backwards compat
from ._protocol.outgoing import DNSOutgoing  # noqa # import needed for backwards compat
from ._record_update import RecordUpdate
from ._services import (  # noqa # import needed for backwards compat
    ServiceListener,
    ServiceStateChange,
    Signal,
    SignalRegistrationInterface,
)
from ._services.browser import ServiceBrowser
from ._services.info import (  # noqa # import needed for backwards compat
    AddressResolver,
    AddressResolverIPv4,
    AddressResolverIPv6,
    ServiceInfo,
    instance_name_from_service_info,
)
from ._services.registry import (  # noqa # import needed for backwards compat
    ServiceRegistry,
)
from ._services.types import ZeroconfServiceTypes
from ._updates import RecordUpdateListener
from ._utils.name import service_type_name  # noqa # import needed for backwards compat
from ._utils.net import (  # noqa # import needed for backwards compat
    InterfaceChoice,
    InterfacesType,
    IPVersion,
    add_multicast_member,
    autodetect_ip_version,
    create_sockets,
    get_all_addresses,
    get_all_addresses_v6,
)
from ._utils.time import (  # noqa # import needed for backwards compat
    current_time_millis,
    millis_to_seconds,
)

__author__ = "Paul Scott-Murphy, William McBrine"
__maintainer__ = "Jakub Stasiak <jakub@stasiak.at>"
__version__ = "0.148.0"
__license__ = "LGPL"


__all__ = [
    "AbstractMethodException",
    "BadTypeInNameException",
    "DNSQuestionType",
    # Exceptions
    "Error",
    "EventLoopBlocked",
    "IPVersion",
    "IncomingDecodeError",
    "InterfaceChoice",
    "NamePartTooLongException",
    "NonUniqueNameException",
    "NotRunningException",
    "RecordUpdate",
    "RecordUpdateListener",
    "ServiceBrowser",
    "ServiceInfo",
    "ServiceListener",
    "ServiceNameAlreadyRegistered",
    "ServiceStateChange",
    "Zeroconf",
    "ZeroconfServiceTypes",
    "__version__",
    "current_time_millis",
]
