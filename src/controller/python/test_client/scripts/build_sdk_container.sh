#! /usr/bin/env bash

# Skip creating environment if it already exists
if [[ -z "${CHIP_SDK_ROOT}" ]]; then
  source ./scripts/activate_environment.sh
fi

# Build CHIP SDK
gn gen $CHIP_SDK_ROOT/out/debug
ninja -C $CHIP_SDK_ROOT/out/debug
cp $CHIP_SDK_ROOT/out/debug/controller/python/*.whl $TEST_CLIENT_ROOT/

# Build Docker image
$TEST_CLIENT_ROOT/scripts/build_container.sh
