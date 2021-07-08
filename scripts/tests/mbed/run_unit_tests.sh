#!/bin/bash

set -e

if [ $# -lt 4 ]; then
    echo "Illegal number of parameters. Please define Github parameters: repository name, action ID, access token and CHIP directory path"
    exit 1
fi

GITHUB_REPOSITORY=$1
GITHUB_APP_BUILD_ACTION_ID=$2
GITHUB_TOKEN=$3
CHIP_DIR=$4

source $CHIP_DIR/scripts/tests/mbed/common.sh

echo "Downloading unit test application from $GITHUB_APP_BUILD_ACTION_ID action in $GITHUB_REPOSITORY"
download_artifacts_gh $GITHUB_REPOSITORY $GITHUB_APP_BUILD_ACTION_ID $GITHUB_TOKEN app_images

target_number=$(cat devices.json | jq length)

set +e

errorCounter=0

for index in $(eval echo "{0..$(($target_number - 1))}"); do
    platform_name=$(cat devices.json | jq -r ".[$index] .platform_name")
    echo "Flash unit-tests application image to $platform_name device"
    flash_image_to_device unit-tests "$platform_name" app_images
    if [ $? -eq 0 ]; then
        echo "Run unit tests on $platform_name device"
        pytest -rAV --platforms="$platform_name" $CHIP_DIR/src/test_driver/mbed/unit_tests/pytest/test_unittests.py
        if [ $? -ne 0 ]; then
            echo "Unit tests on $platform_name device failed"
            ((errorCounter++))
        fi
    else
        echo "Flash unit-tests application image to $platform_name device failed"
        ((errorCounter++))
    fi
done

exit $errorCounter
