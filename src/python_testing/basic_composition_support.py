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

import asyncio
import base64
import copy
import json
import logging
import pathlib
import sys
import typing
from pprint import pformat, pprint
from typing import Any, Optional

import chip.clusters.ClusterObjects
import chip.tlv
from chip.clusters.Attribute import ValueDecodeFailure
from mobly import asserts


def MatterTlvToJson(tlv_data: dict[int, Any]) -> dict[str, Any]:
    """Given TLV data for a specific cluster instance, convert to the Matter JSON format."""

    matter_json_dict = {}

    key_type_mappings = {
        chip.tlv.uint: "UINT",
        int: "INT",
        bool: "BOOL",
        list: "ARRAY",
        dict: "STRUCT",
        chip.tlv.float32: "FLOAT",
        float: "DOUBLE",
        bytes: "BYTES",
        str: "STRING",
        ValueDecodeFailure: "ERROR",
        type(None): "NULL",
    }

    def ConvertValue(value) -> Any:
        if isinstance(value, ValueDecodeFailure):
            raise ValueError(f"Bad Value: {str(value)}")

        if isinstance(value, bytes):
            return base64.b64encode(value).decode("UTF-8")
        elif isinstance(value, list):
            value = [ConvertValue(item) for item in value]
        elif isinstance(value, dict):
            value = MatterTlvToJson(value)

        return value

    for key in tlv_data:
        value_type = type(tlv_data[key])
        value = copy.deepcopy(tlv_data[key])

        element_type: str = key_type_mappings[value_type]
        sub_element_type = ""

        try:
            new_value = ConvertValue(value)
        except ValueError as e:
            new_value = str(e)

        if element_type:
            if element_type == "ARRAY":
                if len(new_value):
                    sub_element_type = key_type_mappings[type(tlv_data[key][0])]
                else:
                    sub_element_type = "?"

        new_key = ""
        if element_type:
            if sub_element_type:
                new_key = f"{str(key)}:{element_type}-{sub_element_type}"
            else:
                new_key = f"{str(key)}:{element_type}"
        else:
            new_key = str(key)

        matter_json_dict[new_key] = new_value

    return matter_json_dict


class BasicCompositionTests:
    def get_code(self, dev_ctrl):
        created_codes = []
        for idx, discriminator in enumerate(self.matter_test_config.discriminators):
            created_codes.append(dev_ctrl.CreateManualCode(discriminator, self.matter_test_config.setup_passcodes[idx]))

        setup_codes = self.matter_test_config.qr_code_content + self.matter_test_config.manual_code + created_codes
        asserts.assert_equal(len(setup_codes), 1,
                             "Require exactly one of either --qr-code, --manual-code or (--discriminator and --passcode).")
        return setup_codes[0]

    def dump_wildcard(self, dump_device_composition_path: typing.Optional[str]) -> tuple[str, str]:
        """ Dumps a json and a txt file of the attribute wildcard for this device if the dump_device_composition_path is supplied.
            Returns the json and txt as strings.
        """
        node_dump_dict = {endpoint_id: MatterTlvToJson(self.endpoints_tlv[endpoint_id]) for endpoint_id in self.endpoints_tlv}
        json_dump_string = json.dumps(node_dump_dict, indent=2)
        logging.debug(f"Raw TLV contents of Node: {json_dump_string}")

        if dump_device_composition_path is not None:
            with open(pathlib.Path(dump_device_composition_path).with_suffix(".json"), "wt+") as outfile:
                json.dump(node_dump_dict, outfile, indent=2)
            with open(pathlib.Path(dump_device_composition_path).with_suffix(".txt"), "wt+") as outfile:
                pprint(self.endpoints, outfile, indent=1, width=200, compact=True)
        return (json_dump_string, pformat(self.endpoints, indent=1, width=200, compact=True))

    async def setup_class_helper(self, allow_pase: bool = True):
        dev_ctrl = self.default_controller
        self.problems = []

        dump_device_composition_path: Optional[str] = self.user_params.get("dump_device_composition_path", None)

        node_id = self.dut_node_id

        task_list = []
        if allow_pase:
            setup_code = self.get_code(dev_ctrl)
            pase_future = dev_ctrl.EstablishPASESession(setup_code, self.dut_node_id)
            task_list.append(asyncio.create_task(pase_future))

        case_future = dev_ctrl.GetConnectedDevice(nodeid=node_id, allowPASE=False)
        task_list.append(asyncio.create_task(case_future))

        for task in task_list:
            asyncio.ensure_future(task)

        done, pending = await asyncio.wait(task_list, return_when=asyncio.FIRST_COMPLETED)

        for task in pending:
            try:
                task.cancel()
                await task
            except asyncio.CancelledError:
                pass

        wildcard_read = (await dev_ctrl.Read(node_id, [()]))

        # ======= State kept for use by all tests =======
        # All endpoints in "full object" indexing format
        self.endpoints = wildcard_read.attributes

        # All endpoints in raw TLV format
        self.endpoints_tlv = wildcard_read.tlvAttributes

        self.dump_wildcard(dump_device_composition_path)

        logging.info("###########################################################")
        logging.info("Start of actual tests")
        logging.info("###########################################################")

    def get_test_name(self) -> str:
        """Return the function name of the caller. Used to create logging entries."""
        return sys._getframe().f_back.f_code.co_name

    def fail_current_test(self, msg: Optional[str] = None):
        if not msg:
            # Without a message, just log the last problem seen
            asserts.fail(msg=self.problems[-1].problem)
        else:
            asserts.fail(msg)
