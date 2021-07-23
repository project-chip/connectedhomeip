#! /usr/bin/env bash

source ./scripts/activate_environment.sh

# Build CHIP SDK
$TEST_CLIENT_ROOT/scripts/build_sdk.sh

# Build Docker image
$TEST_CLIENT_ROOT/scripts/build_container.sh
