"""The CardConnection abstract class manages connections with a card and
apdu transmission.

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

from smartcard.CardConnectionEvent import CardConnectionEvent
from smartcard.Observer import Observable


class CardConnection(Observable):
    """Card connection abstract class."""

    T0_protocol = 0x00000001
    """ protocol T=0 """

    T1_protocol = 0x00000002
    """ protocol T=1 """

    RAW_protocol = 0x00010000
    """ protocol RAW (direct access to the reader) """

    T15_protocol = 0x00000008
    """ protocol T=15 """

    def __init__(self, reader):
        """Construct a new card connection.

        @param reader: name of the reader in which the smartcard to connect
        to is located.
        """
        Observable.__init__(self)
        self.reader = reader
        """ reader name """
        self.errorcheckingchain = None
        """ see L{setErrorCheckingChain} """
        self.defaultprotocol = CardConnection.T0_protocol | CardConnection.T1_protocol
        """ see L{setProtocol} and L{getProtocol} """

    def __del__(self):
        """Connect to card."""

    def addSWExceptionToFilter(self, exClass):
        """Add a status word exception class to be filtered.

        @param exClass: the class to filter, e.g.
        L{smartcard.sw.SWExceptions.WarningProcessingException}

        Filtered exceptions will not be raised when encountered in the
        error checking chain."""
        if self.errorcheckingchain is not None:
            self.errorcheckingchain[0].addFilterException(exClass)

    def addObserver(self, observer):
        """Add a L{CardConnection} observer."""
        Observable.addObserver(self, observer)

    def deleteObserver(self, observer):
        """Remove a L{CardConnection} observer."""
        Observable.deleteObserver(self, observer)

    def connect(self, protocol=None, mode=None, disposition=None):
        """Connect to card.
        @param protocol: a bit mask of the protocols to use, from
        L{CardConnection.T0_protocol}, L{CardConnection.T1_protocol},
        L{CardConnection.RAW_protocol}, L{CardConnection.T15_protocol}

        @param mode: C{smartcard.scard.SCARD_SHARE_SHARED} (default),
        C{smartcard.scard.SCARD_SHARE_EXCLUSIVE} or
        C{smartcard.scard.SCARD_SHARE_DIRECT}

        @param disposition: C{smartcard.scard.SCARD_LEAVE_CARD}
        (default), C{smartcard.scard.SCARD_RESET_CARD},
        C{smartcard.scard.SCARD_UNPOWER_CARD} or
        C{smartcard.scard.SCARD_EJECT_CARD}
        """
        # pylint: disable=unused-argument
        Observable.setChanged(self)
        Observable.notifyObservers(self, CardConnectionEvent("connect"))

    def reconnect(self, protocol=None, mode=None, disposition=None):
        """Reconnect to card.
        @param protocol: a bit mask of the protocols to use, from
        L{CardConnection.T0_protocol}, L{CardConnection.T1_protocol},
        L{CardConnection.RAW_protocol}, L{CardConnection.T15_protocol}

        @param mode: C{smartcard.scard.SCARD_SHARE_SHARED} (default),
        C{smartcard.scard.SCARD_SHARE_EXCLUSIVE} or
        C{smartcard.scard.SCARD_SHARE_DIRECT}

        @param disposition: C{smartcard.scard.SCARD_LEAVE_CARD},
        C{smartcard.scard.SCARD_RESET_CARD} (default),
        C{smartcard.scard.SCARD_UNPOWER_CARD} or
        C{smartcard.scard.SCARD_EJECT_CARD}
        """
        # pylint: disable=unused-argument
        Observable.setChanged(self)
        Observable.notifyObservers(self, CardConnectionEvent("reconnect"))

    def disconnect(self):
        """Disconnect from card."""
        Observable.setChanged(self)
        Observable.notifyObservers(self, CardConnectionEvent("disconnect"))

    def release(self):
        """Release the context."""
        Observable.setChanged(self)
        Observable.notifyObservers(self, CardConnectionEvent("release"))

    def getATR(self):
        """Return card ATR"""

    def getProtocol(self):
        """Return bit mask for the protocol of connection, or None if no
        protocol set.  The return value is a bit mask of
        L{CardConnection.T0_protocol}, L{CardConnection.T1_protocol},
        L{CardConnection.RAW_protocol}, L{CardConnection.T15_protocol}
        """
        return self.defaultprotocol

    def getReader(self):
        """Return card connection reader"""
        return self.reader

    def setErrorCheckingChain(self, errorcheckingchain):
        """Add an error checking chain.
        @param errorcheckingchain: a L{smartcard.sw.ErrorCheckingChain}
        object The error checking strategies in errorchecking chain will
        be tested with each received response APDU, and a
        L{smartcard.sw.SWExceptions.SWException} will be raised upon
        error."""
        self.errorcheckingchain = errorcheckingchain

    def setProtocol(self, protocol):
        """Set protocol for card connection.
        @param protocol: a bit mask of L{CardConnection.T0_protocol},
        L{CardConnection.T1_protocol}, L{CardConnection.RAW_protocol},
        L{CardConnection.T15_protocol}

        >>> setProtocol(CardConnection.T1_protocol | CardConnection.T0_protocol)
        """
        self.defaultprotocol = protocol

    def transmit(self, command, protocol=None):
        """Transmit an apdu. Internally calls L{doTransmit()} class method
        and notify observers upon command/response APDU events.
        Subclasses must override the L{doTransmit()} class method.

        @param command:    list of bytes to transmit

        @param protocol:   the transmission protocol, from
                    L{CardConnection.T0_protocol},
                    L{CardConnection.T1_protocol}, or
                    L{CardConnection.RAW_protocol}
        """
        Observable.setChanged(self)
        Observable.notifyObservers(
            self, CardConnectionEvent("command", [command, protocol])
        )
        data, sw1, sw2 = self.doTransmit(command, protocol)
        Observable.setChanged(self)
        Observable.notifyObservers(
            self, CardConnectionEvent("response", [data, sw1, sw2])
        )
        if self.errorcheckingchain is not None:
            self.errorcheckingchain[0](data, sw1, sw2)
        return data, sw1, sw2

    def doTransmit(self, command, protocol):
        """Performs the command APDU transmission.

        Subclasses must override this method for implementing apdu
        transmission."""
        # pylint: disable=unused-argument
        return [], 0, 0

    def control(self, controlCode, command=None):
        """Send a control command and buffer.  Internally calls
        L{doControl()} class method and notify observers upon
        command/response events.  Subclasses must override the
        L{doControl()} class method.

        @param controlCode: command code

        @param command:     list of bytes to transmit
        """
        if command is None:
            command = []
        Observable.setChanged(self)
        Observable.notifyObservers(
            self, CardConnectionEvent("command", [controlCode, command])
        )
        data = self.doControl(controlCode, command)
        Observable.setChanged(self)
        Observable.notifyObservers(self, CardConnectionEvent("response", data))
        if self.errorcheckingchain is not None:
            self.errorcheckingchain[0](data)
        return data

    def doControl(self, controlCode, command):
        """Performs the command control.

        Subclasses must override this method for implementing control."""
        # pylint: disable=unused-argument
        return []

    def getAttrib(self, attribId):
        """return the requested attribute

        @param attribId: attribute id like
        C{smartcard.scard.SCARD_ATTR_VENDOR_NAME}
        """
        Observable.setChanged(self)
        Observable.notifyObservers(self, CardConnectionEvent("attrib", [attribId]))
        data = self.doGetAttrib(attribId)
        if self.errorcheckingchain is not None:
            self.errorcheckingchain[0](data)
        return data

    def doGetAttrib(self, attribId):
        """Performs the command get attrib.

        Subclasses must override this method for implementing get attrib."""
        # pylint: disable=unused-argument
        return []

    def __enter__(self):
        """Enter the runtime context."""
        return self

    def __exit__(self, e_type, value, traceback):
        """Exit the runtime context trying to disconnect."""
        self.disconnect()
