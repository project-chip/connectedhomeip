"""ATR class managing some of the Answer To Reset content.

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

import functools
import operator
import warnings

from smartcard.Exceptions import SmartcardException


class ATR:
    """Parse and represent Answer to Reset sequences.

    Answer to Reset sequences are defined in ISO 7816-3, section 8.
    """

    # pylint: disable=too-many-instance-attributes
    # pylint: disable=too-many-public-methods

    clockrateconversion: list[int | str] = [
        372,
        372,
        558,
        744,
        1116,
        1488,
        1860,
        "RFU",
        "RFU",
        512,
        768,
        1024,
        1536,
        2048,
        "RFU",
        "RFU",
    ]
    bitratefactor: list[int | str] = [
        "RFU",
        1,
        2,
        4,
        8,
        16,
        32,
        64,
        12,
        20,
        "RFU",
        "RFU",
        "RFU",
        "RFU",
        "RFU",
        "RFU",
    ]
    currenttable: list[int | str] = [25, 50, 100, "RFU"]

    def __init__(self, atr: list[int]) -> None:
        """Parse ATR and initialize members:

          - TS: initial character
          - T0: format character
          - TA[n], TB[n], TC[n], TD[n], for n=0,1,...: protocol parameters

        @note: protocol parameters indices start at 0, e.g.
        TA[0], TA[1] correspond to the ISO standard TA1, TA2
        parameters

          - historicalBytes: the ATR T1, T2, ..., TK historical bytes
          - TCK: checksum byte (only for protocols different from T=0)
          - FI: clock rate conversion factor
          - DI: voltage adjustment factor
          - PI1: programming voltage factor
          - II: maximum programming current factor
          - N: extra guard time
        """

        # pylint: disable=too-many-statements

        if len(atr) < 2:
            raise SmartcardException("ATR sequences must be at least 2 bytes long")
        if atr[0] not in {0x3B, 0x3F}:
            raise SmartcardException(f"invalid TS 0x{atr[0]:02x}")

        self.atr = atr

        # initial character
        self.TS = self.atr[0]

        # format character
        self.T0 = self.atr[1]

        # count of historical bytes
        self.K = self.T0 & 0x0F

        # initialize optional characters lists
        self.TA: list[None | int] = []
        self.TB: list[None | int] = []
        self.TC: list[None | int] = []
        self.TD: list[None | int] = []
        self.Y: list[int] = []

        td: None | int = self.T0
        offset = 1
        while td is not None:
            self.Y.append(td >> 4 & 0x0F)

            self.TA += [None]
            self.TB += [None]
            self.TC += [None]
            self.TD += [None]

            if self.Y[-1] & 0x01:  # TA
                offset += 1
                self.TA[-1] = self.atr[offset]
            if self.Y[-1] & 0x02:  # TB
                offset += 1
                self.TB[-1] = self.atr[offset]
            if self.Y[-1] & 0x04:  # TC
                offset += 1
                self.TC[-1] = self.atr[offset]
            if self.Y[-1] & 0x08:  # TD
                offset += 1
                self.TD[-1] = self.atr[offset]

            td = self.TD[-1]

        self.interfaceBytesCount = offset - 1

        # historical bytes
        self.historicalBytes = self.atr[offset + 1 : offset + 1 + self.K]

        # checksum
        self.TCK: int | None = None
        self.checksumOK: bool | None = None
        self.hasChecksum = len(self.atr) == offset + 1 + self.K + 1
        if self.hasChecksum:
            self.TCK = self.atr[-1]
            self.checksumOK = functools.reduce(operator.xor, self.atr[1:]) == 0

        # clock-rate conversion factor
        self.FI: int | None = None
        if self.TA[0] is not None:
            self.FI = self.TA[0] >> 4 & 0x0F

        # bit-rate adjustment factor
        self.DI: int | None = None
        if self.TA[0] is not None:
            self.DI = self.TA[0] & 0x0F

        # maximum programming current factor
        self.II: int | None = None
        if self.TB[0] is not None:
            self.II = self.TB[0] >> 5 & 0x03

        # programming voltage factor
        self.PI1: int | None = None
        if self.TB[0] is not None:
            self.PI1 = self.TB[0] & 0x1F

        # extra guard time
        self.N = self.TC[0]

    @property
    def hasTA(self) -> list[bool]:
        """Deprecated. Replace usage with `ATR.TA[i] is not None`."""

        warnings.warn("Replace usage with `ATR.TA[i] is not None`", DeprecationWarning)
        return [ta is not None for ta in self.TA]

    @property
    def hasTB(self) -> list[bool]:
        """Deprecated. Replace usage with `ATR.TB[i] is not None`."""

        warnings.warn("Replace usage with `ATR.TB[i] is not None`", DeprecationWarning)
        return [tb is not None for tb in self.TB]

    @property
    def hasTC(self) -> list[bool]:
        """Deprecated. Replace usage with `ATR.TC[i] is not None`."""

        warnings.warn("Replace usage with `ATR.TC[i] is not None`", DeprecationWarning)
        return [tc is not None for tc in self.TC]

    @property
    def hasTD(self) -> list[bool]:
        """Deprecated. Replace usage with `ATR.TD[i] is not None`."""

        warnings.warn("Replace usage with `ATR.TD[i] is not None`", DeprecationWarning)
        return [td is not None for td in self.TD]

    def getChecksum(self) -> int | None:
        """Return the checksum of the ATR. Checksum is mandatory only
        for T=1."""
        return self.TCK

    def getHistoricalBytes(self) -> list[int]:
        """Return historical bytes."""
        return self.historicalBytes

    def getHistoricalBytesCount(self) -> int:
        """Return count of historical bytes."""
        return len(self.historicalBytes)

    def getInterfaceBytesCount(self) -> int:
        """Return count of interface bytes."""
        return self.interfaceBytesCount

    def getTA1(self) -> int | None:
        """Return TA1 byte."""
        return self.TA[0]

    def getTB1(self) -> int | None:
        """Return TB1 byte."""
        return self.TB[0]

    def getTC1(self) -> int | None:
        """Return TC1 byte."""
        return self.TC[0]

    def getTD1(self) -> int | None:
        """Return TD1 byte."""
        return self.TD[0]

    def getBitRateFactor(self) -> int | str:
        """Return bit rate factor."""
        if self.DI is not None:
            return ATR.bitratefactor[self.DI]
        return 1

    def getClockRateConversion(self) -> int | str:
        """Return clock rate conversion."""
        if self.FI is not None:
            return ATR.clockrateconversion[self.FI]
        return 372

    def getProgrammingCurrent(self) -> int | str:
        """Return maximum programming current."""
        if self.II is not None:
            return ATR.currenttable[self.II]
        return 50

    def getProgrammingVoltage(self) -> int:
        """Return programming voltage."""
        if self.PI1 is not None:
            return 5 * (1 + self.PI1)
        return 5

    def getGuardTime(self) -> int | None:
        """Return extra guard time."""
        return self.N

    def getSupportedProtocols(self) -> dict[str, bool]:
        """Returns a dictionary of supported protocols."""
        protocols: dict[str, bool] = {}
        for td in self.TD:
            if td is not None:
                protocols[f"T={td & 0x0F}"] = True
        if self.TD[0] is None:
            protocols["T=0"] = True
        return protocols

    def isT0Supported(self) -> bool:
        """Return True if T=0 is supported."""
        return "T=0" in self.getSupportedProtocols()

    def isT1Supported(self) -> bool:
        """Return True if T=1 is supported."""
        return "T=1" in self.getSupportedProtocols()

    def isT15Supported(self) -> bool:
        """Return True if T=15 is supported."""
        return "T=15" in self.getSupportedProtocols()

    def render(self) -> str:
        """Render the ATR to a readable format."""

        lines: list[str] = []
        enumerated_tx_values = enumerate(zip(self.TA, self.TB, self.TC, self.TD), 1)
        for i, (ta, tb, tc, td) in enumerated_tx_values:
            if ta is not None:
                lines.append(f"TA{i}: {ta:x}")
            if tb is not None:
                lines.append(f"TB{i}: {tb:x}")
            if tc is not None:
                lines.append(f"TC{i}: {tc:x}")
            if td is not None:
                lines.append(f"TD{i}: {td:x}")

        lines.append(f"supported protocols {','.join(self.getSupportedProtocols())}")
        lines.append(f"T=0 supported: {self.isT0Supported()}")
        lines.append(f"T=1 supported: {self.isT1Supported()}")

        if self.getChecksum() is not None:
            lines.append(f"checksum: {self.getChecksum()}")

        lines.append(f"\tclock rate conversion factor: {self.getClockRateConversion()}")
        lines.append(f"\tbit rate adjustment factor: {self.getBitRateFactor()}")
        lines.append(f"\tmaximum programming current: {self.getProgrammingCurrent()}")
        lines.append(f"\tprogramming voltage: {self.getProgrammingVoltage()}")
        lines.append(f"\tguard time: {self.getGuardTime()}")
        lines.append(f"nb of interface bytes: {self.getInterfaceBytesCount()}")
        lines.append(f"nb of historical bytes: {self.getHistoricalBytesCount()}")

        return "\n".join(lines)

    def dump(self) -> None:
        """Deprecated. Replace usage with `print(ATR.render())`"""

        warnings.warn("Replace usage with `print(ATR.render())`", DeprecationWarning)
        print(self.render())

    def __str__(self) -> str:
        """Render the ATR as a space-separated string of uppercase hexadecimal pairs."""

        return bytes(self.atr).hex(" ").upper()
