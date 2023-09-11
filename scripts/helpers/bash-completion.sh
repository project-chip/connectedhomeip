#!/bin/bash

#
# SPDX-FileCopyrightText: 2023 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
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
