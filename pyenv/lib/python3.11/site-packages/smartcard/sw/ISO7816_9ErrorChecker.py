"""ISO7816-9 error checker.

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

iso7816_9SW = {
    0x62: (
        smartcard.sw.SWExceptions.WarningProcessingException,
        {0x82: "End of file/record reached"},
    ),
    0x64: (
        smartcard.sw.SWExceptions.ExecutionErrorException,
        {0x00: "Execution error"},
    ),
    0x69: (
        smartcard.sw.SWExceptions.CheckingErrorException,
        {0x82: "Security status not satisfied"},
    ),
    0x6A: (
        smartcard.sw.SWExceptions.CheckingErrorException,
        {
            0x80: "Incorrect parameters in data field",
            0x84: "Not enough memory space",
            0x89: "File already exists",
            0x8A: "DF name already exists",
        },
    ),
}


class ISO7816_9ErrorChecker(ErrorChecker):
    """ISO7816-8 error checker.

    This error checker raises the following exceptions:
      - sw1 sw2
      - 62  82          WarningProcessingException
      - 64  00          ExecutionErrorException
      - 69  82          CheckingErrorException
      - 6A  80,84,89,8A CheckingErrorException

    This checker does not raise exceptions on undefined sw1 values, e.g.:
      - sw1 sw2
      - 63  any
      - 6F  any

    and on undefined sw2 values, e.g.:
      - sw1 sw2
      - 62  81 83
      - 64  any except 00


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

        if sw1 in iso7816_9SW:
            exception, sw2dir = iso7816_9SW[sw1]
            if isinstance(sw2dir, dict):
                try:
                    message = sw2dir[sw2]
                    raise exception(data, sw1, sw2, message)
                except KeyError:
                    pass


if __name__ == "__main__":
    # Small sample illustrating the use of ISO7816_9ErrorChecker.
    ecs = ISO7816_9ErrorChecker()
    ecs([], 0x90, 0x00)
    ecs([], 0x6A, 0x81)
    try:
        ecs([], 0x6A, 0x8A)
    except smartcard.sw.SWExceptions.CheckingErrorException as e:
        print(str(e) + f" {e.sw1:x} {e.sw2:x}")
