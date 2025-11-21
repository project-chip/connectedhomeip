"""smartcard.util package

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

from __future__ import annotations

import warnings

PACK = 1
HEX = 2
UPPERCASE = 4
COMMA = 8


def padd(bytelist: list[int], length: int, padding: str = "FF"):
    """Padds a byte list with a constant byte value (default is x0FF)
    @param bytelist: the byte list to padd
    @param length: the total length of the resulting byte list;
                  no padding if length is smaller than the byte list length
    @param padding: padding value (default is 0xff)

    @return: the padded bytelist

    >>> padd([59, 101, 0, 0, 156, 17, 1, 1, 3], 16)
    [59, 101, 0, 0, 156, 17, 1, 1, 3, 255, 255, 255, 255, 255, 255, 255]
    >>> padd([59, 101, 0, 0, 156, 17, 1, 1, 3], 12, '80')
    [59, 101, 0, 0, 156, 17, 1, 1, 3, 128, 128, 128]
    >>> padd([59, 101, 0, 0, 156, 17, 1, 1, 3], 8)
    [59, 101, 0, 0, 156, 17, 1, 1, 3]
    """

    return bytelist + [int(padding, 16)] * (length - len(bytelist))


def toASCIIBytes(stringtoconvert: str) -> list[int]:
    """Convert a string to a list of UTF-8 encoded bytes.

    @param stringtoconvert: the string to convert into a byte list

    @return: a byte list of the ASCII codes of the string characters

    L{toASCIIBytes()} is the reverse of L{toASCIIString()}

    >>> toASCIIBytes("Number 101")
    [78, 117, 109, 98, 101, 114, 32, 49, 48, 49]
    """

    return list(stringtoconvert.encode("utf-8"))


def toASCIIString(bytelist: list[int]) -> str:
    """Convert a list of integers in the range ``[32, 127]`` to a string.

    Integer values outside the range ``[32, 127]`` are replaced with a period.

    @param bytelist: list of ASCII bytes to convert into a string

    @return: a string from the ASCII code list

    L{toASCIIString()} is the reverse of L{toASCIIBytes()}

    >>> toASCIIString([0x4E,0x75,0x6D,0x62,0x65,0x72,0x20,0x31,0x30,0x31])
    'Number 101'
    >>> toASCIIString([0x01, 0x20, 0x80, 0x7E, 0xF0])
    ". .~."
    """

    return "".join(chr(c) if 32 <= c <= 127 else "." for c in bytelist)


def toBytes(bytestring: str) -> list[int]:
    """Convert a string of hexadecimal characters to a list of integers.

    @param bytestring: a byte string

    >>> toBytes("3B 65 00 00 9C 11 01 01 03")
    [59, 101, 0, 0, 156, 17, 1, 1, 3]
    >>> toBytes("3B6500009C11010103")
    [59, 101, 0, 0, 156, 17, 1, 1, 3]
    >>> toBytes("3B6500   009C1101  0103")
    [59, 101, 0, 0, 156, 17, 1, 1, 3]
    """

    try:
        return list(bytes.fromhex(bytestring))
    except ValueError as exc:
        raise TypeError("not a string representing a list of bytes") from exc


# GSM3.38 character conversion table.
__dic_GSM_3_38__ = {
    "@": 0x00,  # @ At symbol
    "£": 0x01,  # £ Britain pound symbol
    "$": 0x02,  # $ Dollar symbol
    "¥": 0x03,  # ¥ Yen symbol
    "è": 0x04,  # è e accent grave
    "é": 0x05,  # é e accent aigu
    "ù": 0x06,  # ù u accent grave
    "ì": 0x07,  # ì i accent grave
    "ò": 0x08,  # ò o accent grave
    "Ç": 0x09,  # Ç C majuscule cedille
    "\n": 0x0A,  # LF Line Feed
    "Ø": 0x0B,  # Ø O majuscule barré
    "ø": 0x0C,  # ø o minuscule barré
    "\r": 0x0D,  # CR Carriage Return
    "Å": 0x0E,  # Å Angstroem majuscule
    "å": 0x0F,  # å Angstroem minuscule
    "Δ": 0x10,  # Δ Greek letter delta
    "_": 0x11,  # underscore
    "Φ": 0x12,  # Φ Greek letter phi
    "Γ": 0x13,  # Γ Greek letter gamma
    "Λ": 0x14,  # Λ Greek letter lambda
    "Ω": 0x15,  # Ω Greek letter omega
    "Π": 0x16,  # Π Greek letter pi
    "Ψ": 0x17,  # Ψ Greek letter psi
    "Σ": 0x18,  # Σ Greek letter sigma
    "Θ": 0x19,  # Θ Greek letter theta
    "Ξ": 0x1A,  # Ξ Greek letter xi
    #    0x1B               maps to extension table
    "Æ": 0x1C,  # Æ majuscule ae
    "æ": 0x1D,  # æ minuscule ae
    "ß": 0x1E,  # ß s dur allemand
    "É": 0x1F,  # É majuscule é
    " ": 0x20,
    "!": 0x21,
    '"': 0x22,  # guillemet
    "#": 0x23,
    "¤": 0x24,  # ¤ carré
    #
    #    0x25 ... 0x3F    # % ... ?
    #
    "¡": 0x40,  # ¡ point d'exclamation renversé
    #
    #    0x41 ... 0x5A    # A ... Z
    #
    "Ä": 0x5B,  # Ä majuscule A trema
    "Ö": 0x5C,  # Ö majuscule O trema
    "Ñ": 0x5D,  # Ñ majuscule N tilde espagnol
    "Ü": 0x5E,  # Ü majuscule U trema
    "§": 0x5F,  # § signe paragraphe
    "¿": 0x60,  # ¿ point interrogation renversé
    #
    #    0x61 ... 0x7A    # a ... z
    #
    "ä": 0x7B,  # ä minuscule a trema
    "ö": 0x7C,  # ö minuscule o trema
    "ñ": 0x7D,  # ñ minuscule n tilde espagnol
    "ü": 0x7E,  # ü minuscule u trema
    "à": 0x7F,  # à a accent grave
}


def toGSM3_38Bytes(stringtoconvert: str | bytes) -> list[int]:
    """Returns a list of bytes from a string using GSM 3.38 conversion table.

    @param stringtoconvert:     string to convert

    @return: a list of bytes

    >>> toGSM3_38Bytes("@ùPascal")
    [0, 6, 80, 97, 115, 99, 97, 108]
    """
    if isinstance(stringtoconvert, bytes):
        stringtoconvert = stringtoconvert.decode("iso8859-1")

    result = []
    for char in stringtoconvert:
        if ("%" <= char <= "?") or ("A" <= char <= "Z") or ("a" <= char <= "z"):
            result.append(ord(char))
        else:
            result.append(__dic_GSM_3_38__[char])
    return result


def toHexString(data: list[int] | None = None, output_format: int = 0) -> str:
    """Convert a list of integers to a formatted string of hexadecimal.

    Integers larger than 255 will be truncated to two-byte hexadecimal pairs.

    @param data:   a list of bytes to stringify,
                e.g. [59, 22, 148, 32, 2, 1, 0, 0, 13]
    @param output_format: a logical OR of
      - COMMA: add a comma between bytes
      - HEX: add the 0x chars before bytes
      - UPPERCASE: use 0X before bytes (need HEX)
      - PACK: remove blanks

    >>> vals = [0x3B, 0x65, 0x00, 0x00, 0x9C, 0x11, 0x01, 0x01, 0x03]
    >>> toHexString(vals)
    '3B 65 00 00 9C 11 01 01 03'
    >>> toHexString(vals, COMMA)
    '3B, 65, 00, 00, 9C, 11, 01, 01, 03'
    >>> toHexString(vals, HEX)
    '0x3B 0x65 0x00 0x00 0x9C 0x11 0x01 0x01 0x03'
    >>> toHexString(vals, HEX | COMMA)
    '0x3B, 0x65, 0x00, 0x00, 0x9C, 0x11, 0x01, 0x01, 0x03'
    >>> toHexString(vals, PACK)
    '3B6500009C11010103'
    >>> toHexString(vals, HEX | UPPERCASE)
    '0X3B 0X65 0X00 0X00 0X9C 0X11 0X01 0X01 0X03'
    >>> toHexString(vals, HEX | UPPERCASE | COMMA)
    '0X3B, 0X65, 0X00, 0X00, 0X9C, 0X11, 0X01, 0X01, 0X03'
    """

    if not (data is None or isinstance(data, list)):
        raise TypeError("not a list of bytes")

    if not data:
        return ""

    pformat = "%-0.2X"
    separator = ""
    if COMMA & output_format:
        separator = ","
    if not PACK & output_format:
        separator += " "
    if HEX & output_format:
        if UPPERCASE & output_format:
            pformat = "0X" + pformat
        else:
            pformat = "0x" + pformat
    return separator.join(pformat % (a & 0xFF) for a in data).rstrip()


def HexListToBinString(hexlist: list[int]) -> str:
    """Deprecated. Use `bytes(hexlist).decode("utf-8")` or similar.

    >>> HexListToBinString([78, 117, 109, 98, 101, 114, 32, 49, 48, 49])
    'Number 101'
    """

    warnings.warn(
        'Use `bytes(hexlist).decode("utf-8")` or similar.',
        DeprecationWarning,
    )
    return bytes(hexlist).decode("utf-8")


def BinStringToHexList(binstring: str) -> list[int]:
    """Deprecated. Use `list(binstring.encode("utf-8"))` or similar.

    >>> BinStringToHexList("Number 101")
    [78, 117, 109, 98, 101, 114, 32, 49, 48, 49]
    """

    warnings.warn(
        'Use `list(binstring.encode("utf-8"))` or similar.',
        DeprecationWarning,
    )
    return list(binstring.encode("utf-8"))


hl2bs = HexListToBinString
bs2hl = BinStringToHexList
