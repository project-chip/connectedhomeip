
if [[ -z "${CHIP_SDK_ROOT}" ]]; then
    export CHIP_SDK_ROOT=../../../..
    export TEST_CLIENT_ROOT=./
    source $CHIP_SDK_ROOT/scripts/activate.sh
fi
