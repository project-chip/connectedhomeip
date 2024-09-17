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

import argparse
import os
import sys

from rich.console import Console

# Add the path to python_testing folder, in order to be able to import from matter_testing_support
sys.path.append(os.path.abspath(sys.path[0] + "/../../python_testing"))
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main  # noqa: E402
from spec_parsing_support import build_xml_clusters  # noqa: E402

console = None
xml_clusters = None

parser = argparse.ArgumentParser()
parser.add_argument('--pics-template', required=True)
args, unknown = parser.parse_known_args()

xmlTemplatePathStr = args.pics_template
if not xmlTemplatePathStr.endswith('/'):
    xmlTemplatePathStr += '/'

# Load PICS XML templates
print("Capture list of PICS XML templates")
xmlFileList = os.listdir(xmlTemplatePathStr)
for PICSXmlFile in xmlFileList:
    print(f"{xmlTemplatePathStr}/{PICSXmlFile}")


class XMLPICSValidator(MatterBaseTest):
    @async_test_body
    async def test_xml_pics_validator(self):

        # Create console to print
        global console
        console = Console()

        global xml_clusters
        xml_clusters, problems = build_xml_clusters()

        for cluster in xml_clusters:

            fileName = ""
            clusterName = xml_clusters[cluster].name

            if "ICDManagement" == clusterName:
                picsFileName = "ICD Management"

            elif "OTA Software Update Provider" in clusterName or \
                 "OTA Software Update Requestor" in clusterName:
                picsFileName = "OTA Software Update"

            elif "On/Off" == clusterName:
                picsFileName = clusterName.replace("/", "-")

            elif "GroupKeyManagement" == clusterName:
                picsFileName = "Group Communication"

            elif "Wake on LAN" == clusterName or \
                 "Low Power" == clusterName or \
                 "Keypad Input" == clusterName or \
                 "Audio Output" == clusterName or \
                 "Media Input" == clusterName or \
                 "Target Navigator" == clusterName or \
                 "Content Control" == clusterName or \
                 "Channel" == clusterName or \
                 "Media Playback" == clusterName or \
                 "Account Login" == clusterName or \
                 "Application Basic" == clusterName or \
                 "Content Launcher" == clusterName or \
                 "Content App Observer" == clusterName or \
                 "Application Launcher" == clusterName:

                picsFileName = "Media Cluster"

            elif "Operational Credentials" == clusterName:
                picsFileName = "Node Operational Credentials"

            # Workaround for naming colisions with current logic
            elif "Thermostat" == clusterName:
                picsFileName = "Thermostat Cluster"

            elif "Boolean State" == clusterName:
                picsFileName = "Boolean State Cluster"

            elif "AccessControl" in clusterName:
                picsFileName = "Access Control Cluster"

            else:
                picsFileName = clusterName

            for file in xmlFileList:
                if file.lower().startswith(picsFileName.lower()):
                    fileName = file
                    break

            if fileName:
                console.print(f"[blue]\"{clusterName}\" - \"{fileName}\" ✅")
            else:
                console.print(f"[red]Could not find matching file for \"{clusterName}\" ❌")
                continue


if __name__ == "__main__":
    default_matter_test_main()
