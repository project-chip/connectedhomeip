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

import traceback
from dataclasses import dataclass
from typing import Callable

from utils.artifact import log
from utils.data import MATTER_APPLICATION_DEVICE_TYPES, MATTER_COMMISSIONING_MODE_DESCRIPTIONS, MATTER_PAIRING_HINTS
from utils.log import add_border
from zeroconf import ServiceInfo

logger = log.get_logger(__file__)


@dataclass()
class ServiceTypeInfo:
    type: str
    description: str


class ServiceLibrary:

    def __init__(self):
        self._SERVICE_TYPE_INFO = {
            "_matterd._udp.local.": ServiceTypeInfo(
                "COMMISSIONER",
                "A service for a Matter commissioner aka. controller"
            ),
            "_matterc._udp.local.": ServiceTypeInfo(
                "COMMISSIONABLE / EXTENDED DISCOVERY",
                "A service to be used in the commissioning process and provides more info about the device."
            ),
            "_matter._tcp.local.": ServiceTypeInfo(
                "OPERATIONAL",
                "A service for a commissioned Matter device. It exposes limited info about the device."
            ),
            "_meshcop._udp.local.": ServiceTypeInfo(
                "THREAD BORDER ROUTER",
                "A service for a thread border router; may be used for thread+Matter devices."
            ),
            "_trel._udp.local.": ServiceTypeInfo(
                "THREAD RADIO ENCAPSULATION LINK",
                "A service for Thread Radio Encapsulation Link which is a method for thread BRs to exchange data on "
                "IP links."
            ),
        }

    def known_service_types(self) -> [str]:
        return list(self._SERVICE_TYPE_INFO.keys())

    def get_service_type_info(self, service_type: str) -> ServiceTypeInfo:
        return self._SERVICE_TYPE_INFO[service_type]


@dataclass()
class TxtRecordParser:
    readable_name: str
    explanation: str
    parse: Callable[[str], str]


# TODO: Meshcop parser

class MatterTxtRecordParser:

    def __init__(self):
        self.parsers = {
            # Commissioning
            "D": TxtRecordParser("Discriminator",
                                 "Differentiates advertisements from this instance of the device from "
                                 "advertisement from others devices w/ the same VID/PID.",
                                 MatterTxtRecordParser.parse_d),  # To hex
            "VP": TxtRecordParser("VID/PID",
                                  "The Vendor ID and Product ID (each two bytes of hex) that identify this product.",
                                  self.parse_vp),  # Split + to hex, also stores values for filtering
            "CM": TxtRecordParser("Commissioning mode",
                                  "Whether the device is in commissioning mode or not.",
                                  MatterTxtRecordParser.parse_cm),  # Decode
            "DT": TxtRecordParser("Device type",
                                  "Application type for this end device.",
                                  MatterTxtRecordParser.parse_dt),  # Decode map
            "DN": TxtRecordParser("Device name",
                                  "Manufacturer provided device name. MAY match NodeLabel in Basic info cluster.",
                                  MatterTxtRecordParser.parse_pass_through),  # None
            "RI": TxtRecordParser("Rotating identifier",
                                  "Vendor specific, non-trackable per-device ID.",
                                  MatterTxtRecordParser.parse_pass_through),  # None
            "PH": TxtRecordParser("Pairing hint",
                                  "Given the current device state, follow these instructions to make the "
                                  "device commissionable.",
                                  MatterTxtRecordParser.parse_ph),  # Decode bitmap
            "PI": TxtRecordParser("Pairing instructions",
                                  "Used with the Pairing hint. If the Pairing hint mentions N, this "
                                  "is the value of N.",
                                  MatterTxtRecordParser.parse_pass_through),  # None
            # General
            "SII": TxtRecordParser("Session idle interval",
                                   "Message Reliability Protocol retry interval while the device is idle in "
                                   "milliseconds.",
                                   MatterTxtRecordParser.parse_pass_through),  # None
            "SAI": TxtRecordParser("Session active interval",
                                   "Message Reliability Protocol retry interval while the device is "
                                   "active in milliseconds.",
                                   MatterTxtRecordParser.parse_pass_through),  # None
            "SAT": TxtRecordParser("Session active threshold",
                                   "Duration of time this device stays active after last activity in milliseconds.",
                                   MatterTxtRecordParser.parse_pass_through),  # None
            "T": TxtRecordParser("Supports TCP",
                                 "Whether this device supports TCP.",
                                 MatterTxtRecordParser.parse_t),  # Decode
        }
        self.unparsed_records = ""
        self.parsed_records = ""
        self.vid = None
        self.pid = None

    def parse_single_txt_record(self, key: str, value: str):
        parser: TxtRecordParser = self.parsers[key]
        self.parsed_records += add_border(parser.readable_name + "\n")
        self.parsed_records += parser.explanation + "\n\n"
        try:
            self.parsed_records += "PARSED VALUE:\n" + parser.parse(value) + "\n"
        except Exception:
            logger.error("Exception parsing TXT record, appending raw value")
            logger.error(traceback.format_exc())
            self.parsed_records += f"RAW VALUE: {value}\n"

    def get_output(self) -> str:
        unparsed_exp = "\nThe following TXT records were not parsed or explained:\n"
        parsed_exp = "\nThe following was discovered about this device via TXT records:\n"
        ret = ""
        if self.unparsed_records:
            ret += unparsed_exp + self.unparsed_records
        if self.parsed_records:
            ret += parsed_exp + self.parsed_records
        return ret

    def parse_txt_records(self, info: ServiceInfo) -> str:
        if info.properties is not None:
            for name, value in info.properties.items():
                try:
                    name = name.decode("utf-8")
                except UnicodeDecodeError:
                    name = str(name)
                try:
                    value = value.decode("utf-8")
                except UnicodeDecodeError:
                    value = str(value)
                if name not in self.parsers:
                    self.unparsed_records += f"KEY: {name} VALUE: {value}\n"
                else:
                    self.parse_single_txt_record(name, value)
        return self.get_output()

    @staticmethod
    def parse_pass_through(txt_value: str) -> str:
        return txt_value

    @staticmethod
    def parse_d(txt_value: str) -> str:
        return hex(int(txt_value))

    def parse_vp(self, txt_value: str) -> str:
        vid, pid = txt_value.split("+")
        vid, pid = hex(int(vid)), hex(int(pid))
        self.vid = vid
        self.pid = pid
        return f"VID: {vid}, PID: {pid}"

    @staticmethod
    def parse_cm(txt_value: str) -> str:
        return MATTER_COMMISSIONING_MODE_DESCRIPTIONS[int(txt_value)]

    @staticmethod
    def parse_dt(txt_value: str) -> str:
        return MATTER_APPLICATION_DEVICE_TYPES[hex((int(txt_value))).upper().replace("0X", "0x")]

    @staticmethod
    def parse_ph(txt_value: str) -> str:
        ret = "\n"
        b_arr = [int(b) for b in bin(int(txt_value))[2:]][::-1]
        for i in range(0, len(b_arr)):
            b = b_arr[i]
            if b:
                ret += MATTER_PAIRING_HINTS[i] + "\n"
        return ret

    @staticmethod
    def parse_t(txt_value: str) -> str:
        return "TCP supported" if int(txt_value) else "TCP not supported"
