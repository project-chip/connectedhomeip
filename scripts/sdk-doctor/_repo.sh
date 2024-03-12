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

get_repo_and_branch_info() {
    # Input validation
    if [ -z "$1" ]; then
        echo "Please provide a path."
        return 1
    fi

    path="$1"
    repo_friendly_name="Matter SDK"

    if [ "$path" != "." ]; then
        title_case_path=$(echo "$path" | awk '{ for(i=1;i<=NF;i++) $i=toupper(substr($i,1,1)) tolower(substr($i,2)); }1')
        repo_friendly_name=$title_case_path
    fi

    # Check if the directory exists
    if [ ! -d "$path" ]; then
        echo "Directory '$path' does not exist."
        return 1
    fi

    cd "$path"

    # Get the URL of the remote origin
    remote_url=$(git config --get remote.origin.url)

    if [ -n "$remote_url" ]; then
        # Extract the repository name from the URL
        repo_name=$(basename -s .git "$remote_url")

        # Calculate the necessary padding to align the end pipe
        total_length=95                                    # Adjust this based on your frame width
        text_length=${#repo_friendly_name}+${#repo_name}+4 # 4 for the ": " and two spaces around the repo name
        padding_length=$((total_length - text_length))

        echo '+-----------------------------------------------------------------------------------------------+'
        printf "|  %s: %s%*s|\n" "$repo_friendly_name" "$repo_name" "$padding_length" ""
        echo '+-----------------------------------------------------------------------------------------------+'
    else
        # Print error message if there is no remote URL
        echo "Not a Git repository or no remote set"
        return 1
    fi

    # Get the current branch and its tracking branch
    git_status=$(git status)
    tracking_branch_info=$(echo "$git_status" | grep "Your branch is up to date with")

    # Extract the fork owner and branch from the tracking branch info
    if [[ $tracking_branch_info =~ Your\ branch\ is\ up\ to\ date\ with\ \'([^\']+)\' ]]; then
        fork_owner_and_branch="${BASH_REMATCH[1]}"
    else
        fork_owner_and_branch="Not set or not a tracking branch"
    fi

    # Get the commit SHA of the current HEAD
    commit_sha=$(git rev-parse HEAD)
    echo "Commit SHA: $commit_sha"

    # Get the commit message of the current HEAD
    commit_message=$(git log -1 --pretty=format:"%B")
    trimmed_commit_message=$(trim_commit_message "$commit_message")
    echo "Commit Message: $trimmed_commit_message"

    # Get the commit author of the current HEAD
    commit_author=$(git log -1 --pretty=format:"%an")
    echo "Commit Author: $commit_author"

    # Get the commit date and time of the current HEAD including timezone
    commit_datetime=$(git log -1 --pretty=format:"%cd" --date=format:"%Y-%m-%d %H:%M:%S %z")
    echo "Commit Date: $commit_datetime"

    # Attempt to find branches that contain this commit
    branches=$(git branch --contains "$commit_sha" | sed 's/^/    /')

    if [ -n "$branches" ]; then
        echo "Contained in branches:"
        echo "$branches"
    else
        echo "This commit is not on any known branch."
    fi

    echo "    Tracking: $fork_owner_and_branch"

    echo

    # Navigate back to the original directory
    cd "$ROOT_DIR"
}

trim_commit_message() {
    local commit_message="$1"

    # Check if the commit message contains a newline character
    if [[ "$commit_message" == *$'\n'* ]]; then
        # Extract the first line of the commit message
        local first_line="${commit_message%%$'\n'*}"
    else
        # If there's no newline, use the entire commit message
        local first_line="$commit_message"
    fi

    # Trim leading and trailing whitespace from the first line and echo it
    echo "$first_line" | sed 's/^[ \t]*//;s/[ \t]*$//'
}

# Print SDK root git status
get_repo_and_branch_info "."

# Handle arguments
case "$1" in
    --git-sub)
        # Initialize an array to hold the directories
        declare -a repo_dirs

        cd "$ROOT_DIR"

        # Find directories containing a .github folder and store them in the array, excluding the current directory
        while IFS= read -r dir; do
            # Check if the directory is not the current directory
            if [[ "$dir" != "." ]]; then
                repo_dirs+=("$dir")
            fi
        done < <(find . -type d -name .github | awk -F'/[^/]*$' '{print $1}')

        # Iterate through the directories and call the function for each
        for dir in "${repo_dirs[@]}"; do
            get_repo_and_branch_info "$dir"
        done
        ;;
    *) ;;
esac
