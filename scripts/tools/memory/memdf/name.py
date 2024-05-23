#
# Copyright (c) 2021 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

"""Special section/symbol names."""

ABS = '*ABS*'
UNDEF = '*UNDEF*'
UNKNOWN = '*unknown*'
OTHER = '*other*'
TOTAL = '*total*'

GAP_PREFIX = '*GAP_'
UNUSED_PREFIX = '*UNUSED_'
OVERLAP_PREFIX = '*OVERLAP_'


def gap(address: int, size: int) -> str:
    return f'{GAP_PREFIX}{address:X}_{size:X}*'


def unused(address: int, size: int) -> str:
    return f'{UNUSED_PREFIX}{address:X}_{size:X}*'


def overlap(address: int, size: int) -> str:
    return f'{OVERLAP_PREFIX}{address:X}_{size:X}*'
