#!/bin/bash

SDB="$TIZEN_SDK_ROOT/tools/sdb"
APP_NAME='org.tizen.matter.example.lighting'

if [[ $($SDB devices | wc -l) -le 1 ]]; then
    echo "No device connected"
    exit 1
fi

"$SDB" root on
re='^[0-9]+$'
lauch_out=$("$SDB" shell app_launcher -s org.tizen.matter.example.lighting vendor-id 65521 product-id 32769 discriminator 42 passcode 1234)
APP_PID=$(echo "$lauch_out" | cut -d ' ' -f6)
re='^[0-9]+$'
if ! [[ "$APP_PID" =~ "$re" ]]; then
    echo "Wrong pid number" >&2
    echo "Launch command out:"
    echo "$lauch_out"
    exit 1
fi

"$SDB" shell "gdbserver :9999 --attach $APP_PID"
