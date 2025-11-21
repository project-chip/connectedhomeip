"""PCSCCardConnection class manages connections thru a PCSC reader.

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

from smartcard.CardConnection import CardConnection
from smartcard.Exceptions import (
    CardConnectionException,
    NoCardException,
    SmartcardException,
)
from smartcard.scard import (
    SCARD_E_INVALID_VALUE,
    SCARD_E_NO_SMARTCARD,
    SCARD_PCI_RAW,
    SCARD_PCI_T0,
    SCARD_PCI_T1,
    SCARD_PROTOCOL_RAW,
    SCARD_PROTOCOL_T0,
    SCARD_PROTOCOL_T1,
    SCARD_PROTOCOL_T15,
    SCARD_RESET_CARD,
    SCARD_S_SUCCESS,
    SCARD_SCOPE_USER,
    SCARD_SHARE_SHARED,
    SCARD_UNPOWER_CARD,
    SCARD_W_REMOVED_CARD,
    SCardConnect,
    SCardControl,
    SCardDisconnect,
    SCardEstablishContext,
    SCardGetAttrib,
    SCardGetErrorMessage,
    SCardReconnect,
    SCardReleaseContext,
    SCardStatus,
    SCardTransmit,
)


def translateprotocolmask(protocol):
    """Translate L{CardConnection} protocol mask into PCSC protocol mask."""
    pcscprotocol = 0
    if protocol is not None:
        if CardConnection.T0_protocol & protocol:
            pcscprotocol |= SCARD_PROTOCOL_T0
        if CardConnection.T1_protocol & protocol:
            pcscprotocol |= SCARD_PROTOCOL_T1
        if CardConnection.RAW_protocol & protocol:
            pcscprotocol |= SCARD_PROTOCOL_RAW
        if CardConnection.T15_protocol & protocol:
            pcscprotocol |= SCARD_PROTOCOL_T15
    return pcscprotocol


def translateprotocolheader(protocol):
    """Translate protocol into PCSC protocol header."""
    pcscprotocol = 0
    if protocol is not None:
        if CardConnection.T0_protocol == protocol:
            pcscprotocol = SCARD_PCI_T0
        if CardConnection.T1_protocol == protocol:
            pcscprotocol = SCARD_PCI_T1
        if CardConnection.RAW_protocol == protocol:
            pcscprotocol = SCARD_PCI_RAW
    return pcscprotocol


dictProtocolHeader = {SCARD_PCI_T0: "T0", SCARD_PCI_T1: "T1", SCARD_PCI_RAW: "RAW"}
dictProtocol = {
    SCARD_PROTOCOL_T0: "T0",
    SCARD_PROTOCOL_T1: "T1",
    SCARD_PROTOCOL_RAW: "RAW",
    SCARD_PROTOCOL_T15: "T15",
    SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1: "T0 or T1",
}


class PCSCCardConnection(CardConnection):
    """PCSCCard connection class. Handles connection with a card thru a
    PCSC reader."""

    def __init__(self, reader):
        """Construct a new PCSC card connection.

        @param reader: the reader in which the smartcard to connect to is located.
        """
        CardConnection.__init__(self, reader)
        self.hcard = None
        self.disposition = None
        hresult, self.hcontext = SCardEstablishContext(SCARD_SCOPE_USER)
        if hresult != SCARD_S_SUCCESS:
            raise CardConnectionException(
                "Failed to establish context : " + SCardGetErrorMessage(hresult),
                hresult=hresult,
            )

    def __del__(self):
        """Destructor. Clean PCSC connection resources."""
        # race condition: module CardConnection
        # can disappear before __del__ is called
        self.release()

    def release(self):
        """explicit release"""
        if self.hcontext is not None:
            CardConnection.release(self)
            self.disconnect()
            hresult = SCardReleaseContext(self.hcontext)
            if hresult not in (SCARD_S_SUCCESS, SCARD_E_INVALID_VALUE):
                raise CardConnectionException(
                    "Failed to release context: " + SCardGetErrorMessage(hresult),
                    hresult=hresult,
                )
            self.hcontext = None
        CardConnection.__del__(self)

    def connect(self, protocol=None, mode=None, disposition=None):
        """Connect to the card.

        If protocol is not specified, connect with the default
        connection protocol.

        If mode is not specified, connect with
        C{smartcard.scard.SCARD_SHARE_SHARED}."""
        CardConnection.connect(self, protocol)
        pcscprotocol = translateprotocolmask(protocol)
        if 0 == pcscprotocol:
            pcscprotocol = self.getProtocol()

        if mode is None:
            mode = SCARD_SHARE_SHARED

        # store the way to dispose the card
        if disposition is None:
            disposition = SCARD_UNPOWER_CARD
        self.disposition = disposition

        if self.hcontext is None:
            raise CardConnectionException("Context already released")

        hresult, self.hcard, dwActiveProtocol = SCardConnect(
            self.hcontext, str(self.reader), mode, pcscprotocol
        )
        if hresult != SCARD_S_SUCCESS:
            self.hcard = None
            if hresult in (SCARD_W_REMOVED_CARD, SCARD_E_NO_SMARTCARD):
                raise NoCardException("Unable to connect", hresult=hresult)

            raise CardConnectionException(
                "Unable to connect with protocol: "
                + dictProtocol[pcscprotocol]
                + ". "
                + SCardGetErrorMessage(hresult),
                hresult=hresult,
            )

        protocol = 0
        if dwActiveProtocol == SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1:
            # special case for T0 | T1
            # this happens when mode=SCARD_SHARE_DIRECT and no protocol is
            # then negotiated with the card
            protocol = CardConnection.T0_protocol | CardConnection.T1_protocol
        else:
            for p, p_name in dictProtocol.items():
                if p == dwActiveProtocol:
                    protocol = getattr(CardConnection, f"{p_name}_protocol")
        PCSCCardConnection.setProtocol(self, protocol)

    def reconnect(self, protocol=None, mode=None, disposition=None):
        """Reconnect to the card.

        If protocol is not specified, connect with the default
        connection protocol.

        If mode is not specified, connect with
        C{smartcard.scard.SCARD_SHARE_SHARED}.

        If disposition is not specified, do a warm reset
        (C{smartcard.scard.SCARD_RESET_CARD})"""
        CardConnection.reconnect(self, protocol)
        if self.hcard is None:
            raise CardConnectionException("Card not connected")

        pcscprotocol = translateprotocolmask(protocol)
        if 0 == pcscprotocol:
            pcscprotocol = self.getProtocol()

        if mode is None:
            mode = SCARD_SHARE_SHARED

        # store the way to dispose the card
        if disposition is None:
            disposition = SCARD_RESET_CARD
        self.disposition = disposition

        hresult, dwActiveProtocol = SCardReconnect(
            self.hcard, mode, pcscprotocol, self.disposition
        )
        if hresult != SCARD_S_SUCCESS:
            self.hcard = None
            if hresult in (SCARD_W_REMOVED_CARD, SCARD_E_NO_SMARTCARD):
                raise NoCardException("Unable to reconnect", hresult=hresult)

            raise CardConnectionException(
                "Unable to reconnect with protocol: "
                + dictProtocol[pcscprotocol]
                + ". "
                + SCardGetErrorMessage(hresult),
                hresult=hresult,
            )

        protocol = 0
        if dwActiveProtocol == SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1:
            # special case for T0 | T1
            # this happens when mode=SCARD_SHARE_DIRECT and no protocol is
            # then negotiated with the card
            protocol = CardConnection.T0_protocol | CardConnection.T1_protocol
        else:
            for p, p_name in dictProtocol.items():
                if p == dwActiveProtocol:
                    protocol = getattr(CardConnection, f"{p_name}_protocol")
        PCSCCardConnection.setProtocol(self, protocol)

    def disconnect(self):
        """Disconnect from the card."""

        if self.hcard is not None:
            # when __del__() is invoked in response to a module being
            # deleted, e.g., when execution of the program is done,
            # other globals referenced by the __del__() method may
            # already have been deleted.  this causes
            # CardConnection.disconnect to except with a TypeError
            try:
                CardConnection.disconnect(self)
            except TypeError:
                pass
            hresult = SCardDisconnect(self.hcard, self.disposition)
            self.hcard = None
            if hresult != SCARD_S_SUCCESS:
                raise CardConnectionException(
                    "Failed to disconnect: " + SCardGetErrorMessage(hresult),
                    hresult=hresult,
                )

    def getATR(self):
        """Return card ATR"""
        CardConnection.getATR(self)
        if self.hcard is None:
            raise CardConnectionException("Card not connected")
        hresult, _reader, _state, _protocol, atr = SCardStatus(self.hcard)
        if hresult != SCARD_S_SUCCESS:
            raise CardConnectionException(
                "Failed to get status: " + SCardGetErrorMessage(hresult),
                hresult=hresult,
            )
        return atr

    def doTransmit(self, command, protocol=None):
        """Transmit an apdu to the card and return response apdu.

        @param command:  command apdu to transmit (list of bytes)

        @param protocol: the transmission protocol, from
            L{CardConnection.T0_protocol}, L{CardConnection.T1_protocol}, or
            L{CardConnection.RAW_protocol}

        @return:     a tuple (response, sw1, sw2) where
                    - response are the response bytes excluding status words
                    - sw1 is status word 1, e.g. 0x90
                    - sw2 is status word 2, e.g. 0x1A
        """
        if protocol is None:
            protocol = self.getProtocol()
        CardConnection.doTransmit(self, command, protocol)
        pcscprotocolheader = translateprotocolheader(protocol)
        if 0 == pcscprotocolheader:
            raise CardConnectionException(
                "Invalid protocol in transmit: must be "
                + "CardConnection.T0_protocol, "
                + "CardConnection.T1_protocol, or "
                + "CardConnection.RAW_protocol"
            )
        if self.hcard is None:
            raise CardConnectionException("Card not connected")
        hresult, response = SCardTransmit(self.hcard, pcscprotocolheader, command)
        if hresult != SCARD_S_SUCCESS:
            raise CardConnectionException(
                "Failed to transmit with protocol "
                + dictProtocolHeader[pcscprotocolheader]
                + ". "
                + SCardGetErrorMessage(hresult),
                hresult=hresult,
            )

        if len(response) < 2:
            raise CardConnectionException(
                "Card returned no valid response", hresult=hresult
            )

        sw1 = (response[-2] + 256) % 256
        sw2 = (response[-1] + 256) % 256

        data = [(x + 256) % 256 for x in response[:-2]]
        return list(data), sw1, sw2

    def doControl(self, controlCode, command=None):
        """Transmit a control command to the reader and return response.

        @param controlCode: control command

        @param command:     command data to transmit (list of bytes)

        @return:      response are the response bytes (if any)
        """
        if command is None:
            command = []
        CardConnection.doControl(self, controlCode, command)
        hresult, response = SCardControl(self.hcard, controlCode, command)
        if hresult != SCARD_S_SUCCESS:
            raise SmartcardException(
                "Failed to control " + SCardGetErrorMessage(hresult), hresult=hresult
            )

        data = [(x + 256) % 256 for x in response]
        return list(data)

    def doGetAttrib(self, attribId):
        """get an attribute

        @param attribId: Identifier for the attribute to get

        @return:   response are the attribute byte array
        """
        CardConnection.doGetAttrib(self, attribId)
        hresult, response = SCardGetAttrib(self.hcard, attribId)
        if hresult != SCARD_S_SUCCESS:
            raise SmartcardException(
                "Failed to getAttrib " + SCardGetErrorMessage(hresult), hresult=hresult
            )
        return response
