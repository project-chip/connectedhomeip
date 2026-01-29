#!/bin/bash

echo "Building Python virtual environment..."
scripts/run_in_build_env.sh './scripts/build_python.sh --install_virtual_env out/venv'
./scripts/run_in_build_env.sh \
    "./scripts/build/build_examples.py \
      --target linux-x64-chip-tool-no-ble-tsan-clang \
      --target linux-x64-all-clusters-no-ble-tsan-clang \
      build \
      "

TARGET_TEST="TC_CC_10_1"

scripts/run_in_python_env.sh out/venv \
    "
    ./scripts/tests/run_python_test.py \
    --app out/linux-x64-all-clusters-no-ble-tsan-clang/chip-all-clusters-app  \
    --factory-reset \
    --app-args '--discriminator 1234 --KVS kvs1' \
    --script 'src/python_testing/$TARGET_TEST' \
    --script-args '--endpoint 1 --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021' \
    " | tee ~/test_"$TARGET_TEST".log
