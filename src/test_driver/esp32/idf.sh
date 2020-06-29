#!/usr/bin/env bash
#
#   Copyright (c) 2020 Project CHIP Authors
#   All rights reserved.
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.

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
