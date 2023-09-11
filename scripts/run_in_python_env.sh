#!/usr/bin/env bash
#
# SPDX-FileCopyrightText: 2020 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

# This script executes the command given as an argument after
# activating the given python virtual environment
set -e

_VENV_PATH="$1"
shift

source "$_VENV_PATH/bin/activate"

echo "Executing in python environment $_VENV_PATH: $*"
bash -c -- "$@"
