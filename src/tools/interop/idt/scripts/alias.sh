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

if [[ $SHELL == "/bin/zsh" ]]; then
    echo "idt using zsh config"
    export IDT_SRC_PARENT="$(dirname "$0")/../.."
else
    echo "idt using bash config"
    export IDT_SRC_PARENT="$(dirname "${BASH_SOURCE[0]:-$0}")/../.."
fi

export IDT_OUTPUT_DIR="IDT_ARTIFACTS"

alias idt_dir="echo \"idt dir $IDT_SRC_PARENT\""
idt_dir
alias idt_go="cd \"$IDT_SRC_PARENT\""

alias idt_activate="idt_go && source idt/scripts/activate.sh"
alias idt_bootstrap="idt_go && source idt/scripts/bootstrap.sh"
alias idt_build="idt_go && source idt/scripts/build.sh"
alias idt_clean="idt_go && source idt/scripts/clean.sh"
alias idt_connect="idt_go && source idt/scripts/connect.sh"
alias idt_fetch_artifacts="idt_go && source idt/scripts/fetch_artifacts.sh"
alias idt_prune_docker="idt_go && source idt/scripts/prune_docker.sh"
alias idt_push="idt_go && source idt/scripts/push.sh"
alias idt_vars="idt_go && source idt/scripts/vars.sh"
alias idt_clean_artifacts="idt_go && source idt/scripts/clean_artifacts.sh"
alias idt_clean_all="idt_go && source idt/scripts/clean_all.sh"
alias idt_create_vars="idt_go && source idt/scripts/create_vars.sh"
alias idt_check_child="idt_go && source idt/scripts/check_child.sh"
alias idt_clean_child="idt_go && source idt/scripts/clean_child.sh"

alias idt="idt_go && \
if [ -z $PYTHONPYCACHEPREFIX ]; then export PYTHONPYCACHEPREFIX=$IDT_SRC_PARENT/idt/pycache; fi && \
if [ -z $VIRTUAL_ENV]; then source idt/scripts/py_venv.sh; fi && \
python3 idt "

echo "idt commands available! type idt and press tab twice to see available commands."
