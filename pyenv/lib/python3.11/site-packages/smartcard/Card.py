"""Card class.

__author__ = "gemalto https://www.gemalto.com/"

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

from smartcard.reader.Reader import Reader
from smartcard.System import readers
from smartcard.util import toHexString


class Card:
    """Card class."""

    def __init__(self, reader, atr):
        """Card constructor.
        @param reader: reader in which the card is inserted
        @param atr: ATR of the card"""
        self.reader = reader
        self.atr = atr

    def __repr__(self):
        """Return a string representing the Card (atr and reader
        concatenation)."""
        return toHexString(self.atr) + " / " + str(self.reader)

    def __eq__(self, other):
        """Return True if self==other (same reader and same atr).
        Return False otherwise."""
        if isinstance(other, Card):
            return self.atr == other.atr and repr(self.reader) == repr(other.reader)

        return False

    def __ne__(self, other):
        """Return True if self!=other (same reader and same atr).Returns
        False otherwise."""
        return not self.__eq__(other)

    def __hash__(self):
        """Returns a hash value for this object (str(self) is unique)."""
        return hash(str(self))

    def createConnection(self):
        """Return a CardConnection to the Card object."""
        readerobj = None
        if isinstance(self.reader, Reader):
            readerobj = self.reader
        elif isinstance(self.reader, str):
            for reader in readers():
                if self.reader == str(reader):
                    readerobj = reader

        if readerobj:
            return readerobj.createConnection()

        # raise CardConnectionException(
        # 'not a valid reader: ' + str(self.reader))
        return None
