## How to checkout AWS OTA Repository
 - After checkout to slc branch go to `third_party/silabs/aws_ota_sdk`.
 - To build AWS OTA,the submodule dependency of source folder is required. Need to update AWS OTA SDK submodule using the command: `git submodule update --checkout --init --recursive`

## How to build AWS OTA feature
 - For Building AWS OTA feature, refer [Addition of DIC AWS OTA component](./../../../docs/silabs/slc/sld253-matter-wifi/build-dic.md) 

## How to execute AWS OTA command through MQTT Explorer 
 - For executing AWS OTA command, refer [Download AWS OTA Image through cloud interface](./../../../docs/silabs/slc/sld253-matter-wifi/dic-wifi.md)