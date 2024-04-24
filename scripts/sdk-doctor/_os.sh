#!/bin/bash

#
# Copyright (c) 2024 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

ROOT_DIR=$(realpath "$(dirname "$0")"/../..)
cd "$ROOT_DIR"

# Function to display OS information
get_os_info() {
    if [ -f /etc/os-release ]; then
        # If available, use /etc/os-release file
        . /etc/os-release
        echo "Name: $NAME"
        echo "Version: $VERSION"
        echo "ID: $ID"
        echo "ID Like: $ID_LIKE"
    elif [ -f /etc/*-release ]; then
        # If /etc/os-release is not available, use other available /etc/*-release file
        echo "OS Information from /etc/*-release:"
        cat /etc/*-release
    else
        # Print a message if unable to determine OS information
        echo "Cannot determine OS information."
    fi
}

# Function to display kernel information
get_kernel_info() {
    echo "Kernel Information:"
    uname -a | fold -w 88 -s
}

# Function to display CPU information
get_cpu_info() {
    echo "CPU Information:"
    lscpu | grep -E "^Architecture:|^CPU op-mode\(s\):|^CPU\(s\):|^Vendor ID:|^Model name:|^CPU max MHz:|^CPU min MHz:" |
        sed 's/^[ \t]*//;s/[ \t]*$//'
}

# Function to display memory information
get_memory_info() {
    echo "Memory Information:"
    free -h
}

# Call the functions to display the information
get_os_info
echo

get_kernel_info
echo

get_cpu_info
echo

get_memory_info
