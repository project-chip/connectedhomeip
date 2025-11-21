"""Open Platform 2.1 error checker.

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

op21_SW = {
    0x62: (
        smartcard.sw.SWExceptions.WarningProcessingException,
        {0x83: "Card life cycle is CARD_LOCKED"},
    ),
    0x63: (
        smartcard.sw.SWExceptions.WarningProcessingException,
        {0x00: "Authentication failed"},
    ),
    0x64: (
        smartcard.sw.SWExceptions.ExecutionErrorException,
        {0x00: "Execution error"},
    ),
    0x65: (smartcard.sw.SWExceptions.ExecutionErrorException, {0x81: "Memory failure"}),
    0x67: (
        smartcard.sw.SWExceptions.CheckingErrorException,
        {0x00: "Wrong length in Lc"},
    ),
    0x69: (
        smartcard.sw.SWExceptions.CheckingErrorException,
        {
            0x82: "Security status not satisfied",
            0x85: "Conditions of use not satisfied",
        },
    ),
    0x6A: (
        smartcard.sw.SWExceptions.CheckingErrorException,
        {
            0x80: "Incorrect values in command data",
            0x81: "Function not supported",
            0x82: "Application not found",
            0x84: "Not enough memory space",
            0x86: "Incorrect parameters P1-P2",
            0x88: "Referenced data not found",
        },
    ),
    0x6D: (
        smartcard.sw.SWExceptions.CheckingErrorException,
        {0x00: "Instruction not supported"},
    ),
    0x6E: (
        smartcard.sw.SWExceptions.CheckingErrorException,
        {0x00: "Class not supported"},
    ),
    0x94: (
        smartcard.sw.SWExceptions.CheckingErrorException,
        {0x84: "Algorithm not supported", 0x85: "Invalid key check value"},
    ),
}


class op21_ErrorChecker(ErrorChecker):
    """Open platform 2.1 error checker.

    This error checker raises the following exceptions:
      - sw1 sw2
      - 62  83                  L{WarningProcessingException}
      - 63  00                  L{WarningProcessingException}
      - 64  00                  L{ExecutionErrorException}
      - 65  81                  L{ExecutionErrorException}
      - 67  00                  L{CheckingErrorException}
      - 69  82 85               L{CheckingErrorException}
      - 6A  80 81 82 84 86 88   L{CheckingErrorException}
      - 6D  00                  L{CheckingErrorException}
      - 6E  00                  L{CheckingErrorException}
      - 94  84 85               L{CheckingErrorException}

    This checker does not raise exceptions on undefined sw1 values, e.g.:
      - sw1 sw2
      - 63  any
      - 6F  any

    and on undefined sw2 values, e.g.:
      - sw1 sw2
      - 62  81 83
      - 64  any except 00


    Use another checker in the error checking chain to raise exceptions
    on these undefined values.
    """

    def __call__(self, data, sw1, sw2):
        """Called to test data, sw1 and sw2 for error.

        Derived classes must raise a L{smartcard.sw.SWExceptions} upon error.

        @param data:       apdu response data
        @param sw1:        apdu data status word 1
        @param sw2:        apdu data status word 2
        """
        if sw1 in op21_SW:
            exception, sw2dir = op21_SW[sw1]
            if isinstance(sw2dir, dict):
                try:
                    message = sw2dir[sw2]
                    raise exception(data, sw1, sw2, message)
                except KeyError:
                    pass


if __name__ == "__main__":
    # Small sample illustrating the use of op21_ErrorChecker.
    ecs = op21_ErrorChecker()
    ecs([], 0x90, 0x00)
    ecs([], 0x94, 0x81)
    try:
        ecs([], 0x94, 0x84)
    except smartcard.sw.SWExceptions.CheckingErrorException as e:
        print(str(e) + f"{e.sw1:x} {e.sw2:x}")
