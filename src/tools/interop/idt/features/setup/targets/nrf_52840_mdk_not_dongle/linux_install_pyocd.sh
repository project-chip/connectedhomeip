#!/bin/bash
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

set -e
# Reference https://pyocd.io/docs/installing_on_non_x86.html
# The version is pinned here because we manually copy requirements from
# https://github.com/pyocd/pyOCD/blob/v0.36.0/setup.cfg
# as instructed by the reference above, to support RPi
# TODO: Script to auto update this file based on input version
pip install --no-deps pyocd==0.36.0
requirements=$(cat <<-END
capstone>=4.0,<5.0
colorama<1.0
importlib_metadata>=3.6
importlib_resources
intelhex>=2.0,<3.0
intervaltree>=3.0.2,<4.0
lark>=1.1.5,<2.0
libusb-package>=1.0,<2.0
natsort>=8.0.0,<9.0
prettytable>=2.0,<4.0
pyelftools<1.0
pylink-square>=1.0,<2.0
pyusb>=1.2.1,<2.0
pyyaml>=6.0,<7.0
six>=1.15.0,<2.0
typing-extensions>=4.0,<5.0
END

)

echo "$requirements" > linux_pyocd_requirements.txt
pip install -r linux_pyocd_requirements.txt

# Reference https://pyocd.io/docs/installing#udev-rules-on-linux
set -e
git clone https://github.com/pyocd/pyOCD.git
cd pyOCD
git checkout v0.36.0
cd udev
sudo cp *.rules /etc/udev/rules.d
