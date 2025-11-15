"""ISO7816-4 error checking strategy.

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

iso7816_4SW = {
    0x62: (
        smartcard.sw.SWExceptions.WarningProcessingException,
        {
            0x00: "Response padded/ More APDU commands expected",
            0x81: "Part of returned data may be corrupted",
            0x82: "End of file/record reached before reading Le bytes",
            0x83: "File invalidated",
            0x84: "FCI not correctly formatted",
            0xFF: "Correct execution, response padded",
        },
    ),
    0x63: (
        smartcard.sw.SWExceptions.WarningProcessingException,
        {
            0x00: "Authentication failed",
            0x81: "File filled up by the last write",
            0xC0: "PIN verification failed. 0 tries before blocking PIN",
            0xC1: "PIN verification failed. 1 tries before blocking PIN",
            0xC2: "PIN verification failed. 2 tries before blocking PIN",
            0xC3: "PIN verification failed. 3 tries before blocking PIN",
            0xC4: "PIN verification failed. 4 tries before blocking PIN",
            0xC5: "PIN verification failed. 5 tries before blocking PIN",
            0xC6: "PIN verification failed. 6 tries before blocking PIN",
            0xC7: "PIN verification failed. 7 tries before blocking PIN",
            0xC8: "PIN verification failed. 8 tries before blocking PIN",
            0xC9: "PIN verification failed. 9 tries before blocking PIN",
            0xCA: "PIN verification failed. 10 tries before blocking PIN",
            0xCB: "PIN verification failed. 11 tries before blocking PIN",
            0xCC: "PIN verification failed. 12 tries before blocking PIN",
            0xCD: "PIN verification failed. 13 tries before blocking PIN",
            0xCE: "PIN verification failed. 14 tries before blocking PIN",
            0xCF: "PIN verification failed. 15 tries before blocking PIN",
        },
    ),
    0x64: (
        smartcard.sw.SWExceptions.ExecutionErrorException,
        {0x00: "Integrity error detected in EEPROM"},
    ),
    0x67: (
        smartcard.sw.SWExceptions.CheckingErrorException,
        {0x00: "Wrong length in Lc"},
    ),
    0x68: (
        smartcard.sw.SWExceptions.CheckingErrorException,
        {0x81: "Logical channel not supported", 0x82: "Secure messaging not supported"},
    ),
    0x69: (
        smartcard.sw.SWExceptions.CheckingErrorException,
        {
            0x81: "Command incompatible with file structure.",
            0x82: "Security status not satisfied",
            0x83: "Authentication method blocked",
            0x84: "Referenced data invalid",
            0x85: "Conditions of use not satisfied",
            0x86: "Command not allowed (no current EF)",
            0x87: "Secure messaging data object missing.",
            0x88: "Secure messaging data object incorrect",
        },
    ),
    0x6A: (
        smartcard.sw.SWExceptions.CheckingErrorException,
        {
            0x80: "Incorrect parameters in the data field",
            0x81: "Function not supported",
            0x82: "File not found",
            0x83: "Record not found",
            0x84: "Not enough memory space in the file",
            0x85: "Lc inconsistent with TLV structure",
            0x86: "Incorrect parameters P1-P2",
            0x87: "Lc is inconsistent with P1-P2",
            0x88: "Referenced data not found",
        },
    ),
    0x6B: (
        smartcard.sw.SWExceptions.CheckingErrorException,
        {0x00: "Incorrect parameters P1-P2"},
    ),
    0x6D: (
        smartcard.sw.SWExceptions.CheckingErrorException,
        {0x00: "Instruction (INS) not supported"},
    ),
    0x6E: (
        smartcard.sw.SWExceptions.CheckingErrorException,
        {0x00: "Class (CLA) not supported"},
    ),
    0x6F: (smartcard.sw.SWExceptions.CheckingErrorException, {0x00: "Fatal error"}),
}


class ISO7816_4ErrorChecker(ErrorChecker):
    """ISO7816-4 error checking strategy.

    This strategy raises the following exceptions:
      - sw1 sw2
      - 62  00 81 82 83 84 FF   WarningProcessingException
      - 63  00 81 C0->CF        WarningProcessingException
      - 64  00                  ExecutionErrorException
      - 67  00                  CheckingErrorException
      - 68  81 82               CheckingErrorException
      - 69  81->88  99? c1?     CheckingErrorException
      - 6a  80->88              CheckingErrorException
      - 6b  00                  CheckingErrorException
      - 6d  00                  CheckingErrorException
      - 6e  00                  CheckingErrorException
      - 6f  00                  CheckingErrorException

    This checker does not raise exceptions on undefined sw1 values, e.g.:
      - sw1 sw2
      - 65  any
      - 66  any
      - 6c  any

    and on undefined sw2 values, e.g.:
      - sw1 sw2
      - 62  80 85
      - 6b  any except 00


    Use another checker in the error checking chain, e.g., the
    ISO7816_4SW1ErrorChecker, to raise exceptions on these undefined
    values.
    """

    def __call__(self, data, sw1, sw2):
        """Called to test data, sw1 and sw2 for error.

        Derived classes must raise a L{smartcard.sw.SWExceptions} upon error.

        @param data:       apdu response data
        @param sw1:        apdu data status words
        @param sw2:        apdu data status words
        """

        # pylint: disable=duplicate-code

        if sw1 in iso7816_4SW:
            exception, sw2dir = iso7816_4SW[sw1]
            if isinstance(sw2dir, dict):
                try:
                    message = sw2dir[sw2]
                    raise exception(data, sw1, sw2, message)
                except KeyError:
                    pass


if __name__ == "__main__":
    # Small sample illustrating the use of ISO7816_4ErrorChecker.
    ecs = ISO7816_4ErrorChecker()
    ecs([], 0x90, 0x00)
    try:
        ecs([], 0x6B, 0x00)
    except smartcard.sw.SWExceptions.CheckingErrorException as e:
        print(str(e) + f" {e.sw1:x} {e.sw2:x}")
