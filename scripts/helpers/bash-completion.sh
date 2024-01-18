#!/bin/bash

#
# Copyright (c) 2023 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# Get the list of commands from the output of the chip-tool,
# where each command is prefixed with the ' | * ' string.
_chip_tool_get_commands() {
    "$@" --help 2>&1 | awk '/ [|] [*] /{ print $3 }'
}

# Get the list of options from the output of the chip-tool,
# where each option starts with the '[--' string.
_chip_tool_get_options() {
    "$@" --help 2>&1 | awk -F'[[]|[]]' '/^[[]--/{ print $2 }'
}

_chip_tool() {

    local cur prev words cword split
    _init_completion -s || return

    # Get command line arguments up to the cursor position
    local args=("${COMP_WORDS[@]:0:$cword+1}")

    local command=0
    case "$prev" in
        --commissioner-name)
            readarray -t COMPREPLY < <(compgen -W "alpha beta gamma 4 5 6 7 8 9" -- "$cur")
            ;;
        --paa-trust-store-path | --cd-trust-store-path)
            _filedir -d
            ;;
        --storage-directory)
            _filedir -d
            ;;
        *)
            command=1
            ;;
    esac

    if [ "$command" -eq 1 ]; then
        case "$cur" in
            -*)
                words=$(_chip_tool_get_options "${args[@]}")
                ;;
            *)
                words=$(_chip_tool_get_commands "${args[@]}")
                ;;
        esac
        readarray -t COMPREPLY < <(compgen -W "$words" -- "$cur")
    fi

}

complete -F _chip_tool chip-tool
