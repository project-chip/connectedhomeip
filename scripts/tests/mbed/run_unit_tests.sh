#!/bin/bash

set -e

if [ $# -lt 4 ]; then
    echo "Illegal number of parameters. Please define Github parameters: repository name, action ID, access token and CHIP directory path"
    exit 1
fi

GITHUB_REPOSITORY=$1
GITHUB_ACTION_ID=$2
GITHUB_TOKEN=$3
CHIP_DIR=$4

source $CHIP_DIR/scripts/tests/mbed/common.sh

echo "Downloading artifact from $GITHUB_ACTION_ID action in $GITHUB_REPOSITORY"
download_artifacts_gh $GITHUB_REPOSITORY $GITHUB_ACTION_ID $GITHUB_TOKEN binaries

target_number=$(cat devices.json | jq length)

for index in $(eval echo "{0..$(($target_number - 1))}"); do
    platform_name=$(cat devices.json | jq -r ".[$index] .platform_name")
    echo "Flash unit-tests application image to $platform_name device"
    flash_image_to_device unit-tests "$platform_name" binaries
    echo "Run unit tests on $platform_name device"
    pytest -rAV --platforms="$platform_name" $CHIP_DIR/src/test_driver/mbed/unit_tests/pytest/test_unittests.py
done
