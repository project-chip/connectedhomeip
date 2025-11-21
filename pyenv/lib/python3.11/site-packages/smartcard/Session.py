"""
Smartcard Session.

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

from smartcard.Exceptions import InvalidReaderException, NoReadersException
from smartcard.PassThruCardService import PassThruCardService
from smartcard.System import readers


class Session:
    """The Session object enables programmers to transmit APDU to smartcards.

    This is an example of use of the Session object:

    >>> import smartcard
    >>> reader=smartcard.listReaders()
    >>> s = smartcard.Session(reader[0])
    >>> SELECT = [0xA0, 0xA4, 0x00, 0x00, 0x02]
    >>> DF_TELECOM = [0x7F, 0x10]
    >>> data, sw1, sw2 = s.sendCommandAPDU(SELECT+DF_TELECOM)
    >>> print(data, sw1, sw2)
    >>> s.close()
    >>> print(`s`)
    """

    def __init__(self, readerName=None):
        """Session constructor. Initializes a smart card session and
        connect to the card.

        @param readerName: reader to connect to; default is first PCSC reader
        """

        # if reader name not given, select first reader
        if readerName is None:
            if len(readers()) > 0:
                self.reader = readers()[0]
                self.readerName = repr(self.reader)
            else:
                raise NoReadersException()

        # otherwise select reader from name
        else:
            self.readerName = readerName
            for reader in readers():
                if readerName == str(reader):
                    self.reader = reader
                    self.readerName = repr(self.reader)

        try:
            self.reader
        except AttributeError as exc:
            raise InvalidReaderException(self.readerName) from exc

        # open card connection and bind PassThruCardService
        cc = self.reader.createConnection()
        self.cs = PassThruCardService(cc)
        self.cs.connection.connect()

    def close(self):
        """Close the smartcard session.

        Closing a session will disconnect from the card."""
        self.cs.connection.disconnect()

    def sendCommandAPDU(self, command):
        """Send an APDU command to the connected smartcard.

        @param command: list of APDU bytes, e.g. [0xA0, 0xA4, 0x00, 0x00, 0x02]

        @return: a tuple (response, sw1, sw2) where
                response is the APDU response
                sw1, sw2 are the two status words
        """

        response, sw1, sw2 = self.cs.connection.transmit(command)

        if len(response) > 2:
            response.append(sw1)
            response.append(sw2)
        return response, sw1, sw2

    def getATR(self):
        """Returns the ATR of the connected card."""
        return self.cs.connection.getATR()

    def __repr__(self):
        """Returns a string representation of the session."""
        return f"<Session instance: readerName={self.readerName}>"


if __name__ == "__main__":
    # Small sample illustrating the use of Session.py.
    pass
