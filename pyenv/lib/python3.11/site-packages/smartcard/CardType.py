"""Abstract CarType.

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

from smartcard.Exceptions import InvalidATRMaskLengthException
from smartcard.System import readers
from smartcard.util import toHexString

# pylint: disable=too-few-public-methods


class CardType:
    """Abstract base class for CardTypes.

    Known subclasses: L{smartcard.CardType.AnyCardType}
    L{smartcard.CardType.ATRCardType}."""

    def __init__(self):
        """CardType constructor."""

    def matches(self, atr, reader=None):
        """Returns true if atr and card connected match the L{CardType}.

        @param atr:    the atr to check for matching
        @param reader: the reader (optional); default is None

        The reader can be used in some subclasses to do advanced
        matching that require connecting to the card."""


class AnyCardType(CardType):
    """The AnyCardType matches any card."""

    def matches(self, atr, reader=None):
        """Always returns true, i.e. AnyCardType matches any card.

        @param atr:    the atr to check for matching
        @param reader: the reader (optional); default is None"""
        return True


class ATRCardType(CardType):
    """The ATRCardType defines a card from an ATR and a mask."""

    def __init__(self, atr, mask=None):
        """ATRCardType constructor.
        @param atr:    the ATR of the CardType
        @param mask:   an optional mask to be applied to the ATR for
            L{CardType} matching default is None
        """
        super().__init__()
        self.atr = list(atr)
        self.mask = mask
        if mask is None:
            self.maskedatr = self.atr
        else:
            if len(self.atr) != len(self.mask):
                raise InvalidATRMaskLengthException(toHexString(mask))
            self.maskedatr = list(map(lambda x, y: x & y, self.atr, self.mask))

    def matches(self, atr, reader=None):
        """Returns true if the atr matches the masked CardType atr.

        @param atr:    the atr to check for matching
        @param reader: the reader (optional); default is None

        When atr is compared to the CardType ATR, matches returns true if
        and only if CardType.atr & CardType.mask = atr & CardType.mask,
        where & is the bitwise logical AND."""

        if len(atr) != len(self.atr):
            return not True

        if self.mask is not None:
            maskedatr = list(map(lambda x, y: x & y, list(atr), self.mask))
        else:
            maskedatr = atr
        return self.maskedatr == maskedatr


if __name__ == "__main__":
    # Small sample illustrating the use of CardType.py.
    r = readers()
    print(r)
    connection = r[0].createConnection()
    connection.connect()
    atrct = ATRCardType([0x3B, 0x16, 0x94, 0x20, 0x02, 0x01, 0x00, 0x00, 0x0D])
    print(atrct.matches(connection.getATR()))
