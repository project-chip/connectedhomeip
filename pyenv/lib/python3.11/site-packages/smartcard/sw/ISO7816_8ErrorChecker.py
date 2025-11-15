"""ISO7816-8 error checker.

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

import smartcard.sw.SWExceptions
from smartcard.sw.ErrorChecker import ErrorChecker

# pylint: disable=too-few-public-methods

iso7816_8SW = {
    0x63: (
        smartcard.sw.SWExceptions.WarningProcessingException,
        {
            0x00: "Authentication failed",
            0xC0: "PIN verification failed. 0 retries before blocking PIN",
            0xC1: "PIN verification failed. 1 retries before blocking PIN",
            0xC2: "PIN verification failed. 2 retries before blocking PIN",
            0xC3: "PIN verification failed. 3 retries before blocking PIN",
            0xC4: "PIN verification failed. 4 retries before blocking PIN",
            0xC5: "PIN verification failed. 5 retries before blocking PIN",
            0xC6: "PIN verification failed. 6 retries before blocking PIN",
            0xC7: "PIN verification failed. 7 retries before blocking PIN",
            0xC8: "PIN verification failed. 8 retries before blocking PIN",
            0xC9: "PIN verification failed. 9 retries before blocking PIN",
            0xCA: "PIN verification failed. 10 retries before blocking PIN",
            0xCB: "PIN verification failed. 11 retries before blocking PIN",
            0xCC: "PIN verification failed. 12 retries before blocking PIN",
            0xCD: "PIN verification failed. 13 retries before blocking PIN",
            0xCE: "PIN verification failed. 14 retries before blocking PIN",
            0xCF: "PIN verification failed. 15 retries before blocking PIN",
        },
    ),
    0x65: (
        smartcard.sw.SWExceptions.ExecutionErrorException,
        {0x81: "Memory failure (unsuccessful changing)"},
    ),
    0x66: (
        smartcard.sw.SWExceptions.SecurityRelatedException,
        {
            0x00: "The environment cannot be set or modified",
            0x87: "Expected SM data objects missing",
            0x88: "SM data objects incorrect",
        },
    ),
    0x67: (
        smartcard.sw.SWExceptions.CheckingErrorException,
        {0x00: "Wrong length (empty Lc field)"},
    ),
    0x68: (
        smartcard.sw.SWExceptions.CheckingErrorException,
        {0x83: "Final command expected", 0x84: "Command chaining not supported"},
    ),
    0x69: (
        smartcard.sw.SWExceptions.CheckingErrorException,
        {
            0x82: "Security status not satisfied",
            0x83: "Authentication method blocked",
            0x84: "Referenced data invalidated",
            0x85: "Conditions of use not satisfied",
        },
    ),
    0x6A: (
        smartcard.sw.SWExceptions.CheckingErrorException,
        {
            0x81: "Function not supported",
            0x82: "File not found",
            0x86: "Incorrect parameters P1-P2",
            0x88: "Referenced data not found",
        },
    ),
}


class ISO7816_8ErrorChecker(ErrorChecker):
    """ISO7816-8 error checker.

    This error checker raises the following exceptions:
      - sw1 sw2
      - 63  00,c0-cf    L{WarningProcessingException}
      - 65  81          L{ExecutionErrorException}
      - 66  00,87,88    L{SecurityRelatedException}
      - 67  00          L{CheckingErrorException}
      - 68  82,84       L{CheckingErrorException}
      - 69  82,83,84,85 L{CheckingErrorException}
      - 6A  81,82,86,88 L{CheckingErrorException}

    This checker does not raise exceptions on undefined sw1 values, e.g.:
      - sw1 sw2
      - 62  any
      - 6f  any

    and on undefined sw2 values, e.g.:
      - sw1 sw2
      - 66  81 82
      - 67  any except 00


    Use another checker in the error checking chain, e.g., the
    L{ISO7816_4_SW1ErrorChecker} or L{ISO7816_4ErrorChecker}, to raise
    exceptions on these undefined values.
    """

    def __call__(self, data, sw1, sw2):
        """Called to test data, sw1 and sw2 for error.

        Derived classes must raise a L{smartcard.sw.SWExceptions} upon error.

        @param data:       apdu response data
        @param sw1:        apdu data status word 1
        @param sw2:        apdu data status word 2
        """

        # pylint: disable=duplicate-code

        if sw1 in iso7816_8SW:
            exception, sw2dir = iso7816_8SW[sw1]
            if isinstance(sw2dir, dict):
                try:
                    message = sw2dir[sw2]
                    raise exception(data, sw1, sw2, message)
                except KeyError:
                    pass


if __name__ == "__main__":
    # Small sample illustrating the use of ISO7816_8ErrorChecker.
    ecs = ISO7816_8ErrorChecker()
    ecs([], 0x90, 0x00)
    ecs([], 0x6A, 0x83)
    try:
        ecs([], 0x66, 0x87)
    except smartcard.sw.SWExceptions.SecurityRelatedException as e:
        print(str(e) + f" {e.sw1:x} {e.sw2:x}")
