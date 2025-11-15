"""AbstractCardRequest class.

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

import smartcard.System
from smartcard.CardType import AnyCardType
from smartcard.PassThruCardService import PassThruCardService


class AbstractCardRequest:
    """The base class for xxxCardRequest classes.

    A CardRequest is used for waitForCard() invocations and specifies what
    kind of smart card an application is waited for."""

    # pylint: disable=too-many-arguments
    # pylint: disable=too-many-positional-arguments
    def __init__(
        self,
        newcardonly=False,
        readers=None,
        cardType=None,
        cardServiceClass=None,
        timeout=1,
    ):
        """Construct new CardRequest.

        @param newcardonly: if True, request a new card; default is
                            False, i.e. accepts cards already inserted

        @param readers:     the list of readers to consider for
                            requesting a card; default is to consider
                            all readers

        @param cardType:    the L{smartcard.CardType.CardType} to wait for;
                            default is L{smartcard.CardType.AnyCardType},
                            i.e. the request will succeed with any card

        @param cardServiceClass: the specific card service class to create
                            and bind to the card;default is to create
                            and bind a L{smartcard.PassThruCardService}

        @param timeout:     the time in seconds we are ready to wait for
                            connecting to the requested card.  default
                            is to wait one second; to wait forever, set
                            timeout to None
        """
        self.newcardonly = newcardonly
        self.readersAsked = readers
        self.cardType = cardType
        self.cardServiceClass = cardServiceClass
        self.timeout = timeout

        # if no CardType requested, use AnyCardType
        if self.cardType is None:
            self.cardType = AnyCardType()

        # if no card service requested, use pass-thru card service
        if self.cardServiceClass is None:
            self.cardServiceClass = PassThruCardService

    def getReaders(self):
        """Returns the list or readers on which to wait for cards."""
        # if readers not given, use all readers
        if self.readersAsked is None:
            return smartcard.System.readers()
        return self.readersAsked

    def waitforcard(self):
        """Wait for card insertion and returns a card service."""

    def waitforcardevent(self):
        """Wait for card insertion or removal."""
