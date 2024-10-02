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

import argparse
import os
import sys

from pics_generator_support import map_cluster_name_to_pics_xml, pics_xml_file_list_loader

# Add the path to python_testing folder, in order to be able to import from matter_testing_support
sys.path.append(os.path.abspath(sys.path[0] + "/../../python_testing"))
from spec_parsing_support import build_xml_clusters  # noqa: E402

parser = argparse.ArgumentParser()
parser.add_argument('--pics-template', required=True)
args, unknown = parser.parse_known_args()

xml_template_path_str = args.pics_template

print("Build list of PICS XML")
pics_xml_file_list = pics_xml_file_list_loader(xml_template_path_str, True)

print("Build list of spec XML")
xml_clusters, problems = build_xml_clusters()

for cluster in xml_clusters:
    pics_xml_file_name = map_cluster_name_to_pics_xml(xml_clusters[cluster].name, pics_xml_file_list)

    if pics_xml_file_name:
        print(f"{xml_clusters[cluster].name} - {pics_xml_file_name} ✅")
    else:
        print(
            f"Could not find matching PICS XML file for {xml_clusters[cluster].name} - {xml_clusters[cluster].pics} (Provisional: {xml_clusters[cluster].is_provisional}) ❌")
