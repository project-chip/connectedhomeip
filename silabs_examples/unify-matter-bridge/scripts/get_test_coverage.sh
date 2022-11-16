#!/usr/bin/env bash

while getopts b: flag
do
    case "${flag}" in
        b) BUILD_DIR=${OPTARG};;
    esac
done

if [ -z "$BUILD_DIR" ]
then
    echo "Building and running tests before calculating coverage"
    BUILD_DIR="out/arm_test"
    $( dirname -- "${BASH_SOURCE[0]}" )/compile_tests.sh
    $( dirname -- "${BASH_SOURCE[0]}" )/run_tests.sh -b ${BUILD_DIR}
fi

# Compile the coverage report of the unit tests
cd $BUILD_DIR; gcovr -r ../.. -f ../../src -e ../../src/tests --html-details coverage.html