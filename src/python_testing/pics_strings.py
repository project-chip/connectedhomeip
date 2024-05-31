#
#    Copyright (c) 2023 Project CHIP Authors
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

def attribute_pics(pics_base: str, id: int) -> str:
    return f'{pics_base}.S.A{id:04x}'


def accepted_cmd_pics(pics_base: str, id: int) -> str:
    return f'{pics_base}.S.C{id:02x}.Rsp'


def generated_cmd_pics(pics_base: str, id: int) -> str:
    return f'{pics_base}.S.C{id:02x}.Tx'


def feature_pics(pics_base: str, bit: int) -> str:
    return f'{pics_base}.S.F{bit:02x}'

def server_pics(pics_base: str) -> str:
    return f'{pics_base}.S'

def client_pics(pics_base: str) -> str:
    return f'{pics_base}.C'