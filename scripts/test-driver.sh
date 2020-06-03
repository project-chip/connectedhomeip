#!/usr/bin/env bash

#
#    Copyright (c) 2020 Project CHIP Authors
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

#
#    Description:
#      This file implements a wrapper for autoconf's test-driver script
#       that show's tests output during the build
#
#    Use me like this:
#       make LOG_DRIVER=<abs path to this file> ...
#

here=${0%/*}

set -e

${here}/../build/autoconf/test-driver "$@"

while ((${#@})); do
    [[ $1 == "--log-file" ]] && {
        cat "$2" && break
    }
    shift
done
