#!/usr/bin/env python3

# Copyright (c) 2022 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import argparse
import json
from xml.etree import ElementTree as ET


class TizenManifest:

    def __init__(self, manifest):
        self.manifest = ET.parse(manifest).getroot()

    def _get_application(self, apptype):
        return self.manifest.find("{*}" + apptype)

    def get_package_name(self):
        return self.manifest.get("package")

    def get_package_version(self):
        return self.manifest.get("version")

    def get_service_exec(self):
        app = self._get_application("service-application")
        return app.get("exec", "") if app else ""

    def get_ui_exec(self):
        app = self._get_application("ui-application")
        return app.get("exec", "") if app else ""

    def get_watch_exec(self):
        app = self._get_application("watch-application")
        return app.get("exec", "") if app else ""

    def get_widget_exec(self):
        app = self._get_application("widget-application")
        return app.get("exec", "") if app else ""


if __name__ == '__main__':

    parser = argparse.ArgumentParser(
        description="Tool for extracting data from Tizen XML manifest file")
    parser.add_argument('MANIFEST', help="Tizen manifest XML file")

    args = parser.parse_args()
    manifest = TizenManifest(args.MANIFEST)

    print(json.dumps({
        'package': {
            'name': manifest.get_package_name(),
            'version': manifest.get_package_version(),
        },
        'apps': {
            'service': manifest.get_service_exec(),
            'ui': manifest.get_ui_exec(),
            'watch': manifest.get_watch_exec(),
            'widget': manifest.get_widget_exec(),
        }
    }))
