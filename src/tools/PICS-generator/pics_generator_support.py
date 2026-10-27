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

import os
from dataclasses import dataclass

cluster_to_pics_dict = {
    # Name mapping due to inconsistent naming of PICS files
    "ICDManagement": "ICD Management",
    "OTA Software Update Provider": "OTA Software Update",
    "OTA Software Update Requestor": "OTA Software Update",
    "On/Off": "On-Off",
    "GroupKeyManagement": "Group Communication",
    "Group Key Management": "Group Communication",
    "Wake On LAN": "Media Cluster",
    "Wake on LAN": "Media Cluster",
    "Low Power": "Media Cluster",
    "Keypad Input": "Media Cluster",
    "Audio Output": "Media Cluster",
    "Media Input": "Media Cluster",
    "Target Navigator": "Media Cluster",
    "Content Control": "Media Cluster",
    "Channel": "Media Cluster",
    "Media Playback": "Media Cluster",
    "Account Login": "Media Cluster",
    "Application Basic": "Media Cluster",
    "Content Launcher": "Media Cluster",
    "Content App Observer": "Media Cluster",
    "Application Launcher": "Media Cluster",
    "Operational Credentials": "Node Operational Credentials",

    # Workaround for naming colisions with current logic
    "Thermostat": "Thermostat Cluster",
    "Boolean State": "Boolean State Cluster",
    "Access Control": "Access Control Cluster",
    "Energy EVSE": "Energy EVSE Cluster",
    "AV Analysis": "AV Analytics Control",
}


def pics_xml_file_list_loader(pics_xml_path: str, log_loaded_pics_files: bool) -> list:

    # Sort so template selection is deterministic. os.listdir order is
    # filesystem-dependent, so without this the prefix match below could pick
    # different templates on different machines when names share a prefix
    # (e.g. "Access Control Cluster ..." vs "Access Control Enforcement ...").
    pics_xml_file_list = sorted(os.listdir(pics_xml_path))

    if log_loaded_pics_files:
        if not pics_xml_path.endswith('/'):
            pics_xml_path += '/'

        for pics_xml_file in pics_xml_file_list:
            print(f"{pics_xml_path}/{pics_xml_file}")

    return pics_xml_file_list


def map_cluster_name_to_pics_xml(cluster_name, pics_xml_file_list) -> str:
    file_name = ""

    pics_file_name = cluster_to_pics_dict.get(cluster_name, cluster_name)

    for file in pics_xml_file_list:
        if file.lower().startswith(pics_file_name.lower()):
            file_name = file
            break

    return file_name


def normalize_pics_item_number(item_number: str | None) -> str:
    """Return a PICS itemNumber in the form used for comparisons.

    The PICS Guidelines require lowercase hex digits in PICS codes, but some
    CSA PICS XML templates use uppercase (e.g. "BRBINFO.S.A000a" and
    "HSTAT.S.A000A" in the same bundle). itemNumbers are compared
    case-insensitively so those items still get marked; see
    is_case_only_mismatch() for flagging them. A missing itemNumber
    normalizes to "", which never matches a generated PICS code.
    """
    if item_number is None:
        return ""
    return item_number.lower()


@dataclass(frozen=True)
class CaseMismatchedPicsItem:
    """A supported PICS item whose template itemNumber differs from its PICS code only in case."""
    # Generated file the item is in, e.g. "endpoint1/Humidistat Cluster Test Plan.xml".
    output_file: str
    # itemNumber as written in the template, e.g. "HSTAT.S.A000A".
    item_number: str
    # PICS code TC-IDM-10.4 looks up, e.g. "HSTAT.S.A000a".
    pics_code: str


def is_case_only_mismatch(item_number: str, pics_code: str) -> bool:
    """Return True if a template itemNumber matches pics_code only when case is ignored.

    pics_code is in the lowercase-hex format the PICS Guidelines require.
    TC-IDM-10.4 looks PICS codes up exactly, so an item like "HSTAT.S.A000A"
    that is marked as supported still fails that test, which expects
    "HSTAT.S.A000a".
    """
    return item_number != pics_code and normalize_pics_item_number(item_number) == normalize_pics_item_number(pics_code)


def format_case_mismatch_warning(items: list[CaseMismatchedPicsItem]) -> str:
    """Return the end-of-run warning text for supported items that will fail TC-IDM-10.4."""
    lines = [
        f"{len(items)} supported PICS item(s) will fail TC-IDM-10.4.",
        "",
        "These items are marked as supported, but their itemNumbers in the PICS XML",
        "templates use uppercase hex digits. The PICS Guidelines require lowercase",
        "hex, and TC-IDM-10.4 looks PICS codes up exactly, so it reports each item",
        "below as missing until the template is fixed.",
    ]
    items_by_file: dict[str, list[CaseMismatchedPicsItem]] = {}
    for item in items:
        items_by_file.setdefault(item.output_file, []).append(item)
    for output_file, file_items in items_by_file.items():
        lines += ["", output_file]
        lines += [f"  {item.item_number}  (TC-IDM-10.4 expects {item.pics_code})" for item in file_items]
    return "\n".join(lines)
