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

import logging

from res.splash import splash as _splash

IDT_VERSION = "1.0.0"
DEBUG = False
ENABLE_COLOR = True
LOG_LEVEL = logging.DEBUG if DEBUG else logging.INFO
PY_MAJOR_VERSION = 3
PY_MINOR_VERSION = 11
SPLASH = _splash
# TODO: Model this differently, e.g. dataclass: help, link, reason etc.?
# TODO: Script to autodetect dependencies used in the repo to make maintaining this easier
HOST_DEPENDENCIES = {
    "ALL": {
        "adb":
            "adb is required for interacting with Android devices: "
            "https://developer.android.com/studio/command-line/adb",
        "tcpdump":
            "tcpdump is required for taking host packet captures. It should be available on all systems.",
        "git":
            "git is required for downloading build tools. It should already be present on a system using this tool.",
        "tee":
            "tee is required to write and view logs at the same time. It should be available on all systems."
    },
    "LINUX": {
        "airmon-ng":
            "The aircrack suite is used to manage processes and interfaces for monitor mode pcaps"
            "https://www.aircrack-ng.org/doku.php?id=install_aircrack#installing_pre-compiled_binaries",
    },
    "MAC": {
        "airport":
            "Airport is required for taking monitor mode packet captures on the idt host."
            "It is likely on your system but not on your path."
            "Try looking for the binary in a path like this"
            "/System/Library/PrivateFrameworks/Apple80211.framework/Versions/A/Resources/airport"
            "and add it to your path!",
        # "brew":
        #     "brew is required for installing otbrrcp build tools on MacOS: https://brew.sh/",
    },
}
