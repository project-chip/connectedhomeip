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

source idt/scripts/vars.sh
if [[ -d idt/venv ]]; then
    echo "TEMP MOVING venv"
    mv idt/venv TEMPvenv
fi
if [[ -d "idt/$IDT_OUTPUT_DIR" ]]; then
    echo "TEMP MOVING IDT_OUTPUT_DIR"
    mv "idt/$IDT_OUTPUT_DIR" "TEMP""$IDT_OUTPUT_DIR"
fi
if [[ -d idt/pycache ]]; then
    echo "TEMP moving pycache"
    mv idt/pycache TEMPpycache
fi

scp -r ./idt/* "$PIUSER@$PIHOST:/home/$PIUSER"/idt

if [[ -d TEMPvenv ]]; then
    mv TEMPvenv idt/venv
    echo "venv restored"
fi
if [[ -d "TEMP"$IDT_OUTPUT_DIR ]]; then
    mv "TEMP""$IDT_OUTPUT_DIR" "idt/$IDT_OUTPUT_DIR"
    echo "IDT_OUTPUT_DIR restored"
fi
if [[ -d "idt/$IDT_OUTPUT_DIR" ]]; then
    echo "pycache restored"
    mv TEMPpycache idt/pycache
fi
