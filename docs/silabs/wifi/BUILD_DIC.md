# Build Procedure For Direct Internet Connectivity (DIC)

## Build command with DIC
- To enable DIC functionality use the `enable_dic=true` flag.
### Here is an example to build the lighting-app with DIC feature for the EFR32MG24 + 9116
   ```shell
./scripts/examples/gn_silabs_example.sh examples/lighting-app/silabs/ out/rs9116_DIC_light BRD418XX disable_lcd=true use_external_flash=false chip_enable_wifi_ipv4=true enable_dic=true chip_enable_ble_rs911x=true --wifi rs9116
   ```
### Here is an example to build the lighting-app with DIC feature for the EFR32MG24 + WF200
   ```shell
./scripts/examples/gn_silabs_example.sh examples/lighting-app/silabs/ out/wf200_DIC_light BRD418XX chip_build_libshell=false chip_enable_wifi_ipv4=true enable_dic=true --wifi wf200
```
### Here is an example to build the lighting-app with DIC for the EFR32MG24 + 917 NCP

   ```shell
./scripts/examples/gn_silabs_example.sh examples/lighting-app/silabs/ out/siwx917_DIC_light BRD418XX disable_lcd=true use_external_flash=false chip_enable_wifi_ipv4=true enable_dic=true chip_enable_ble_rs911x=true --wifi SiWx917
```
### Here is an example to build the lighting-app with DIC for the SiWx917 SoC
     
   ```shell
./scripts/examples/gn_silabs_example.sh examples/lighting-app/silabs/ out/siwx917_DIC_light BRD4338A enable_dic=true chip_enable_wifi_ipv4=true
   ```

## Build command with DIC AWS OTA
- To enable DIC AWS OTA functionality use the `aws_sdk_ota=true` flag.

### Here is an example to build the lighting-app with DIC AWS OTA feature for the EFR32MG24 + 9116
   ```shell
./scripts/examples/gn_silabs_example.sh examples/lighting-app/silabs/ out/rs9116_DIC_light BRD418XX disable_lcd=true use_external_flash=false chip_enable_wifi_ipv4=true enable_dic=true aws_sdk_ota=true chip_enable_ble_rs911x=true --wifi rs9116
   ```
### Here is an example to build the lighting-app with DIC AWS OTA feature for the EFR32MG24 + WF200
   ```shell
./scripts/examples/gn_silabs_example.sh examples/lighting-app/silabs/ out/wf200_DIC_light BRD418XX chip_build_libshell=false chip_enable_wifi_ipv4=true enable_dic=true aws_sdk_ota=true --wifi wf200
```
### Here is an example to build the lighting-app with DIC AWS OTA feature for the EFR32MG24 + 917 NCP

   ```shell
./scripts/examples/gn_silabs_example.sh examples/lighting-app/silabs/ out/siwx917_DIC_light BRD418XX disable_lcd=true use_external_flash=false chip_enable_wifi_ipv4=true enable_dic=true aws_sdk_ota=true chip_enable_ble_rs911x=true --wifi SiWx917
  ```
### Here is an example to build the lighting-app with DIC AWS OTA feature for the SiWx917 SoC
     
   ```shell
./scripts/examples/gn_silabs_example.sh examples/lighting-app/silabs/ out/siwx917_DIC_light BRD4338A enable_dic=true chip_enable_wifi_ipv4=true aws_sdk_ota=true
   ```
## Compile using new/different certificates

   - Two devices should not use the same Client ID. To use a different Client ID for your second connection do the following:
   - If using AWS, Change the following file `examples/platform/silabs/DIC/matter_abs_interface/src/dic_nvm_cert.cpp` under `#if USE_AWS`
      - Use `DIC_SERVER_HOST` name with your Server host name.
         - For Example: a2m21kovu9tcsh-ats.iot.ap-southeast-1.amazonaws.com 
      - Use AWS CA certificate as ca_certificate, device_certificate and device_key with your device cert and device key.
      - Use `DIC_CLIENT_ID` macro value with your Client ID in `examples/platform/silabs/DIC/matter_abs_interface/include/dic_config.h`
      - The preferred certificate type to use in the application is ECDSA.
   - If using mosquitto, change the following files `matter/examples/platform/silabs/DIC/matter_abs_interface/src/dic_nvm_cert.cpp`, `examples/platform/silabs/DIC/matter_abs_interface/include/dic_config.h` and enable `USE_MOSQUITTO` and disable `USE_AWS`.
      - Under `#if USE_MOSQUITTO`
      - Use `DIC_SERVER_HOST` name with your Server host name where Mosquitto Broker is running.
      - Use OpenSSL CA certificate as ca_certificate, device_certificate and device_key.
      - `DIC_CLIENT_ID` is not required here but `DIC_CLIENT_USER` and `DIC_CLIENT_PASS` in `examples/platform/silabs/DIC/matter_abs_interface/include/dic_config.h` needs to be updated as per your Mosquitto password file.
      - The preferred certificate type to use in the application is ECDSA.
