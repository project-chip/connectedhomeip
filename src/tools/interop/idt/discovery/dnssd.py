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
import os
import re
import traceback
from dataclasses import dataclass
from textwrap import dedent
from typing import Callable

from probe.ip_utils import get_addr_type
from utils.artifact import create_standard_log_name, log
from utils.log import add_border, border_print
from zeroconf import ServiceBrowser, ServiceInfo, ServiceListener, Zeroconf

logger = log.get_logger(__file__)


@dataclass()
class ServiceTypeInfo:
    type: str
    description: str


_COMMISSIONER_SERVICE_INFO = ServiceTypeInfo(
    "COMMISSIONER",
    "A service for a Matter commissioner aka. controller"
)
_COMMISSIONABLE_SERVICE_INFO = ServiceTypeInfo(
    "COMMISSIONABLE / EXTENDED DISCOVERY",
    "A service to be used in the commissioning process and provides more info about the device."
)
_OPERATIONAL_SERVICE_INFO = ServiceTypeInfo(
    "OPERATIONAL",
    "A service for a commissioned Matter device. It exposes limited info about the device."
)
_TBR_SERVICE_INFO = ServiceTypeInfo(
    "THREAD BORDER ROUTER",
    "A service for a thread border router; may be used for thread+Matter devices."
)

_TREL_SERVICE_INFO = ServiceTypeInfo(
    "THREAD RADIO ENCAPSULATION LINK",
    "A service for Thread Radio Encapsulation Link which is a method for thread BRs to exchange data on IP links."
)

_SERVICE_INFO = {
    "_matterd._udp.local.": _COMMISSIONER_SERVICE_INFO,
    "_matterc._udp.local.": _COMMISSIONABLE_SERVICE_INFO,
    "_matter._tcp.local.": _OPERATIONAL_SERVICE_INFO,
    "_meshcop._udp.local.": _TBR_SERVICE_INFO,
    "_trel._udp.local.": _TREL_SERVICE_INFO,
}


@dataclass()
class TxtRecordParser:
    readable_name: str
    explanation: str
    parse: Callable[[str], str]


def unwrap_str(to_unwrap: str) -> str:
    to_unwrap = to_unwrap.replace("\n", " ")
    to_unwrap = to_unwrap.replace("\t", " ")
    return re.sub(' +', ' ', to_unwrap)


# TODO: Thread parser

