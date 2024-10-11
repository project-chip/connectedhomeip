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

{
    echo "          __  __       _   _               _____ _____  _  __  _____             _             "
    echo "         |  \/  |     | | | |             / ____|  __ \| |/ / |  __ \           | |            "
    echo "         | \  / | __ _| |_| |_ ___ _ __  | (___ | |  | | ' /  | |  | | ___   ___| |_ ___  _ __ "
    echo '         | |\/| |/ _` | __| __/ _ \ `__|  \___ \| |  | |  <   | |  | |/ _ \ / __| __/ _ \| `__|'
    echo "         | |  | | (_| | |_| ||  __/ |     ____) | |__| | . \  | |__| | (_) | (__| || (_) | |   "
    echo "         |_|  |_|\__,_|\__|\__\___|_|    |_____/|_____/|_|\_\ |_____/ \___/ \___|\__\___/|_|   "
    echo "                                                                                               "

    usage() {
        echo "Displays Matter SDK setup information."
        echo
        echo "Usage:"
        echo
        echo "    Show Matter SDK Version, SHA, repository details:  $0"
        echo "    Show Matter SDK git submodule information:         $0 --git-sub"
        echo "    Show Matter SDK host OS system information:        $0 --system"
        echo "    Show all available Matter SDK details:             $0 --complete"
        echo
    }

    width=104

    # Get the date string
    date_string=$(date)

    # Calculate the length of the date string
    date_length=${#date_string}

    # Calculate the padding on each side
    padding=$(((width - date_length) / 2))

    # Print spaces for left padding
    printf "%${padding}s"

    # Print the date string
    echo "$date_string"
    echo

    ROOT_DIR=$(realpath "$(dirname "$0")"/../..)
    TH_DEV_SCRIPTS_DIR=$ROOT_DIR/scripts/sdk-doctor
    cd "$ROOT_DIR"

    # Check for arguments
    if [ "$#" -gt 1 ]; then
        echo "Error: Too many arguments."
        usage
        exit 1
    fi

    print_framed_text() {
        # Get the text and title from function arguments
        input_text="$1"
        title="$2"

        # Ensure 'width' is set and has a reasonable value
        if [ -z "$width" ] || [ "$width" -lt 10 ]; then
            echo "Error: 'width' is not set or too small."
            return 1
        fi

        max_line_length=$((width - 6)) # Maximum characters in a line before wrapping

        # Word-wrap the input text
        input_text_wrapped=$(echo -e "$input_text" | fold -w "$max_line_length" -s)

        # Calculate height based on the number of lines in the input text
        height=$(echo -e "$input_text_wrapped" | wc -l)
        height=$((height + 4)) # Add 4 to account for the top and bottom frame borders and inner padding

        # Print the top border with title
        title_with_padding=" $title "
        title_padding_left=$(((width - 2 - ${#title_with_padding}) / 2))
        [ "$title_padding_left" -lt 0 ] && title_padding_left=0
        title_padding_right=$((width - 2 - ${#title_with_padding} - title_padding_left))
        [ "$title_padding_right" -lt 0 ] && title_padding_right=0
        echo '+'"$(printf "%0.s-" "$(seq 1 "$title_padding_left")")$title_with_padding""$(printf "%0.s-" "$(seq 1 "$title_padding_right")")"'+'

        # Inner top padding
        echo "|$(printf ' %.0s' "$(seq 1 $((width - 2)))")|"

        # Print each line of wrapped input text with frame borders and padding
        echo -e "$input_text_wrapped" | while IFS= read -r line; do
            padding_right=$((width - 4 - ${#line} - 2)) # Subtract 4 for the borders and 2 for the left padding
            [ "$padding_right" -lt 0 ] && padding_right=0
            echo "|  $line$(printf ' %.0s' "$(seq 1 "$padding_right")")  |"
        done

        # Inner bottom padding
        echo "|$(printf ' %.0s' "$(seq 1 $((width - 2)))")|"

        # Print the bottom border
        echo '+'"$(printf "%0.s-" "$(seq 1 $((width - 2)))")"'+'
        echo
    }

    show_system() {
        # OS
        os_output=$("$TH_DEV_SCRIPTS_DIR/_os.sh")
        print_framed_text "$os_output" "OS"

        # Network
        network_output=$("$TH_DEV_SCRIPTS_DIR/_network.sh")
        print_framed_text "$network_output" "Network"
    }

    # Matter SDK Version
    th_version_output=$("$TH_DEV_SCRIPTS_DIR/_version.sh")
    print_framed_text "$th_version_output" "Matter SDK Version"

    # Git Status
    if [[ "$1" = "--git-sub" ]] || [[ "$1" = "--complete" ]]; then
        repository_branches_output=$("$TH_DEV_SCRIPTS_DIR/_repo.sh" "--git-sub")
        print_framed_text "$repository_branches_output" "Git Status"
    else
        repository_branches_output=$("$TH_DEV_SCRIPTS_DIR/_repo.sh" "--git")
        print_framed_text "$repository_branches_output" "Git Status"
    fi

    # Handle arguments
    case "$1" in
        --system)
            show_system
            usage
            ;;
        --complete)
            show_system
            usage
            ;;
        *)
            usage
            exit 1
            ;;
    esac

} 2>&1 | tee sdk-doctor.txt
