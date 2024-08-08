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
import glob
import json
import os
import typing
import xml.etree.ElementTree as ET


def attribute_pics_str(pics_base: str, id: int) -> str:
    return f'{pics_base}.S.A{id:04x}'


def accepted_cmd_pics_str(pics_base: str, id: int) -> str:
    return f'{pics_base}.S.C{id:02x}.Rsp'


def generated_cmd_pics_str(pics_base: str, id: int) -> str:
    return f'{pics_base}.S.C{id:02x}.Tx'


def feature_pics_str(pics_base: str, bit: int) -> str:
    return f'{pics_base}.S.F{bit:02x}'


def server_pics_str(pics_base: str) -> str:
    return f'{pics_base}.S'


def client_pics_str(pics_base: str) -> str:
    return f'{pics_base}.C'


def parse_pics(lines: typing.List[str]) -> dict[str, bool]:
    pics = {}
    for raw in lines:
        line, _, _ = raw.partition("#")
        line = line.strip()

        if not line:
            continue

        key, _, val = line.partition("=")
        val = val.strip()
        if val not in ["1", "0"]:
            raise ValueError('PICS {} must have a value of 0 or 1'.format(key))

        pics[key.strip()] = (val == "1")
    return pics


def parse_pics_xml(contents: str) -> dict[str, bool]:
    pics = {}
    mytree = ET.fromstring(contents)
    for pi in mytree.iter('picsItem'):
        name = pi.find('itemNumber').text
        support = pi.find('support').text
        pics[name] = int(json.loads(support.lower())) == 1
    return pics


def read_pics_from_file(path: str) -> dict[str, bool]:
    """ Reads a dictionary of PICS from a file (ci format) or directory (xml format). """
    if os.path.isdir(os.path.abspath(path)):
        pics_dict = {}
        for filename in glob.glob(f'{path}/*.xml'):
            with open(filename, 'r') as f:
                contents = f.read()
                pics_dict.update(parse_pics_xml(contents))
        return pics_dict

    else:
        with open(path, 'r') as f:
            lines = f.readlines()
            return parse_pics(lines)
