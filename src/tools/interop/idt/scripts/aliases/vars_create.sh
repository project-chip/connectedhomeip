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

host_prompt="Enter RPi host name "
user_prompt="Enter RPi user name "

if [[ $SHELL == "/bin/zsh" ]]; then
    read "pihost?$host_prompt"
    read "piuser?$user_prompt"
else
    read -p "$host_prompt" pihost
    read -p "$user_prompt" piuser
fi

echo "export PIHOST=\"$pihost\"" >"$IDT_SRC_PARENT"/idt/scripts/vars.sh
echo "export PIUSER=\"$piuser\"" >>"$IDT_SRC_PARENT"/idt/scripts/vars.sh
