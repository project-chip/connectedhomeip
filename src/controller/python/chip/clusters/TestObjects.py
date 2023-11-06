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


import typing
from dataclasses import dataclass
from enum import IntEnum

from chip import ChipUtility

from .ClusterObjects import ClusterCommand, ClusterObjectDescriptor


@dataclass
class OnOff:
    id: typing.ClassVar[int] = 0x0006

    class DelayedAllOffEffectVariantEnum(IntEnum):
        kDelayedOffFastFade = 0x00
        kNoFade = 0x01
        kDelayedOffSlowFade = 0x02

    class DyingLightEffectVariantEnum(IntEnum):
        kDyingLightFadeOff = 0x00

    class EffectIdentifierEnum(IntEnum):
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
