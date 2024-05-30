#
#    Copyright (c) 2024 Project CHIP Authors
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


def th_read_attr(name: str):
    return f"TH reads from the DUT the _{name}_ attribute"


dut_reply = "Verify that the DUT response contains"


def dut_reply_attr(name: str):
    return f"{dut_reply} the _{name}_ attribute"


def dut_reply_value(typename: str, min: typing.Union[int, str, None], max: stepnum: typing.Union[int, str, None]):
    value_range = ''
    if min is not None:
        if max is not None:
            value_range = f". Value has to be within a range of {min} and {max}"
        else:
            value_range = f". Value has to be at least {min}"
    return f"{dut_reply} a {typename} value{value_range}"


if_attr_supported = "If the attribute is supported on the cluster"

com_dut_th = "Commission DUT to TH (can be skipped if done in a preceding test)"

store_value_as = "Store the value as"