class MatterTxtRecordParser:

    def __init__(self):
        self.parsers = {
            # Commissioning
            "D": TxtRecordParser("Discriminator",
                                 unwrap_str("Differentiates advertisements from this instance of the device from \
                                             advertisement from others devices w/ the same VID/PID."),
                                 MatterTxtRecordParser.parse_d),  # To hex
            "VP": TxtRecordParser("VID/PID",
                                  "The Vendor ID and Product ID (each two bytes of hex) that identify this product.",
                                  MatterTxtRecordParser.parse_vp),  # Split + to hex
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
                                  unwrap_str("Given the current device state, follow these instructions to make the \
                                              device commissionable."),
                                  MatterTxtRecordParser.parse_ph),  # Decode bitmap
            "PI": TxtRecordParser("Pairing instructions",
                                  unwrap_str("Used with the Pairing hint. If the Pairing hint mentions N, this \
                                              is the value of N."),
                                  MatterTxtRecordParser.parse_pass_through),  # None
            # General
            "SII": TxtRecordParser("Session idle interval",
                                   unwrap_str("Message Reliability Protocol retry interval while the device is idle in \
                                               milliseconds."),
                                   MatterTxtRecordParser.parse_pass_through),  # None
            "SAI": TxtRecordParser("Session active interval",
                                   unwrap_str("Message Reliability Protocol retry interval while the device is \
                                               active in milliseconds."),
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

    @staticmethod
    def parse_vp(txt_value: str) -> str:
        vid, pid = txt_value.split("+")
        vid, pid = hex(int(vid)), hex(int(pid))
        return f"VID: {vid}, PID: {pid}"

    @staticmethod
    def parse_cm(txt_value: str) -> str:
        cm = int(txt_value)
        mode_descriptions = [
            "Not in commissioning mode",
            "In passcode commissioning mode (standard mode)",
            "In dynamic passcode commissioning mode",
        ]
        return mode_descriptions[cm]

    @staticmethod
    def parse_dt(txt_value: str) -> str:
        application_device_types = {
            # lighting
            "0x100": "On/Off Light",
            "0x101": "Dimmable Light",
            "0x10C": "Color Temperature Light",
            "0x10D": "Extended Color Light",
            # smart plugs/outlets and other actuators
            "0x10A": "On/Off Plug-in Unit",
            "0x10B": "Dimmable Plug-In Unit",
            "0x303": "Pump",
            # switches and controls
            "0x103": "On/Off Light Switch",
            "0x104": "Dimmer Switch",
            "0x105": "Color Dimmer Switch",
            "0x840": "Control Bridge",
            "0x304": "Pump Controller",
            "0xF": "Generic Switch",
            # sensors
            "0x15": "Contact Sensor",
            "0x106": "Light Sensor",
            "0x107": "Occupancy Sensor",
            "0x302": "Temperature Sensor",
            "0x305": "Pressure Sensor",
            "0x306": "Flow Sensor",
            "0x307": "Humidity Sensor",
            "0x850": "On/Off Sensor",
            # closures
            "0xA": "Door Lock",
            "0xB": "Door Lock Controller",
            "0x202": "Window Covering",
            "0x203": "Window Covering Controller",
            # HVAC
            "0x300": "Heating/Cooling Unit",
            "0x301": "Thermostat",
            "0x2B": "Fan",
            # media
            "0x28": "Basic Video Player",
            "0x23": "Casting Video Player",
            "0x22": "Speaker",
            "0x24": "Content App",
            "0x29": "Casting Video Client",
            "0x2A": "Video Remote Control",
            # generic
            "0x27": "Mode Select",
        }
        return application_device_types[hex((int(txt_value))).upper().replace("0X", "0x")]

    @staticmethod
    def parse_ph(txt_value: str) -> str:
        pairing_hints = [
            "Power Cycle",
            "Custom commissioning flow",
            "Use existing administrator (already commissioned)",
            "Use settings menu on device",
            "Use the PI TXT record hint",
            "Read the manual",
            "Press the reset button",
            "Press Reset Button with application of power",
            "Press Reset Button for N seconds",
            "Press Reset Button until light blinks",
            "Press Reset Button for N seconds with application of power",
            "Press Reset Button until light blinks with application of power",
            "Press Reset Button N times",
            "Press Setup Button",
            "Press Setup Button with application of power",
            "Press Setup Button for N seconds",
            "Press Setup Button until light blinks",
            "Press Setup Button for N seconds with application of power",
            "Press Setup Button until light blinks with application of power",
            "Press Setup Button N times",
        ]
        ret = "\n"
        b_arr = [int(b) for b in bin(int(txt_value))[2:]][::-1]
        for i in range(0, len(b_arr)):
            b = b_arr[i]
            if b:
                ret += pairing_hints[i] + "\n"
        return ret

    @staticmethod
    def parse_t(txt_value: str) -> str:
        return "TCP supported" if int(txt_value) else "TCP not supported"


class MatterDnssdListener(ServiceListener):

    def __init__(self, artifact_dir: str) -> None:
        super().__init__()
        self.artifact_dir = artifact_dir
        self.logger = logger
        self.discovered_matter_devices: [str, ServiceInfo] = {}

    def write_log(self, line: str, log_name: str) -> None:
        with open(self.create_device_log_name(log_name), "a+") as log_file:
            log_file.write(line)

    def create_device_log_name(self, device_name) -> str:
        return os.path.join(
            self.artifact_dir,
            create_standard_log_name(f"{device_name}_dnssd", "txt"))

    @staticmethod
    def log_addr(info: ServiceInfo) -> str:
        ret = add_border("This device has the following IP addresses\n")
        for addr in info.parsed_scoped_addresses():
            ret += f"{get_addr_type(addr)}: {addr}\n"
        return ret

    def handle_service_info(
            self,
            zc: Zeroconf,
            type_: str,
            name: str,
            delta_type: str) -> None:
        to_log = "SERVICE EVENT\n"
        to_log += f"{name}\n"
        to_log += f"SERVICE {delta_type}\n"
        to_log += _SERVICE_INFO[type_].type + "\n"
        to_log += _SERVICE_INFO[type_].description + "\n"
        info = zc.get_service_info(type_, name)
        if info is not None:
            self.discovered_matter_devices[name] = info
            to_log += f"A/SRV TTL: {str(info.host_ttl)}\n"
            to_log += f"PTR/TXT TTL: {str(info.other_ttl)}\n"
            txt_parser = MatterTxtRecordParser()
            to_log += txt_parser.parse_txt_records(info)
            to_log += self.log_addr(info)
            self.logger.info(to_log)
            self.write_log(to_log, name)
        else:
            self.logger.warning(f"No info found for {to_log}")

    def add_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        self.handle_service_info(zc, type_, name, "ADDED")

    def update_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        self.handle_service_info(zc, type_, name, "UPDATED")

    def remove_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        to_log = "SERVICE_EVENT\n"
        to_log += f"Service {name} removed\n"
        to_log += _SERVICE_INFO[type_].type + "\n"
        to_log += _SERVICE_INFO[type_].description
        if name in self.discovered_matter_devices:
            del self.discovered_matter_devices[name]
        self.logger.warning(to_log)
        self.write_log(to_log, name)

    def browse_interactive(self) -> None:
        zc = Zeroconf()
        ServiceBrowser(zc, list(_SERVICE_INFO.keys()), self)
        try:
            self.logger.warning(
                dedent("\
                \n\
                Browsing Matter DNS-SD\n\
                DCL Lookup: https://webui.dcl.csa-iot.org/\n\
                See spec section 4.3 for details of Matter TXT records.\n"))
            border_print("Press enter to stop!", important=True)
            input("")
        finally:
            zc.close()

    async def browse_once(self, browse_time_seconds: int) -> Zeroconf:
        zc = Zeroconf()
        ServiceBrowser(zc, list(_SERVICE_INFO.keys()), self)
        await asyncio.sleep(browse_time_seconds)
        zc.close()
        return zc
