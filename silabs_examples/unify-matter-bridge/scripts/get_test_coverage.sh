#!/usr/bin/env bash

while getopts bt: flag
do
    case "${flag}" in
        b) BUILD_DIR=${OPTARG};;
        t) TARGET_ARCH=${OPTARG};;
    esac
done

if [ -z "$TARGET_ARCH" ]; then
    TARGET_ARCH="arm"
fi

if [ -z "$BUILD_DIR" ]
then
    echo "Building and running tests before calculating coverage"
    BUILD_DIR="out/${TARGET_ARCH}_test"
    $( dirname -- "${BASH_SOURCE[0]}" )/compile_tests.sh -t ${TARGET_ARCH}
    $( dirname -- "${BASH_SOURCE[0]}" )/run_tests.sh -b ${BUILD_DIR}
fi

# Compile the coverage report of the unit tests
cd $BUILD_DIR; gcovr -r ../.. -f ../../src -e ../../src/tests --html-details coverage.html