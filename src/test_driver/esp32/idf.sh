#!/usr/bin/env bash
#
# SPDX-FileCopyrightText: 2020 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0

# If this file is sourced, it exports a function called "idf" that initializes
# the espressif environment via the espressive export.sh script and runs
# a command presented as arguments
#
# This file can also be used as an executable

error() {
    echo "${me:?}: *** ERROR: " "${*}"
}
idf() {
    [[ -d $IDF_PATH && -r $IDF_PATH/export.sh ]] || {
        error "can't find IDF's export.sh, please set IDF_PATH"
        return 1
    }
    (
        # shellcheck source=/dev/null
        . "$IDF_PATH/export.sh"
        export IDF_PATH
        "$@"
    )
}
if [[ ${0} == "${BASH_SOURCE[0]}" ]]; then
    me=${0##*/}
    idf "${@}"
else
    me=idf
    [[ $PS1 =~ \[idf\].* ]] || PS1="[idf]$PS1"
fi
