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

# The IDT installation directory is located via relative path to this file.
# Different shells offer different facilities for checking the path of a sourced file at run time,
# so we have different configs here accordingly.
if [[ $SHELL == "/bin/zsh" ]]; then
    echo "idt using zsh config"
    export IDT_SRC_PARENT="$(dirname "$0")/../.."
else
    echo "idt using bash config"
    export IDT_SRC_PARENT="$(dirname "${BASH_SOURCE[0]:-$0}")/../.."
fi

# Make sure these are setup before anything else runs
export IDT_OUTPUT_DIR="IDT_ARTIFACTS"
alias idt_dir="echo \"idt dir $IDT_SRC_PARENT\""
alias idt_go="cd \"$IDT_SRC_PARENT\""

# One time setup should not be aliases
alias idt_vars="idt_go && source idt/scripts/vars.sh"

# Setting things up this way allows us to edit the source of the aliases without having to manually call
# source / relaunch shell for changes to take effect (unless changing the alias declaration itself)
# also, if the script is deleted, an old shell won't have access to run the alises anymore, which is desired
alias_src_path="idt_go && source idt/scripts/aliases"
alias idt_artifacts_fetch="$alias_src_path/artifacts_fetch.sh"
alias idt_child_check="$alias_src_path/child_check.sh"
alias idt_child_kill="$alias_src_path/child_kill.sh"
alias idt_clean_all="$alias_src_path/clean_all.sh"
alias idt_clean_artifacts="$alias_src_path/clean_artifacts.sh"
alias idt_connect="$alias_src_path/connect.sh"
alias idt_delete="$alias_src_path/delete.sh"
alias idt_docker_build="$alias_src_path/docker_build.sh"
alias idt_docker_prune="$alias_src_path/docker_prune.sh"
alias idt_docker_run="$alias_src_path/docker_run.sh"
alias idt_linux_bootstrap="$alias_src_path/linux_bootstrap.sh"
alias idt_push="$alias_src_path/push.sh"
alias idt_vars_create="$alias_src_path/vars_create.sh"
alias idt_linux_install_compilers_for_arm_tcpdump="$alias_src_path/linux_install_compilers_for_arm_tcpdump.sh"

# Setup the venv if it doesn't exist; ensure byte code cache is compartmentalized
alias idt="idt_go && \
if [ -z $PYTHONPYCACHEPREFIX ]; then export PYTHONPYCACHEPREFIX=$IDT_SRC_PARENT/idt/pycache; fi && \
if [ -z $VIRTUAL_ENV ]; then source idt/scripts/py_venv.sh; fi && \
python3 idt "

idt_dir
echo "idt commands available! type idt and press tab twice to see available commands."
