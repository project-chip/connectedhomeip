#
# SPDX-FileCopyrightText: 2021 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
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
