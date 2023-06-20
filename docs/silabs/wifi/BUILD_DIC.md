# Build Procedure For Direct Internet Connectivity (DIC)

## Build command with DIC
To enable DIC functionality use the `enable_dic=true` flag.
### Here is an example to build the lighting-app with DIC feature for the EFR32MG24 + 9116
   ```shell
./scripts/examples/gn_efr32_example.sh silabs_examples/Direct_Internet_lighting-app/efr32 out/rs9116_DIC_light BRD418XX disable_lcd=true use_external_flash=false chip_enable_wifi_ipv4=true enable_dic=true chip_enable_ble_rs911x=true --wifi rs9116
   ```
### Here is an example to build the lighting-app with DIC feature for the EFR32MG24 + WF200
   ```shell
./scripts/examples/gn_efr32_example.sh silabs_examples/Direct_Internet_lighting-app/efr32/ out/wf200_DIC_light BRD418XX chip_build_libshell=false chip_enable_wifi_ipv4=true enable_dic=true --wifi wf200
```
### Here is an example to build the lighting-app with DIC for the EFR32MG24 + 917 NCP

   ```shell
./scripts/examples/gn_efr32_example.sh silabs_examples/Direct_Internet_lighting-app/efr32 out/siwx917_DIC_light BRD418XX disable_lcd=true use_external_flash=false chip_enable_wifi_ipv4=true enable_dic=true chip_enable_ble_rs911x=true --wifi SiWx917
```
### Here is an example to build the lighting-app with DIC for the SiWx917 SoC
     
   ```shell
./scripts/examples/gn_efr32_example.sh silabs_examples/Direct_Internet_lighting-app/SiWx917/ out/siwx917_DIC_light BRD4325B enable_dic=true chip_enable_wifi_ipv4=true
   ```
## Compile using new/different certificates

   - Two devices should not use the same Client ID. To use a different Client ID for your second connection do the following:
   - If using AWS, Change the following file `examples/platform/silabs/DIC/matter_abs_interface/include/dic_config.h` under `#if USE_AWS`
        * Replace `DIC_CLIENT_ID` macro value with your Client ID
        * Replace device_certificate and device_key with your device cert and device key. (There is no need to change the CA certificate.)
   - The preferred certificate type to use in the application is ECDSA.
   - If using mosquitto, change the following file `matter/examples/platform/silabs/DIC/matter_abs_interface/include/dic_config.h` enable `USE_MOSQUITTO` and disable `USE_AWS`. 
   - Under `#if USE_MOSQUITTO`
        * Replace DIC_CLIENT_ID macro value with your Client ID
        * Replace ca_certficate, device_certificate and device_key with your ca_certficate, device cert and device key.
   - The preferred certificate type to use in the application is ECDSA.
