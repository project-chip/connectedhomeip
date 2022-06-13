#!/usr/bin/env -S python3

#
#    Copyright (c) 2022 Project CHIP Authors.
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

##
#    @file
# Generates OTA image list file based on Mbed project configuration file (mbed_app.json)
# OTA image list JSON file (see /examples/ota-provider-app/linux/README.md)
#
# Usage: generate_ota_list_image.py <mbed_config_json_file_path> <image_path>

import json
import os
import string
import sys

FILE_NAME = "ota-image-list.json"

FILE_DATA_TEMPLATE = """{
    "deviceSoftwareVersionModel": [
        {
            "vendorId": 0,
            "productId": 0,
            "softwareVersion": 0,
            "softwareVersionString": "",
            "cDVersionNumber": 0,
            "softwareVersionValid": true,
            "minApplicableSoftwareVersion": 0,
            "maxApplicableSoftwareVersion": 1000,
            "otaURL": ""
        }
    ]
}
"""


def CreateOtaListFile(config, imagePath):
    data = json.loads(FILE_DATA_TEMPLATE)

    data["deviceSoftwareVersionModel"][0]["vendorId"] = int(
        config["config"]["vendor-id"]["value"], base=16)
    data["deviceSoftwareVersionModel"][0]["productId"] = int(
        config["config"]["product-id"]["value"], base=16)
    data["deviceSoftwareVersionModel"][0]["softwareVersion"] = int(
        config["config"]["version-number"]["value"])
    data["deviceSoftwareVersionModel"][0]["softwareVersionString"] = config[
        "config"]["version-number-str"]["value"].strip('"\\')
    data["deviceSoftwareVersionModel"][0]["otaURL"] = str(imagePath)

    output_path = os.path.join(os.path.dirname(imagePath), FILE_NAME)

    with open(output_path, 'w') as jsonFile:
        json.dump(data, jsonFile)


def main():
    if len(sys.argv) != 3:
        print('Usage: ' + sys.argv[0] +
              ' <mbed_config_json_file_path> <image_path>')
        exit(1)

    config_json_path = sys.argv[1]
    image_path = sys.argv[2]
    with open(config_json_path, 'r') as json_file:
        config = json.loads(json_file.read())

    CreateOtaListFile(config, image_path)


if __name__ == "__main__":
    main()
