#!/bin/bash

set -o pipefail

RED='\033[0;31m'
NC='\033[0m'       # No Color
GREEN='\033[0;32m' # Green

VERBOSE=0
while getopts b:vt: flag
do
    case "${flag}" in
        b) BUILD_DIR=${OPTARG};;
        v) VERBOSE=1;;
        t) TEST_TO_EXECUTE=${OPTARG};;
    esac
done

if [ -z "$BUILD_DIR" ]
then
    echo "Running tests from default build directory out/armhf_test"
    BUILD_DIR="out/arm_test"
fi

# Execute either all or one unit test
if [ -z "$TEST_TO_EXECUTE" ]
then
    testExecutables=$(find ./${BUILD_DIR}/tests -type f -executable)
else
    testExecutables=$(find ./${BUILD_DIR}/tests -name $TEST_TO_EXECUTE -type f -executable)
fi

failed_tests=0
passed_tests=0
for testExecutable in $testExecutables; do
    testName=$(basename -- $testExecutable)
    if [ $VERBOSE -eq 1 ]; then
        $testExecutable
    else
        $testExecutable &>/dev/null
    fi
    if [ $? -eq 0 ]; then
        passed_tests=$((passed_tests + 1))
        echo -e "Running test: $testName:\t ${GREEN}Passed${NC}"
    else
        failed_tests=$((failed_tests + 1))
        echo -e "Running test: $testName:\t ${RED}Failed${NC}"
    fi
done

echo ""
echo -e "${GREEN}Number of passed tests: $passed_tests ${NC}"
echo -e "${RED}Number of failed tests: $failed_tests ${NC}"

if [ $failed_tests -ne 0 ]; then
    exit 1
fi
