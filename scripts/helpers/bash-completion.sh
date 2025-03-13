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

_chip_app() {

    local cur prev words cword split
    _init_completion -s || return

    case "$prev" in
        --ble-device)
            readarray -t words < <(ls -I '*:*' /sys/class/bluetooth)
            # Get the list of Bluetooth devices without the 'hci' prefix.
            readarray -t COMPREPLY < <(compgen -W "${words[*]#hci}" -- "$cur")
            return
            ;;
        --custom-flow)
            readarray -t COMPREPLY < <(compgen -W "0 1 2" -- "$cur")
            return
            ;;
        --capabilities)
            # The capabilities option is a bit-field with 3 bits currently defined.
            readarray -t COMPREPLY < <(compgen -W "001 010 011 100 101 111" -- "$cur")
            return
            ;;
        --KVS)
            _filedir
            return
            ;;
        --PICS)
            _filedir
            return
            ;;
        --trace_file)
            _filedir
            return
            ;;
        --trace_log | --trace_decode)
            readarray -t COMPREPLY < <(compgen -W "0 1" -- "$cur")
            return
            ;;
        --trace-to)
            readarray -t COMPREPLY < <(compgen -W "json perfetto" -- "$cur")
            compopt -o nospace
            return
            ;;
    esac

    case "$cur" in
        -*)
            readarray -t COMPREPLY < <(compgen -W "$(_parse_help "$1")" -- "$cur")
            ;;
    esac

}

_chip_tool() {

    local cur prev words cword split
    _init_completion -s || return

    # Get command line arguments up to the cursor position
    local args=("${COMP_WORDS[@]:0:$cword+1}")

    case "$prev" in
        --commissioner-name)
            readarray -t COMPREPLY < <(compgen -W "alpha beta gamma 4 5 6 7 8 9" -- "$cur")
            return
            ;;
        --only-allow-trusted-cd-keys)
            readarray -t COMPREPLY < <(compgen -W "0 1" -- "$cur")
            return
            ;;
        --paa-trust-store-path | --cd-trust-store-path | --dac-revocation-set-path)
            _filedir -d
            return
            ;;
        --storage-directory)
            _filedir -d
            return
            ;;
        --trace-to)
            readarray -t COMPREPLY < <(compgen -W "json perfetto" -- "$cur")
            compopt -o nospace
            return
            ;;
    esac

    case "$cur" in
        -*)
            words=$(_chip_tool_get_options "${args[@]}")
            ;;
        *)
            words=$(_chip_tool_get_commands "${args[@]}")
            ;;
    esac
    readarray -t COMPREPLY < <(compgen -W "$words" -- "$cur")

}

complete -F _chip_app chip-air-purifier-app
complete -F _chip_app chip-all-clusters-app
complete -F _chip_app chip-bridge-app
complete -F _chip_app chip-dishwasher-app
complete -F _chip_app chip-energy-management-app
complete -F _chip_app chip-lighting-app
complete -F _chip_app chip-lock-app
complete -F _chip_app chip-log-source-app
complete -F _chip_app chip-microwave-oven-app
complete -F _chip_app chip-ota-provider-app
complete -F _chip_app chip-ota-requestor-app
complete -F _chip_app chip-refrigerator-app
complete -F _chip_app chip-rvc-app
complete -F _chip_app chip-tv-app
complete -F _chip_app chip-tv-casting-app

complete -F _chip_tool chip-tool
