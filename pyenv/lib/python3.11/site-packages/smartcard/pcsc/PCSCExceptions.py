"""Smartcard module exceptions.

This module defines the exceptions raised by the smartcard.pcsc modules.

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

# gemalto scard library
import smartcard.scard


class BaseSCardException(Exception):
    """Base class for scard (aka PCSC) exceptions.

    scard exceptions are raised by the scard module, i.e.
    low-level PCSC access to readers and cards.

    """

    def __init__(self, *args, hresult=-1, message=""):
        """Constructor that stores the pcsc error status."""
        if not message:
            message = "scard exception"
        if -1 == hresult and len(args) > 0:
            hresult = args[0]
            args = args[1:]
        super().__init__(message, *args)
        self.message = message
        self.hresult = hresult

    def __str__(self):
        """Returns a string representation of the exception."""
        text = super().__str__()
        if self.hresult != -1:
            hresult = self.hresult
            if hresult < 0:
                # convert 0x-7FEFFFE3 into 0x8010001D
                hresult += 0x100000000
            text += f": {smartcard.scard.SCardGetErrorMessage(self.hresult)} (0x{hresult:08X})"
        return text


class AddReaderToGroupException(BaseSCardException):
    """Raised when scard fails to add a new reader to a PCSC reader group."""

    def __init__(self, hresult, readername="", groupname=""):
        super().__init__(
            message="Failed to add reader: " + readername + " to group: " + groupname,
            hresult=hresult,
        )
        self.readername = readername
        self.groupname = groupname


class EstablishContextException(BaseSCardException):
    """Raised when scard failed to establish context with PCSC."""

    def __init__(self, hresult):
        super().__init__(message="Failed to establish context", hresult=hresult)


class ListReadersException(BaseSCardException):
    """Raised when scard failed to list readers."""

    def __init__(self, hresult):
        super().__init__(message="Failed to list readers", hresult=hresult)


class IntroduceReaderException(BaseSCardException):
    """Raised when scard fails to introduce a new reader to PCSC."""

    def __init__(self, hresult, readername=""):
        super().__init__(
            message="Failed to introduce a new reader: " + readername, hresult=hresult
        )
        self.readername = readername


class ReleaseContextException(BaseSCardException):
    """Raised when scard failed to release PCSC context."""

    def __init__(self, hresult):
        super().__init__(message="Failed to release context", hresult=hresult)


class RemoveReaderFromGroupException(BaseSCardException):
    """Raised when scard fails to remove a reader from a PCSC reader group."""

    def __init__(self, hresult, readername="", groupname=""):
        BaseSCardException.__init__(self, hresult)
        self.readername = readername
        self.groupname = groupname
        super().__init__(
            message="Failed to remove reader: "
            + readername
            + " from group: "
            + groupname,
            hresult=hresult,
        )


if __name__ == "__main__":
    try:
        raise EstablishContextException(smartcard.scard.SCARD_E_NO_MEMORY)
    except BaseSCardException as exc:
        print(exc)
