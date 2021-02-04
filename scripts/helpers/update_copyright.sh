#!/usr/bin/env bash

#
# Copyright (c) 2021 Project CHIP Authors
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

# This script extends copyright messages of the form
#   Copyright (c) 20xx[-20xx] Project CHIP Authors
# to include the current year.

SELF=${0##*/}

function git_active_files
{
    typeset -n _a=$1
    typeset file root=$(git rev-parse --show-toplevel)
    shopt -s lastpipe
    git status --ignore-submodules --porcelain |
        sed -n -e 's/^[^!?D]. //p' |
        while read -r file
        do
            [[ -n $file ]] && _a+=("$root/$file")
        done
}

function git_files_for_commit
{
    typeset -n _c=$1
    shift
    typeset file root=$(git rev-parse --show-toplevel)
    shopt -s lastpipe
    git --no-pager show --name-only --pretty=format: "$@" |
        while read -r file
        do
            _c+=("$root/$file")
        done
}

function usage
{
    {
        echo "Usage: $SELF [-a] [-c git-object] [file ...]"
        echo "  -a              -- update the active files according to git"
        echo "  -c git-object   -- update the files changed in git-object"
    } >&2
    exit 1
}

copyright='Copyright (c)'
authors='Project CHIP Authors'
year=$(date +%Y)
use_active_files=
typeset -a files

while getopts C:A:Y:ac: c
do
    case $c in
        C)  copyright=$OPTARG;;
        A)  authors=$OPTARG;;
        Y)  year=$OPTARG;;
        a)  use_active_files=1;;
        c)  git_files_for_commit files "$OPTARG";;
        ?)  usage
    esac
done
shift $((OPTIND-1))
files+=("$@")

if ((year > 2099))
then
    echo >&2 "$SELF: Not Y2.1K compliant. Please contact the original author for a fix."
    exit 1
fi

if [[ -n "$use_active_files" ]]
then
    git_active_files files
    if [[ -z "${files[0]}" ]]
    then
        echo >&2 "$SELF: No active files."
        exit 1
    fi
elif [[ -z "${files[0]}" ]]
then
    echo >&2 "$SELF: no files given."
    typeset -a gits
    git_active_files gits
    if [[ -n "${gits[0]}" ]]
    then
        echo >&2 "Using '$SELF -a' will try the active git files, which currently are:"
        for i in "${gits[@]}"
        do
            echo >&2 "  $i"
        done
        exit 1
    fi
    usage
fi

sed -i \
  -e "s/$copyright \(20[0-9][0-9]\) $authors/$copyright \1-$year $authors/" \
  -e "s/$copyright \(20[0-9][0-9]\)-20[0-9][0-9] $authors/$copyright \1-$year $authors/" \
  -e "s/$copyright $year-$year $authors/$copyright $year $authors/" \
  "${files[@]}"
