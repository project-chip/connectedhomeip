#!/bin/bash

set -eo pipefail

RED='\033[0;31m'
NC='\033[0m'       # No Color
GREEN='\033[0;32m' # Green

while getopts b: flag
do
    case "${flag}" in
        b) BUILD_DIR=${OPTARG};;
    esac
done

if [ -z "$BUILD_DIR" ]
then
    echo "Running tests from default build directory out/armhf_test"
    BUILD_DIR="out/arm_test"
fi

# Execute all the unit tests
testExecutables=$(find ./${BUILD_DIR}/tests -type f -executable)
failed_tests=0
passed_tests=0
for testExecutable in $testExecutables; do
    echo Running test: $testExecutable
    if $testExecutable; then
        passed_tests=$((passed_tests + 1))
    else
        failed_tests=$((failed_tests + 1))
    fi
done

echo ""
echo -e "${GREEN}Number of passed tests: $passed_tests ${NC}"
echo -e "${RED}Number of failed tests: $failed_tests ${NC}"

if [ $failed_tests -ne 0 ]; then
    exit 1
fi
