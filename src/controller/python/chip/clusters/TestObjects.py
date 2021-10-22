#
#    Copyright (c) 2021 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#


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
