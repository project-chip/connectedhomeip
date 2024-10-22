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

cluster_to_pics_dict = {
    # Name mapping due to inconsistent naming of PICS files
    "ICDManagement": "ICD Management",
    "OTA Software Update Provider": "OTA Software Update",
    "OTA Software Update Requestor": "OTA Software Update",
    "On/Off": "On-Off",
    "GroupKeyManagement": "Group Communication",
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
    "Application Launch": "Media Cluster",
    "Operational Credentials": "Node Operational Credentials",

    # Workaround for naming colisions with current logic
    "Thermostat": "Thermostat Cluster",
    "Boolean State": "Boolean State Cluster",
    "AccessControl": "Access Control Cluster",
}


def pics_xml_file_list_loader(pics_xml_path: str, log_loaded_pics_files: bool) -> list:

    pics_xml_file_list = os.listdir(pics_xml_path)

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
