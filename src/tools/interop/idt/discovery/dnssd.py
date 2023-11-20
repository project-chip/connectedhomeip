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
class MdnsTypeInfo:
    type: str
    description: str


commissioner = MdnsTypeInfo(
    "COMMISSIONER",
    "This is a service for a Matter commissioner aka. controller"
)
commissionable = MdnsTypeInfo(
    "COMMISSIONABLE / EXTENDED DISCOVERY",
    "This is a service to be used in the commissioning process and provides more info about the device."
)
operational = MdnsTypeInfo(
    "OPERATIONAL",
    "This is a service for a commissioned Matter device. It exposes limited info about the device."
)
border_router = MdnsTypeInfo(
    "THREAD BORDER ROUTER",
    "This is a service for a thread border router; may be used for thread+Matter devices."
)

_MDNS_TYPES = {
    "_matterd._udp.local.": commissioner,
    "_matterc._udp.local.": commissionable,
    "_matter._tcp.local.": operational,
    "_meshcop._udp.local.": border_router,
}


@dataclass()
class RecordParser:
    readable_name: str
    explanation: str
    parse: Callable[[str], str]


# TODO: Meshcop parser

class MatterTxtRecordParser:

    def __init__(self):
        self.parsers = {
            "D": RecordParser("Discriminator",
                              dedent("\
                              Differentiates this instance of the device from others w/ same VID/PID that might be \n\
                              in the environment."),
                              MatterTxtRecordParser.parse_d),  # To hex
            "VP": RecordParser("VID/PID",
                               "The Vendor ID and Product ID (each are two bytes of hex) that identify this product.",
                               MatterTxtRecordParser.parse_vp),  # Split + to hex
            "CM": RecordParser("Commissioning mode",
                               "Whether the device is in commissioning mode or not.",
                               MatterTxtRecordParser.parse_cm),  # Decode
            "DT": RecordParser("Device type",
                               "Application type for this end device.",
                               MatterTxtRecordParser.parse_dt),  # Decode
            "DN": RecordParser("Device name",
                               "Manufacturer provided device name. MAY match NodeLabel in Basic info cluster.",
                               MatterTxtRecordParser.parse_pass_through),  # None
            "RI": RecordParser("Rotating identifier",
                               "Vendor specific, non-trackable per-device ID.",
                               MatterTxtRecordParser.parse_pass_through),  # None
            "PH": RecordParser("Pairing hint",
                               dedent("\
                               Given the current device state, follow these instructions to make the device \n\
                               commissionable."),
                               MatterTxtRecordParser.parse_ph),  # Decode
            "PI": RecordParser("Pairing instructions",
                               dedent("\
                               Used with the Pairing hint. If the Pairing hint mentions N, this is the \n\
                               value of N."),
                               MatterTxtRecordParser.parse_pass_through),  # None
            # General records
            "SII": RecordParser("Session idle interval",
                                "Message Reliability Protocol retry interval while the device is idle in milliseconds.",
                                MatterTxtRecordParser.parse_pass_through),  # None
            "SAI": RecordParser("Session active interval",
                                dedent("\
                                Message Reliability Protocol retry interval while the device is active \n\
                                in milliseconds."),
                                MatterTxtRecordParser.parse_pass_through),  # None
            "SAT": RecordParser("Session active threshold",
                                "Duration of time this device stays active after last activity in milliseconds.",
                                MatterTxtRecordParser.parse_pass_through),  # None
            "T": RecordParser("Supports TCP",
                              "Whether this device supports TCP client and or Server.",
                              MatterTxtRecordParser.parse_t),  # Decode
        }
        self.unparsed_records = ""
        self.parsed_records = ""

    def parse_single_record(self, key: str, value: str):
        parser: RecordParser = self.parsers[key]
        self.parsed_records += add_border(parser.readable_name + "\n")
        self.parsed_records += parser.explanation + "\n\n"
        try:
            self.parsed_records += "PARSED VALUE: " + parser.parse(value) + "\n"
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

    def parse_records(self, info: ServiceInfo) -> str:
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
                    self.parse_single_record(name, value)
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
        info = zc.get_service_info(type_, name)
        self.discovered_matter_devices[name] = info
        to_log = f"{name}\n"
        update_str = f"\nSERVICE {delta_type}\n"
        to_log += ("*" * (len(update_str) - 2)) + update_str
        to_log += _MDNS_TYPES[type_].type + "\n"
        to_log += _MDNS_TYPES[type_].description + "\n"
        to_log += f"A/SRV TTL: {str(info.host_ttl)}\n"
        to_log += f"PTR/TXT TTL: {str(info.other_ttl)}\n"
        txt_parser = MatterTxtRecordParser()
        to_log += txt_parser.parse_records(info)
        to_log += self.log_addr(info)
        self.logger.info(to_log)
        self.write_log(to_log, name)

    def add_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        self.handle_service_info(zc, type_, name, "ADDED")

    def update_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        self.handle_service_info(zc, type_, name, "UPDATED")

    def remove_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        to_log = f"Service {name} removed\n"
        to_log += _MDNS_TYPES[type_].type + "\n"
        to_log += _MDNS_TYPES[type_].description
        if name in self.discovered_matter_devices:
            del self.discovered_matter_devices[name]
        self.logger.warning(to_log)
        self.write_log(to_log, name)

    def browse_interactive(self) -> None:
        zc = Zeroconf()
        ServiceBrowser(zc, list(_MDNS_TYPES.keys()), self)
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
        ServiceBrowser(zc, list(_MDNS_TYPES.keys()), self)
        await asyncio.sleep(browse_time_seconds)
        zc.close()
        return zc
