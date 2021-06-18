#!/bin/bash

set -e

if [ $# -lt 5 ]; then
    echo "Illegal number of parameters. Please define Github parameters: repository name, build apps and chip-tools action IDs, access token and CHIP source directory paths"
    exit 1
fi

GITHUB_REPOSITORY=$1
GITHUB_BUILD_APP_ACTION_ID=$2
GITHUB_CHIP_TOOLS_BUILD_ACTION_ID=$3
GITHUB_TOKEN=$4
CHIP_DIR=$5
CHIP_TOOLS_DIR=chip-tools

source $CHIP_DIR/scripts/tests/mbed/common.sh

echo "Downloading applications images from $GITHUB_BUILD_APP_ACTION_ID action in $GITHUB_REPOSITORY"
download_artifacts_gh $GITHUB_REPOSITORY $GITHUB_BUILD_APP_ACTION_ID $GITHUB_TOKEN app_images

echo "Downloading chip-tools image from $GITHUB_CHIP_TOOLS_BUILD_ACTION_ID action in $GITHUB_REPOSITORY"
download_artifacts_gh $GITHUB_REPOSITORY $GITHUB_CHIP_TOOLS_BUILD_ACTION_ID $GITHUB_TOKEN $CHIP_TOOLS_DIR

# Install Python Chip Device Controller
pip install $CHIP_TOOLS_DIR/controller/python/chip*.whl
pip install -r $CHIP_DIR/src/test_driver/mbed/integration_tests/requirements.txt

echo "Get test sets"
mapfile -t test_sets <$CHIP_DIR/src/test_driver/mbed/integration_tests/test_set.in

target_number=$(cat devices.json | jq length)

for index in $(eval echo "{0..$(($target_number - 1))}"); do
    platform_name=$(cat devices.json | jq -r ".[$index] .platform_name")
    for test in "${test_sets[@]}"; do
        echo "Flash $test application image to $platform_name device"
        flash_image_to_device "$test" "$platform_name" app_images
        echo "Run integration test for $test application image on $platform_name device"
        pytest -rAV --platforms="$platform_name" --network=$AP_SSID:$AP_PASSWORD --echo_server=$AP_GATEWAY:$ECHO_SERVER_PORT --chip_tools_dir=$CHIP_TOOLS_DIR $CHIP_DIR/src/test_driver/mbed/integration_tests/"$test"
    done
done
