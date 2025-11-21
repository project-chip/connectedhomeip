"""Smartcard CardRequest.

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

from smartcard.pcsc.PCSCCardRequest import PCSCCardRequest


class CardRequest:
    """A CardRequest is used for waitForCard() invocations and specifies what
    kind of smart card an application is waited for.
    """

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

        @param newcardonly: if True, request a new card
                            default is False, i.e. accepts cards already
                            inserted

        @param readers:     the list of readers to consider for
                            requesting a card default is to consider all
                            readers

        @param cardType:    the L{smartcard.CardType.CardType} to wait for;
                            default is L{smartcard.CardType.AnyCardType},
                            i.e. the request will succeed with any card

        @param cardServiceClass: the specific card service class to create
                            and bind to the card default is to create
                            and bind a L{smartcard.PassThruCardService}

        @param timeout:     the time in seconds we are ready to wait for
                            connecting to the requested card.  default
                            is to wait one second to wait forever, set
                            timeout to None
        """
        self.pcsccardrequest = PCSCCardRequest(
            newcardonly, readers, cardType, cardServiceClass, timeout
        )

    def getReaders(self):
        """Returns the list or readers on which to wait for cards."""
        return self.pcsccardrequest.getReaders()

    def waitforcard(self):
        """Wait for card insertion and returns a card service."""
        return self.pcsccardrequest.waitforcard()

    def waitforcardevent(self):
        """Wait for card insertion or removal."""
        return self.pcsccardrequest.waitforcardevent()

    def __enter__(self):
        """Enter the runtime context."""
        return self

    def __exit__(self, exc_type, exc_value, exc_traceback):
        """Exit the runtime context and release the PC/SC context."""
        self.pcsccardrequest.release()
        return False


if __name__ == "__main__":
    # Small sample illustrating the use of CardRequest.py.

    from smartcard.util import toHexString

    print("Insert a new card within 10 seconds")
    with CardRequest(timeout=10, newcardonly=True) as cr:
        with cr.waitforcard() as cs:
            cs.connection.connect()
            print(cs.connection.getReader() + " " + toHexString(cs.connection.getATR()))
            cs.connection.disconnect()
