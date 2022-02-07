#!/bin/bash

if [[ ! -d $MW320_FOLDER ]]; then
    echo "MW320_FOLDER is not set"
    exit 1
fi

tar xzvf ./third_party/mw320_sdk/sdk_fixes/SDK_2_9_4_Matter_ipv6.tgz -C "$MW320_FOLDER"/
tar xzvf ./third_party/mw320_sdk/sdk_fixes/mw320_matter_openocd.tgz -C "$MW320_FOLDER"/
echo "export MW320_SDK_ROOT=\"$MW320_FOLDER/SDK_2_9_4_Matter_ipv6\"" > ./third_party/mw320_sdk/sdk_fixes/set_env.sh
chmod +x+w ./third_party/mw320_sdk/sdk_fixes/set_env.sh
echo "MW320_SDK_ROOT=$MW320_FOLDER/SDK_2_9_4_Matter_ipv6" >> ./third_party/mw320_sdk/sdk_fixes/set_env.sh
echo "echo \"MW320 SDK is at \$MW320_SDK_ROOT\"" >> ./third_party/mw320_sdk/sdk_fixes/set_env.sh
echo "MATTER_SDK=\$(pwd)" >> ./third_party/mw320_sdk/sdk_fixes/set_env.sh
echo "export MW320_LIGHTING=\$MATTER_SDK/examples/lighting-app/mw320" >> ./third_party/mw320_sdk/sdk_fixes/set_env.sh
echo "echo \"MW320 LIGHTING is at \$MW320_LIGHTING\"" >> ./third_party/mw320_sdk/sdk_fixes/set_env.sh

echo "MW320 SDK and Flsahing tool  was install to $MW320_FOLDER !"
exit 0
