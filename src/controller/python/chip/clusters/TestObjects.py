# SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0


from dataclasses import dataclass
import typing
from .ClusterObjects import ClusterObjectDescriptor, ClusterCommand
from chip import ChipUtility
from enum import IntEnum


@dataclass
class OnOff:
    id: typing.ClassVar[int] = 0x0006

    class OnOffDelayedAllOffEffectVariant(IntEnum):
        kFadeToOffIn0p8Seconds = 0x00
        kNoFade = 0x01
        k50PercentDimDownIn0p8SecondsThenFadeToOffIn12Seconds = 0x02

    class OnOffDyingLightEffectVariant(IntEnum):
        k20PercenterDimUpIn0p5SecondsThenFadeToOffIn1Second = 0x00

    class OnOffEffectIdentifier(IntEnum):
        kDelayedAllOff = 0x00
        kDyingLight = 0x01

    class Commands:
        @dataclass
        class Off(ClusterCommand):
            cluster_id: typing.ClassVar[int] = 0x0006
            command_id: typing.ClassVar[int] = 0x0000

            @ChipUtility.classproperty
            def descriptor(cls) -> ClusterObjectDescriptor:
                return ClusterObjectDescriptor(Fields=[])

        @dataclass
        class On(ClusterCommand):
            cluster_id: typing.ClassVar[int] = 0x0006
            command_id: typing.ClassVar[int] = 0x0001

            @ChipUtility.classproperty
            def descriptor(cls) -> ClusterObjectDescriptor:
                return ClusterObjectDescriptor(Fields=[])
