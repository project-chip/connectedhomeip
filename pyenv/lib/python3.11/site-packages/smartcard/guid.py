"""smartcard.guid

Utility functions to handle GUIDs as strings or list of bytes

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

import uuid


def strToGUID(s: str) -> list[int]:
    """Converts a GUID string into a list of bytes.

    >>> strToGUID('{AD4F1667-EA75-4124-84D4-641B3B197C65}')
    [103, 22, 79, 173, 117, 234, 36, 65, 132, 212, 100, 27, 59, 25, 124, 101]
    """
    return list(uuid.UUID(s).bytes_le)


def GUIDToStr(g: list[int]) -> str:
    """Converts a GUID sequence of bytes into a string.

    >>> GUIDToStr([103,22,79,173,  117,234,  36,65,
    ...            132, 212, 100, 27, 59, 25, 124, 101])
    '{AD4F1667-EA75-4124-84D4-641B3B197C65}'
    """
    return f"{{{uuid.UUID(bytes_le=bytes(g))}}}".upper()
