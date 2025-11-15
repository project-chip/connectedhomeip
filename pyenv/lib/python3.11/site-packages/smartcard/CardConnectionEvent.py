"""The CardConnectionEvent is sent to CardConnectionObserver objects
when a CardConnection event occurs.

__author__ = "https://www.gemalto.com/"

Copyright 2001-2012 gemalto
Author: Jean-Daniel Aussel, mailto:jean-daniel.aussel@gemalto.com

This file is part of pyscard.

pyscard is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

pyscard is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with pyscard; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
"""

# pylint: disable=too-few-public-methods


class CardConnectionEvent:
    """Base class for card connection events.

    This event is notified by CardConnection objects."""

    def __init__(self, event_type, args=None):
        """
        @param event_type:   'connect', 'reconnect', 'disconnect', 'command', 'response'
        @param args:   None for 'connect', 'reconnect' or 'disconnect'
                command APDU byte list for 'command'
                [response data, sw1, sw2] for 'response'
        """
        self.type = event_type
        self.args = args
