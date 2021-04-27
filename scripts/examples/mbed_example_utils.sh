#!/usr/bin/env bash

#
#    Copyright (c) 2020 Project CHIP Authors
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

source "$(dirname "$0")/../../scripts/activate.sh"

cd "$(dirname "$0")/../../examples"

SUPPORTED_TARGET_BOARD=(DISCO_L475VG_IOT01A NRF52840_DK)
SUPPORTED_APP=(shell lock-app)
SUPPORTED_PROFILES=(release develop debug)

APP=
TARGET_BOARD=
PYOCD_TARGET=
PYOCD_TARGET_ARGS=
PROFILE=

for i in "$@"; do
    case $i in
        -a=* | --app=*)
            APP="${i#*=}"
            shift
            ;;
        -b=* | --board=*)
            TARGET_BOARD="${i#*=}"
            shift
            ;;
        -p=* | --profile=*)
            PROFILE="${i#*=}"
            shift
            ;;
        *)
            # unknown option
            ;;
    esac
done

case $TARGET_BOARD in
    NRF52840_DK)
        PYOCD_TARGET=nrf52840
        ;;

    DISCO_L475VG_IOT01A)
        PYOCD_TARGET=stm32l475xg
        PYOCD_TARGET_ARGS="--frequency 4M"
        ;;

    *)
        # unknown option
        ;;
esac

if [[ ! " ${SUPPORTED_TARGET_BOARD[@]} " =~ " ${TARGET_BOARD} " ]]; then
    echo "ERROR: Target $TARGET_BOARD not supported"
    exit 1
fi

if [[ ! " ${SUPPORTED_APP[@]} " =~ " ${APP} " ]]; then
    echo "ERROR: Application $APP not supported"
    exit 1
fi

if [[ ! " ${SUPPORTED_PROFILES[@]} " =~ " ${PROFILE} " ]]; then
    echo "ERROR: Profile $PROFILE not supported"
    exit 1
fi

echo "############################"
pyocd flash -t "$PYOCD_TARGET" -O connect_mode=under-reset "$PYOCD_TARGET_ARGS" "$PWD/$APP/mbed/build-$TARGET_BOARD/$PROFILE/chip-mbed-$APP"-example.hex
echo "############################"
