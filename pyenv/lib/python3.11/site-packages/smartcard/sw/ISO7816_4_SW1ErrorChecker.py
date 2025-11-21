"""ISO7816-4 sw1 only error checker.

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

iso7816_4SW1 = {
    0x62: smartcard.sw.SWExceptions.WarningProcessingException,
    0x63: smartcard.sw.SWExceptions.WarningProcessingException,
    0x64: smartcard.sw.SWExceptions.ExecutionErrorException,
    0x65: smartcard.sw.SWExceptions.ExecutionErrorException,
    0x66: smartcard.sw.SWExceptions.SecurityRelatedException,
    0x67: smartcard.sw.SWExceptions.CheckingErrorException,
    0x68: smartcard.sw.SWExceptions.CheckingErrorException,
    0x69: smartcard.sw.SWExceptions.CheckingErrorException,
    0x6A: smartcard.sw.SWExceptions.CheckingErrorException,
    0x6B: smartcard.sw.SWExceptions.CheckingErrorException,
    0x6C: smartcard.sw.SWExceptions.CheckingErrorException,
    0x6D: smartcard.sw.SWExceptions.CheckingErrorException,
    0x6E: smartcard.sw.SWExceptions.CheckingErrorException,
    0x6F: smartcard.sw.SWExceptions.CheckingErrorException,
}


class ISO7816_4_SW1ErrorChecker(ErrorChecker):
    """ISO7816-4 error checker based on status word sw1 only.

    This error checker raises the following exceptions:
      - sw1 sw2
      - 62  any   L{WarningProcessingException}
      - 63  any   L{WarningProcessingException}
      - 64  any   L{ExecutionErrorException}
      - 65  any   L{ExecutionErrorException}
      - 66  any   L{SecurityRelatedException}
      - 67  any   L{CheckingErrorException}
      - 68  any   L{CheckingErrorException}
      - 69  any   L{CheckingErrorException}
      - 6a  any   L{CheckingErrorException}
      - 6b  any   L{CheckingErrorException}
      - 6c  any   L{CheckingErrorException}
      - 6d  any   L{CheckingErrorException}
      - 6e  any   L{CheckingErrorException}
      - 6f  any   L{CheckingErrorException}
    """

    def __call__(self, data, sw1, sw2):
        """Called to test data, sw1 and sw2 for error.

        @param data:       apdu response data
        @param sw1:        apdu data status word 1
        @param sw2:        apdu data status word 2
        """
        if sw1 in iso7816_4SW1:
            exception = iso7816_4SW1[sw1]
            raise exception(data, sw1, sw2)


if __name__ == "__main__":
    # Small sample illustrating the use of ISO7816_4_SW1ErrorChecker.
    ecs = ISO7816_4_SW1ErrorChecker()
    ecs([], 0x90, 0x00)
    try:
        ecs([], 0x66, 0x80)
    except smartcard.sw.SWExceptions.SecurityRelatedException as e:
        print(str(e) + f" {e.sw1:x} {e.sw2:x}")
