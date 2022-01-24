# Copyright (c) 2022 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import logging
import stringcase
import enum

from .types import ParseDataType, BasicInteger, BasicString, FundamentalType, IdlType


def filter_IsBasicInteger(type) -> bool:
    return type(type) == BasicInteger


def filter_IsBasicString(type) -> bool:
    return type(type) == BasicString


def filter_IsFundamentalType(type) -> bool:
    return type(type) == FundamentalType


def filter_IsIdlType(type) -> bool:
    return type(type) == IdlType


def RegisterCommonFilters(filtermap):
    """
    Register filters that are NOT considered platform-generator specific.
    """

    # General casing for output naming
    filtermap['camelcase'] = stringcase.camelcase
    filtermap['capitalcase'] = stringcase.capitalcase
    filtermap['constcase'] = stringcase.constcase
    filtermap['pascalcase'] = stringcase.pascalcase
    filtermap['snakecase'] = stringcase.snakecase
    filtermap['spinalcase'] = stringcase.spinalcase

    # type conversion is generally very used
    filtermap['parseDataType'] = ParseDataType
    filtermap['isBasicInteger'] = filter_IsBasicInteger
    filtermap['isBasicString'] = filter_IsBasicString
    filtermap['isFundamentalType'] = filter_IsFundamentalType
    filtermap['isIdlType'] = filter_IsIdlType
