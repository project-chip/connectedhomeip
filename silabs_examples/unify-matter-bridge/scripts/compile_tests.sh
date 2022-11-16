#!/bin/bash

set -eo pipefail

while getopts t: flag
do
    case "${flag}" in
        t) TARGET_ARCH=${OPTARG};;
    esac
done

if [ -z "$TARGET_ARCH" ]
then
    echo "Compiling tests for the default target architecture armhf"
    TARGET_ARCH="arm"
fi
BUILD_DIR="out/${TARGET_ARCH}_test"

# Generate ninja files and compile the unit tests
if [ "$TARGET_ARCH" = "amd64" ]; then
    gn gen ${BUILD_DIR} --args='chip_build_tests=true use_coverage=true'
else 
    gn gen ${BUILD_DIR} --args='target_cpu="'$TARGET_ARCH'" chip_build_tests=true use_coverage=true'
fi

ninja -C ${BUILD_DIR} check